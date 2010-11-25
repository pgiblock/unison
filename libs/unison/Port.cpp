/*
 * Port.cpp
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

#include "Port.h"

#include "BufferProvider.h"
#include "Commander.h"
#include "PortConnect.h"
#include "PortDisconnect.h"

#include <QDebug>

#define UNISON_BUFFER_LENGTH 1024

namespace Unison {

Port::Port () :
  Node(),
  m_buffer(NULL),
  m_connectedPorts()
{}


void Port::setBufferLength (nframes_t len)
{
  Q_UNUSED(len);
  // TODO: Forcefully resize the buffer we own. Who cares? changing
  // bufferlength won't happen in the middle of a song, realloc
  // should be safe.
}


void Port::connect (Port* other, BufferProvider& bp)
{
  Command* cmd = new Internal::PortConnect(this, other, bp);
  Internal::Commander::instance()->push(cmd);
}


void Port::disconnect (Port* other, BufferProvider& bp)
{
  Command* cmd = new Internal::PortDisconnect(this, other, bp);
  Internal::Commander::instance()->push(cmd);
}


bool Port::isConnected (Port* other) const
{
  return m_connectedPorts.contains(other);
}


const QSet<Node* const> Port::dependencies () const
{
  switch (direction()) {
    case Input:
    {
      QSet<Node* const> p;
      for (QSet<Port* const>::const_iterator i = m_connectedPorts.begin();
           i != m_connectedPorts.end(); ++i) {
        p.insert(*i);
      }
      return p;
    }

    case Output:
    {
      return interfacedNodes();
    }
  }
  return QSet<Node* const>();
}



const QSet<Node* const> Port::dependents () const {
  switch (direction()) {
    case Input:
    {
      return interfacedNodes();
    }

    case Output:
    {
      QSet<Node* const> p;
      for (QSet<Port* const>::const_iterator i = m_connectedPorts.begin();
           i != m_connectedPorts.end(); ++i) {
        p.insert(*i);
      }
      return p;
    }
  }
  return QSet<Node* const>();
}


void Port::acquireBuffer (BufferProvider& provider)
{
  switch (direction()) {
    case Input:
      acquireInputBuffer(provider, UNISON_BUFFER_LENGTH);
      break;

    case Output:
      acquireOutputBuffer(provider, UNISON_BUFFER_LENGTH);
      break;
  }
}


void Port::acquireInputBuffer (BufferProvider& provider, nframes_t len)
{
  int numConnections = dependencies().count();
  switch (numConnections) {
    case 0:
      if (type() == AudioPort) {
        // Use silence
        m_buffer = provider.zeroAudioBuffer();
        return;
      }
      break;
    case 1:
    {
      // Use the other port's buffer
      // type should match due to validation on connect
      Port* other = static_cast<Port*>( *(dependencies().begin()) );
      m_buffer = other->buffer();
      break;
    }
    default:
      qFatal("Internal mixing is not yet supported");
      return;
  }

  if (!m_buffer) {
    // Return internal port
    m_buffer = provider.acquire(type(), len);
    updateBufferValue();
  }
}


void Port::acquireOutputBuffer (BufferProvider& provider, nframes_t len)
{
  // TODO: if there are no connections, we should probably just connect to some shared
  // bit-bucket buffer some place to save memory on plugins with sparsly used ports

  if (!m_buffer) {
    m_buffer = provider.acquire(type(), len);
  }
}


void Port::updateBufferValue ()
{
  //qDebug() << "Updating value of private buffer for port" << name();
  if (buffer() && type() == ControlPort) {
    // Verified ControlPort, so data must be a single float
    float* data = static_cast<float*>( buffer()->data() );
    data[0] = value();
  }
}


} // Unison

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
