/*
 * PostExecuter.cpp
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

#include "PostExecuter.hpp"

#include "Command.hpp"

#include <QDebug>
#include <QWaitCondition>

namespace Unison {
  namespace Internal {


PostExecuter::PostExecuter() :
  QThread(),
  m_buffer(COMMAND_BUFFER_LENGTH),
  m_done(false)
{
}


PostExecuter::~PostExecuter()
{
  stop();
}


void PostExecuter::push (Command* command)
{
  m_buffer.write(&command, 1);

}


void PostExecuter::run ()
{
  const int bufSize = 8;
  Command* buf[bufSize];

  setPriority(QThread::LowestPriority);

  forever {
    QThread::msleep(IDLE_TIMEOUT);
    const bool done = m_done;

    // Hold on to done, and check after loop, gives us a final iteration for cleanup
    int cnt;
    do {
      cnt = m_buffer.read(buf, bufSize);
      for (int i=0; i<cnt; ++i) {
        postExecute(buf[i]);
      }

    } while (cnt > 0);

    if (done)
      break;
  }
}


bool PostExecuter::stop ()
{
  m_done = true;
  // Wait for another iteration
  return wait();
}


void PostExecuter::postExecute (Command* cmd)
{
  cmd->postExecute();
  delete cmd;
}


  } // Internal
} // Unison


// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
