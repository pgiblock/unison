/*
 * Session.cpp
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

#include <QDebug>

#include "unison/Node.h"
#include "unison/CompositeProcessor.h"
#include "unison/Session.h"
#include "unison/PooledBufferProvider.h"
#include "unison/JackEngine.h"

namespace Core {

Session::Session (JackEngine& engine) :
  m_rootProcessor(NULL),
  m_bufferProvider(),
  m_engine(&engine)
{
  // FIXME: Remove hardcoded bufferlength1
  m_bufferProvider = new PooledBufferProvider();
  m_bufferProvider->setBufferLength(UNISON_BUFFER_LENGTH);

  m_rootProcessor = new CompositeProcessor();

  engine.setSession(this);
}


Session::~Session ()
{
  delete m_bufferProvider;
}


BufferProvider& Session::bufferProvider () const
{
  return *m_bufferProvider;
}


void Session::process (const ProcessingContext& context)
{
  m_rootProcessor->process(context);
}


void Session::hackCompile ()
{
  m_rootProcessor->hackCompile(bufferProvider());
}

void Session::add (Processor* processor)
{
  m_rootProcessor->add(processor);
}


void Session::remove (Processor* processor)
{
  m_rootProcessor->remove(processor);
}

} // Core

// vim: ts=8 sw=2 sts=2 et sta noai
