/*
 * JackPort.cpp
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

#include "JackPort.h"
#include "JackBufferProvider.h"
#include "JackBackend.h"

#include <unison/AudioBuffer.h>

#include <jack/jack.h>
#include <QDebug>


const int UNISON_BUFFER_LENGTH = 1024;

using namespace Unison;

namespace Jack {
  namespace Internal {

JackBufferProvider* JackPort::m_jackBufferProvider =
    new JackBufferProvider();


JackPort::JackPort (JackBackend& backend, const QString& name,
                    PortDirection direction) :
  BackendPort(),
  m_backend(backend),
  m_port(NULL),
  m_id(name),
  m_direction(direction)
{}


bool JackPort::registerPort ()
{
  JackPortFlags flags = JackPort::flagsFromDirection(m_direction);

  m_port = jack_port_register(m_backend.client(),
      m_id.toLatin1(), JACK_DEFAULT_AUDIO_TYPE, flags, 0 );

  return isRegistered();
}


const QSet<Node* const> JackPort::interfacedNodes() const
{
  const char** name = jack_port_get_connections( m_port );
  QSet<Node* const> dependencies;

  // Actually have something connected
  if (name != NULL) {
    uint32_t count = m_backend.portCount();
    // Within all connected ports
    for (; *name != NULL; name++) {
      // See if we own the port
      for (uint32_t i = 0; i < count; ++i) {
        JackPort* port = m_backend.port(i);
        if (port->name() == *name) {
          dependencies += port;
        }
      }
      //jack_free(*name);
    }
  }

  return dependencies;
}


void JackPort::connectToBuffer ()
{
  m_buffer = m_jackBufferProvider->acquire(this, backend().bufferLength());
}


Unison::PortDirection JackPort::directionFromFlags (JackPortFlags flags)
{
  if (flags & JackPortIsInput) {
    return Unison::Output;
  }
  if (flags & JackPortIsOutput) {
    return Unison::Input;
  }
  Q_ASSERT_X(0, "JackPort", "direction is neither Input or Output.");
  return (Unison::PortDirection)0;
}


JackPortFlags JackPort::flagsFromDirection (Unison::PortDirection dir)
{
  switch (dir) {
    case Unison::Input:
      return JackPortIsOutput;
    case Unison::Output:
    default:
      return JackPortIsInput;
  }
}


  } // Internal
} // Jack

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
