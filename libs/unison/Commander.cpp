/*
 * Commander.cpp
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

#include "Commander.hpp"
#include "Command.hpp"

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

namespace Unison {
  namespace Internal {

Commander* Commander::m_instance = static_cast<Commander*>(NULL);

void Commander::initialize ()
{
  Q_ASSERT(m_instance == NULL);
  m_instance = new Commander();
}


Commander::Commander () :
  m_writeLock(),
  m_blockWait(),
  m_buffer(COMMAND_BUFFER_LENGTH)
{}


void Commander::push (Command* command)
{
  bool block = command->isBlocking();

  QMutexLocker locker(&m_writeLock);
  command->preExecute();
  m_buffer.write(&command, 1);
  if (block) {
    m_blockWait.acquire();
  }
} 


void Commander::process (ProcessingContext& context)
{
  Command* commands[COMMANDS_PER_PROCESS];
  int cnt = m_buffer.read(commands, COMMANDS_PER_PROCESS);

  for (int i=0; i<cnt; ++i) {
    commands[i]->execute(context);
  }
}

  } // Internal
} // Unison

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
