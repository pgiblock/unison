/*
 * Scheduler.cpp
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

#include "Scheduler.h"

#include <QAtomicInt>
#include <QSemaphore>

#include <stdlib.h> // for rand

namespace Unison {
  namespace Internal {


WorkQueue::WorkQueue () :
  m_head(NULL),
  m_tail(NULL)
{}

void WorkQueue::push (WorkUnit* u)
{
  u->prev = NULL;
  u->next = m_head;
  if (m_head) {
    m_head->prev = u;
  }
  else {
    m_tail = u;
  }

  m_head = u;
}

WorkUnit* WorkQueue::pop ()
{
  Q_ASSERT(m_head);
  WorkUnit* r = m_head;
  m_head = m_head->next;
  m_head->prev = NULL;
  if (!m_head) {
    m_tail = NULL;
  }
  return r;
}

WorkUnit* WorkQueue::steal ()
{
  Q_ASSERT(m_tail);
  WorkUnit* r = m_tail;
  m_tail = m_tail->prev;
  m_tail->next = NULL;
  if (!m_tail) {
    m_head = NULL;
  }
  return r;
}

void WorkQueue::initializeFrom (const WorkQueue& other)
{
  // Init end pointers
  m_head = other.m_head;
  m_tail = other.m_tail;
  
  // Init linked list
  WorkUnit* unit = m_head;
  WorkUnit* prev = NULL;
  while (unit) {
    unit->next = unit->initialNext;
    unit->prev = prev;
    prev = unit;
    unit = unit->next;
  }
}


Worker::Worker (WorkerGroup& group) :
  m_group(group),
  m_readyList(),
  m_lock(),
  m_random()
{
  // Assume stdlib RNG has been seeded
  m_random.seed(rand() % RAND_MAX);
}


  } // Internal
} // Unison;

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai

