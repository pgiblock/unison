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

#include "Commander.h"
#include "Command.h"

#include <QMutex>
#include <QMutexLocker>

namespace Unison {
  namespace Internal {

Commander* Commander::m_instance = static_cast<Commander*>(NULL);

void Commander::initialize ()
{
  m_instance = new Commander();
}


Commander::Commander () :
  m_writeLock(),
  m_blockWait(),
  m_buffer(24)
{
  //
}


void Commander::push (Command *command)
{
  bool block = command->isBlocking();
  size_t size = sizeof(command);

  QMutexLocker locker(&m_writeLock);
  command->preExecute();
  m_buffer.write(&command, 1);
  if (block) {
    m_blockWait.acquire();
  }
} 


void Commander::process (ProcessingContext &context)
{
  const int MAX_COMMANDS = 1;
  Command* commands[MAX_COMMANDS];
  int cnt = m_buffer.read(commands, MAX_COMMANDS);

  printf("COMMANDER: Executing %d commands.\n", cnt);
  for (int i=0; i<cnt; ++i) {
    commands[i]->execute(context);
  }
}

  } // Internal
} // Unison

// vim: ts=8 sw=2 sts=2 et sta noai
