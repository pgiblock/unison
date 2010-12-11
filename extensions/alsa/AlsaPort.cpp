/*
 * AlsaPort.cpp
 *
 * Copyright (c) 2010 Andrew Kelley <superjoe30+unison@gmail.com>
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

#include "AlsaPort.hpp"
#include "AlsaBackend.hpp"

#include <unison/AudioBuffer.hpp>
#include <unison/Patch.hpp>

#include <QDebug>


const int UNISON_BUFFER_LENGTH = 1024;

using namespace Unison;

namespace Alsa {
  namespace Internal {


AlsaPort::AlsaPort (AlsaBackend& backend, const QString& name, PortDirection direction) :
  BackendPort(),
  m_backend(backend),
  m_id(name),
  m_direction(direction)
{}


// Deferred initialization
bool AlsaPort::registerPort()
{
  // nothing to do
  return true;
}


Unison::Node* AlsaPort::parent() const
{
  return m_backend.rootPatch();
}


const QSet<Node* const> AlsaPort::interfacedNodes() const
{
  QSet<Node* const> dependencies;
  dependencies.insert(parent());
  return dependencies;
}


void AlsaPort::connectToBuffer()
{
  // don't need to do anything
}


void AlsaPort::activate(Unison::BufferProvider& bp)
{
  acquireBuffer(bp);
  connectToBuffer();
}

  } // Internal
} // Alsa

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
