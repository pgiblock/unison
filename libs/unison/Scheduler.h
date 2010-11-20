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
#include <QThread> // for debugging
#include <stdio.h>

#include <stdlib.h> // for rand

// The most RT-unsafe way to message ever! but is handy anyways
#define rtprintf(...) //printf(__VA_ARGS__)

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
  QAtomicInt liveWorkers; // workers who haven't resorted to stealing
  QSemaphore done;

  // Possibility 2:
  // keep count of processors, decrement when run. done when it reaches 0
  // works as long as the number of processors is known upfront.
  //QAtomicInt workLeft;

  friend class Worker;
};



class Worker 
{
  public:
    Worker (WorkerGroup& group);
    
    bool runOnce (const ProcessingContext& ctx)
    {
      rtprintf("%x: Hello again.\n", QThread::currentThreadId());
      // Look at us
      lock();
      WorkUnit* unit = m_readyList.pop();
      unlock();

      // Stealing
      if (!unit) {
        if (m_group.workerCount==1) {
          rtprintf("W: nothing in readyList, singleThread done.\n");
          return false;
        }
        
        rtprintf("%x: Stealing..\n", QThread::currentThreadId());
        unit = stealRandomly();
        if (!unit) {
          rtprintf("%x: Failed steal\n", QThread::currentThreadId());
          if (!m_stealing) {
            rtprintf("%x: Our very first steal!! awww..\n", QThread::currentThreadId());
            m_stealing=true;
            if (m_group.liveWorkers.fetchAndAddOrdered(-1) == 1) {
              rtprintf("%x: WE ARE DONE!!\n", QThread::currentThreadId());
              m_group.done.release(1);
              return false;
            }
          }
          // Otherwise, hope we have more luck next time
          QThread::yieldCurrentThread();
          return true; // We aren't done necessarily done yet
        }
        else if (m_stealing) {
          m_group.liveWorkers.fetchAndAddOrdered(1);
          m_stealing = false;
        }
      }
      else {
        rtprintf("%x: Actually have a unit!\n", QThread::currentThreadId());
      }

      // Loop over the immediate execution path (depth first)
      while (unit) {

        // Processing
        rtprintf("%x: Processing `%s` (%x)\n", QThread::currentThreadId(),
               qPrintable(unit->processor->name()),unit->processor);
        Processor* p = unit->processor;
        p->process(ctx);

        // Readying dependents
        lock(); // TODO: Move lock to immediately before the loop below?
        WorkUnit** dp = unit->dependents;
        WorkUnit*  u;
        
        // Reuse unit for finding our next unit
        unit = NULL;

        // Loop until the first ready one, stash that one for ourself
        for (; *dp; ++dp) {
          u = *dp;
          rtprintf("Decr1\n");
          if (u->wait.fetchAndAddOrdered(-1) == 1) {
            rtprintf("  Saving..\n");
            unit = u;
            ++dp;
            break; // now continue with the "queueing" loop
          }
        }
        
        // Queue the rest
        for (; *dp; ++dp) {
          u = *dp;
          rtprintf("Decr2\n");
          //printf("%x: decr: %llx  (%x) pre: %d\n", QThread::currentThreadId(),
          //       (unsigned long long)u, u->processor, (int)u->wait);
          if (u->wait.fetchAndAddOrdered(-1) == 1) {
            rtprintf("  Queuing..\n");
            m_readyList.push(u);
          }
        }
        unlock();
      }


  //
  //      // FIXME: I DON"T THINK THIS WORKLEFT IS WORKING THE WAY WE THINK!!
  //         I THINK IT IS POSSIBLY BREAKING EARLY, HENCE LOW UTILIZATION

        /*
        foo = m_group.workLeft.fetchAndAddOrdered(-1);
        //printf("FOO -1: %d\n", (int)foo);
        if (foo == 1) {
          //printf("%x: WE SHOULD BE DONE!!!\n", QThread::currentThreadId());
          m_group.workLeft.fetchAndStoreOrdered(-1); // Special case: -1 done
          return false;
        }*/


      return true;
    }

    inline void run (const ProcessingContext& ctx)
    {
      rtprintf("%x: run() --------------------------------------------------\n", QThread::currentThreadId());
      m_stealing = false;
      while (runOnce(ctx) && m_group.liveWorkers != 0) {}
      rtprintf("%x: we are done.", QThread::currentThreadId());
    }

    inline WorkUnit* trySteal ()
    {
      WorkUnit* u = NULL;
      if (tryLock()) {
        u = m_readyList.steal();
        unlock();
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
        n = (n+1) % m_group.workerCount;
      }
      //printf("Going to steal from workers[%d]!\n", n);
      victim = m_group.workers[n];

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
    bool       m_stealing;
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

