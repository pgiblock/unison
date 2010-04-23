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
#include "unison/Processor.h"
#include "unison/Session.h"
#include "unison/BufferProvider.h"
#include "unison/JackEngine.h"

namespace Unison
{

Session::Session (JackEngine& engine) :
  m_bufferProvider(),
  m_engine(&engine),
  m_rootNode(NULL)
{
  // FIXME: Remove hardcoded bufferlength1
  m_bufferProvider = new PooledBufferProvider();
  m_bufferProvider->setBufferLength(1024);
  engine.setSession(this);
}


Session::~Session ()
{
  delete m_bufferProvider;
}


BufferProvider& Session::bufferProvider() const
{
  return *m_bufferProvider;
}

void Session::process(const ProcessingContext& context)
{
  if (m_rootNode) {
    m_rootNode->process(context);
  }
}

} // Unison

// vim: et ts=8 sw=2 sts=2 noai
