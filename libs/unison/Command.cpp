/*
 * Command.cpp
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

#include "Command.h"

#include <QtCore/QtGlobal>

using namespace Unison;


Command::Command () :
  m_blocking(true),
  m_state(Invalid),
  m_errorCode(0)
{}


void Command::preExecute ()
{
  Q_ASSERT(m_state==Created);
}


void Command::execute (ProcessingContext &ctx)
{
  Q_UNUSED(ctx);
  Q_ASSERT(m_state==PreExecuted);
  m_state = Executed;
}


void Command::postExecute ()
{
  Q_ASSERT(m_state==Executed);
  m_state = PostExecuted;
}


bool Command::isBlocking () const
{
  return m_blocking;
}


Command::State Command::state () const
{
  return m_state;
}


bool Command::hasError () const
{
  return m_errorCode != 0;
}


int Command::errorCode () const
{
  return m_errorCode;
}

// vim: ts=8 sw=2 sts=2 et sta noai
