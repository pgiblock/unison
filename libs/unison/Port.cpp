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

#include "unison/Port.h"
#include "unison/PortConnect.h"

#include "unison/Commander.h"

namespace Unison {

Port::Port () :
  Node(),
  m_buffer(NULL),
  m_connectedPorts()
{}


void Port::setBufferLength (BufferProvider &bp, nframes_t len)
{
  // TODO: Forcefully resize the buffer we own. Who cares? changing
  // bufferlength won't happen in the middle of a song, realloc
  // should be safe.
}


void Port::connect (Port* other)
{
  Internal::PortConnect *cmd = new Internal::PortConnect(this, other);
  Internal::Commander::instance()->push(cmd);
}


void Port::disconnect (Port* other)
{
  m_connectedPorts -= other;
  other->m_connectedPorts -= this;
}


bool Port::isConnected (Port* other) const
{
  return m_connectedPorts.contains(other);
}


const QSet<Node* const> Port::dependencies () const
{
  switch (direction()) {
    case INPUT:
    {
      QSet<Node* const> p;
      for (QSet<Port* const>::const_iterator i = m_connectedPorts.begin();
          i != m_connectedPorts.end(); ++i) {
        p.insert(*i);
      }
      return p;
    }

    case OUTPUT:
      return interfacedNodes();
  }
}



const QSet<Node* const> Port::dependents () const {
  switch (direction()) {
    case INPUT:
    {
      return interfacedNodes();
    }

    case OUTPUT:
    {
      QSet<Node* const> p;
      for (QSet<Port* const>::const_iterator i = m_connectedPorts.begin();
          i != m_connectedPorts.end(); ++i) {
        p.insert(*i);
      }
      return p;
    }
  }
}

} // Unison

// vim: ts=8 sw=2 sts=2 et sta noai
