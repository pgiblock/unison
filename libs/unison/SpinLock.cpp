/*
 * SpinLock.cpp
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

#include "SpinLock.h"
#include <QThread>

namespace Unison {


SpinLock::SpinLock ()
{
  release();
}


void SpinLock::lock ()
{
  int count;
  const int maxspin = 1048;

  if (!acquire()) {
    count = 0;
    do {
      do {
        pause();

        if (++count >= maxspin) {
          /* let the OS reschedule every once in a while */
          yield();
          count = 0;
        }
      }
      while (m_lock != 0);
    }
    while (!acquire());
  }
}


bool SpinLock::tryLock ()
{
  return acquire();
}


void SpinLock::unlock ()
{
     release();
}


void SpinLock::pause ()
{
#if defined __i386__ || defined __x86_64
    __asm__("pause");
#else
# error SpinLock::pause undefined on this platform
#endif
}


void SpinLock::yield ()
{
  // Could yield to OS if we deem it safe.
  QThread::yieldCurrentThread();
}


} // Unison

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
