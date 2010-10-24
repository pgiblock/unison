/*
 * Scheduler.h
 *
 * Copyright (c) 2010 Paul Giblock <pgib/at/users.sourceforge.net>
 *
 * This file is part of Unison - http://unison.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#ifndef UNISON_SCHEDULER_H_
#define UNISON_SCHEDULER_H_

#include "FastRandom.h"
#include "Processor.h"
#include "SpinLock.h"

#include <QAtomicInt>
#include <QSemaphore>

#include <stdlib.h> // for rand

namespace Unison {
  namespace Internal {

// Notes:
//  Pointers are generally bad. Unless you're in DSP RT tight-loop land.
//  We are using 'ordered' operation on QAtomics, we may be able to loosen this

class Worker;

/**
 * This represents an individual unit of work.  A WorkUnit can be summed up as a
 * Processor along with the dependents and count of dependencies.  For performance, we
 * also store the current state of the work unit (wait count).  Work units are only
 * used by a single  Backend, so there is no harm in keeping a single state.  Finally,
 * the WorkUnit participates in an intrusive doubly-linked list (There is one list per
 * Worker). The WorkUnit only exists in a single list, so this restriction is fine. */
struct WorkUnit
{
  //Q_DISABLE_COPY(WorkUnit)

  // Work unit definition 
  Processor* processor;   ///< The processor to run
  int initialWait;  ///< Initial count of unresolved dependencies
  WorkUnit** dependents;   ///< Decrement these waits when we are done processing

  // State
  QAtomicInt wait;        ///< Initialized to number of dependencies each run

  // Intrusive Doubly-linked list
  WorkUnit* initialNext;  ///< Used to rebuild the schedule (next and prev) each run
  WorkUnit* next;         ///< Next pointer for whichever dequeue we live in
  WorkUnit* prev;         ///< Next pointer for whichever dequeue we live in
};



class WorkQueue
{
  // TODO: Disable copy
  public:
    WorkQueue ();

    /** 
     * Used by the worker to enqueue work that has just been made ready */
    void push (WorkUnit* u);

    /**
     * Used by the worker to get work when the current execution path has ended */
    WorkUnit* pop ();

    /**
     * Used by thieves to steal work when their queue is empty */
    WorkUnit* steal ();

    /**
     * Initialize the queue by means of another queue.  We _could_ have used a copy
     * ctor for this, but WorkQueue doesn't support copy per-se.  What we want is to
     * restore the state of a queue multiple times */
    void initializeFrom (const WorkQueue& other);

  private:
    WorkUnit* m_head;
    WorkUnit* m_tail;

  friend class Worker;  // FIXME: HACK
};



class WorkerGroup
{
  public: // Temporary full-public
  Worker** workers;
  int workerCount;

  // Possibility 1:
  // increment when a thread has a food supply, decrement when resorting to stealing
  // if value == 0, then we are done (threads wait for post)
  //QAtomicInt liveWorkers; // workers who haven't resorted to stealing

  // Possibility 2:
  // keep count of processors, decrement when run. done when it reaches 0
  // works as long as the number of processors is known upfront.
  QAtomicInt workLeft;

  friend class Worker;
};



class Worker 
{
  public:
    Worker (WorkerGroup& group);
    
    bool runOnce (const ProcessingContext& ctx)
    {
      printf("Run ...\n");
      // Look at us
      lock();
      WorkUnit* unit = m_readyList.pop();
      unlock();

      // Stealing
      if (!unit) {
#ifdef ENABLE_STEALING
        unit = stealRandomly();
        if (!unit) {
          return true;
        }
#else  // ENABLE_STEALING
        return false;
#endif // ENABLE_STALING
      }

      // Processing
      printf("Processing `%s`\n", qPrintable(unit->processor->name()));
      Processor* p = unit->processor;
      p->process(ctx);

      if (m_group.workLeft.fetchAndAddOrdered(-1) == 1) {
        return false;
      }

      // Readying dependents
      lock();
      for (WorkUnit** dp = unit->dependents; *dp; ++dp) {
        WorkUnit *u = *dp;
        printf("decr: %llx  pre: %d\n", (unsigned long long)u, (int)u->wait);
        // Decrement the wait. if the old value was 1, then we are at 0 and done.
        if (u->wait.fetchAndAddOrdered(-1) == 1) {
          printf("push: %llx \n", (unsigned long long)u);
          // TODO: Can optimize by not pushing, then popping right back
          m_group.workLeft.fetchAndAddOrdered(1);
          m_readyList.push(u);
        }
      }
      unlock();

      return true;
    }

    inline void run (const ProcessingContext& ctx)
    {
      printf("Running ...\n");
      while (runOnce(ctx)) {}
      printf("Running Done.\n");
    }

    inline WorkUnit* trySteal ()
    {
      WorkUnit* u = NULL;
      if (tryLock()) {
        u = m_readyList.steal();
        lock();
      }
      return u;
    }

    /**
     * Pushes the null-terminated list onto the queue. No locking occurs since this
     * function would run while any other workers are blocked */
    inline void pushReadyWorkUnsafe (WorkUnit* units, int count)
    {
      for (int i=0; i<count; ++i) {
        m_readyList.push(&units[i]);
      }
    }

  private:

    bool canStealFrom (Worker* victim) const
    {
      return victim->m_readyList.m_tail;
    }


    WorkUnit* stealRandomly ()
    {
      // TODO: if num-threads is set to 2, no need to randomly pick
      unsigned n = m_random.nextInt() % m_group.workerCount;
      Worker* victim = m_group.workers[n];
      // Can't steal from self.  TODO: Avoid this instead of working around it
      if (victim == this) {
        victim = m_group.workers[(n+1) % m_group.workerCount];
      }

      // Now check if there is any point in stealing (avoid locking)
      if (!canStealFrom(victim)) {
        return NULL;
      }

      return victim->trySteal();
    }

    inline void lock ()
    {
      m_lock.lock();
    }

    inline bool tryLock ()
    {
      return m_lock.tryLock();
    }

    inline void unlock ()
    {
      m_lock.unlock();
    }

  protected:

  private:
    WorkerGroup& m_group;
    WorkQueue m_readyList;
    SpinLock  m_lock;
    FastRandom m_random;
};



/**
 * A schedule is prepared by non-RT land and then passed over to the backend in this
 * handy class. */
class Schedule
{
  public:
  // /**
  //  * The initial queue of ready work. This queue is technically invalid at all times.
  //  * It is only used to initialize the Queue used by the Worker itself. */
  // const WorkQueue queue;

  WorkUnit* readyWork;
  int readyWorkCount;

  /**
   * All the units, for resource management.
   * Client should use a smart pointer around Schedule itself.  */
  WorkUnit* work;
  int workCount;
};


  } // Internal
} // Unison;

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai

