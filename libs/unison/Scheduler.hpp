/*
 * Scheduler.hpp
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

#ifndef UNISON_SCHEDULER_HPP_
#define UNISON_SCHEDULER_HPP_

#include "FastRandom.hpp"
#include "Processor.hpp"
#include "SpinLock.hpp"

#include <QAtomicInt>
#include <QSemaphore>
#include <QThread> // for debugging
#include <stdio.h>

#include <stdlib.h> // for rand

//#define NO_PROCESS

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
  // Work unit definition 
  Processor* processor;   ///< The processor to run
  int initialWait;        ///< Initial count of unresolved dependencies
  WorkUnit** dependents;  ///< Decrement these waits when we are done processing

  // State
  QAtomicInt wait;        ///< Initialized to number of dependencies each run

  // Intrusive Doubly-linked list
  WorkUnit* initialNext;  ///< Used to rebuild the schedule (next and prev) each run
  WorkUnit* next;         ///< Next pointer for whichever dequeue we live in
  WorkUnit* prev;         ///< Next pointer for whichever dequeue we live in
};


/**
 * A WorkQueue is held by a worker and maintains a list of ready-work.  The queue
 * is intrusive to avoid allocating nodes for the container.  The front of the queue
 * is used by the owning worker and the rear is used by thieves.  This helps maintain
 * reasonable locality on the owning-side, while leaving units that are likely to have
 * more dependents for theives.  The implementation could be replaced with a more
 * complicated lock-free structure in the future, but this seems fine for now. 
 *
 * TODO: rewrite using a sentinel node for extra efficiency. */
class WorkQueue
{
  Q_DISABLE_COPY(WorkQueue)

  public:
    WorkQueue ();

    /** 
     * Used by the worker to enqueue work that has just been made ready
     * @param unit work that has just met its dependencies. */
    void push (WorkUnit* unit);

    /**
     * Used by the worker to get work when the current execution path has ended
     * @return the "last-in" WorkUnit */
    WorkUnit* pop ();

    /**
     * Used by thieves to steal work when their queue is empty
     * @return the "first-in" WorkUnit */
    WorkUnit* steal ();

    /**
     * Check if there is anything worth stealing
     * @return true if work exists */
    inline bool isNotEmpty ()
    {
      return m_tail;
    }

    /**
     * Initialize the queue by means of another queue.  We _could_ have used a copy
     * ctor for this, but WorkQueue doesn't support copy per-se.  What we want is to
     * restore the state of a queue multiple times 
     * @param other the head of the linked list walked through initialNext pointer */
    void initializeFrom (const WorkQueue& other);

  private:
    WorkUnit* m_head;   ///< The head - used by the owner Worker
    WorkUnit* m_tail;   ///< The tail - used by theives
};



/**
 * A happy family of workers.  Used for synchronization */
struct WorkerGroup
{
  public: // Temporary full-public
    
  Worker** workers; ///< The workers in this group
  int workerCount;  ///< The size of this group

  /**
   * Used by workers to determine when to quit.  Incremented when a thread gains
   * food supply, decremented when resorting to stealing. */
  QAtomicInt liveWorkers;

  /**
   * Waitcondition for main processing thread. Signaled by the first thread to
   * realize that all workers are starved. */
  QSemaphore done;
};



/**
 * Workers process a schedule.  Many workers can work in parallel if they are
 * in a WorkerGroup.  There is no requirement how the workers are run exactly,
 * For example, one Worker could be executed in the main processing thread, while
 * additional Workers are run in a QThread. */
class Worker 
{
  public:
    /**
     * Create a Worker.
     * @param group The WorkerGroup the worker belongs to. */
    Worker (WorkerGroup& group);
    
    /**
     * Runs a single processing iteration.  This could be multiple WorkUnits,
     * if dependents are readied in order to reduce the cost of pushing just to
     * pop immediately afterwards.
     * @param ctx The ProcessingContext to run in
     * @return true if the Worker was able to run at least one WorkUnit */
    bool runOnce (const ProcessingContext& ctx)
    {
      // Look at us
      lock();
      WorkUnit* unit = m_readyList.pop();
      unlock();

      // Stealing
      if (!unit) {
        if (m_group.workerCount==1) {
          return false;
        }
        
        unit = stealRandomly();
        if (!unit) {
          if (!m_stealing) {
            m_stealing=true;
            // TODO: this could probably be a single counting Semaphore.
            if (m_group.liveWorkers.fetchAndAddOrdered(-1) == 1) {
              m_group.done.release(1);
              return false;
            }
          }
          // Otherwise, hope we have more luck next time
          QThread::yieldCurrentThread();
          return true; // We aren't necessarily done yet
        }
        else if (m_stealing) {
          m_group.liveWorkers.fetchAndAddOrdered(1);
          m_stealing = false;
        }
      }

      // Loop over the immediate execution path (depth first)
      while (unit) {

        // Processing
        Processor* p = unit->processor;
#ifndef NO_PROCESS
        p->process(ctx);
#endif

        // Readying dependents
        lock(); // TODO: Move lock to immediately before the loop below?
        WorkUnit** dp = unit->dependents;
        WorkUnit*  u;
        
        // Reuse unit for finding our next unit
        unit = NULL;

        // Loop until the first ready one, stash that one for ourself
        for (; *dp; ++dp) {
          u = *dp;
          if (u->wait.fetchAndAddOrdered(-1) == 1) {
            unit = u;
            ++dp;
            break; // now continue with the "queueing" loop
          }
        }
        
        // Queue the rest
        for (; *dp; ++dp) {
          u = *dp;
          if (u->wait.fetchAndAddOrdered(-1) == 1) {
            m_readyList.push(u);
          }
        }
        unlock();
      }

      return true;
    }

    /**
     * Run the Worker while there is work.
     * @param ctx The ProcessingContext to run in */
    inline void run (const ProcessingContext& ctx)
    {
      m_stealing = false;
      while (runOnce(ctx) && m_group.liveWorkers != 0) {}
    }

    /**
     * Try to steal from this Worker, used by other workesr when they've resorted to
     * stealing. 
     * @return The stolen WorkUnit if success, otherwise NULL. */
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
     * function would run while any other workers are blocked
     * @param units Array of units to ready
     * @oaram count size of units array */
    inline void pushReadyWorkUnsafe (WorkUnit* units, int count)
    {
      for (int i=0; i<count; ++i) {
        m_readyList.push(&units[i]);
      }
    }

  private:

    bool canStealFrom (Worker* victim) const
    {
      return victim->m_readyList.isNotEmpty();
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
    WorkerGroup& m_group;   ///< The WorkerGroup 
    WorkQueue m_readyList;  ///< List of WorkUnits that that have satisfied dependencies
    SpinLock  m_lock;       ///< Lock for the readyList
    FastRandom m_random;    ///< RNG used for picking victim Workers
    bool       m_stealing;  ///< Are we stealing?
};



/**
 * A schedule is prepared by non-RT land and then passed over to the backend in this
 * handy class. */
class Schedule
{
  public:
  /**
   * Work without any data dependencies, used to begin the processing */
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

