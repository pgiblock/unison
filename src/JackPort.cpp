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

#include <jack/jack.h>
#include "unison/JackEngine.h"
#include "unison/JackPort.h"

namespace Unison {

  const QSet<Node*> JackPort::dependencies () const
  {
    const char** name = jack_port_get_connections( m_port );
    uint32_t count = m_engine.myPortCount();
    QSet<Node*> dependencies;

    switch (direction())
    {
    case INPUT:
      // Return internal connections
      break;

    case OUTPUT:
      // Within all connected ports
      // TODO: move this routine into JackEngine
      while (name != NULL)
      {
        // See if we own the port
        for (uint32_t i = 0; i < count; ++i) {
          JackPort* port = m_engine.myPort(i);
          if (port->fullName() == *name)
          {
            dependencies += port;
          }
        }
      }
      break;
    }
    return dependencies;
  }


  const QSet<Node*> JackPort::dependents () const
  {
    const char** name = jack_port_get_connections( m_port );
    uint32_t count = m_engine.myPortCount();
    QSet<Node*> dependents;

    switch (direction())
    {
    case INPUT:
      // Within all connected ports
      while (name != NULL)
      {
        // See if we own the port
        for (uint32_t i = 0; i < count; ++i) {
          JackPort* port = m_engine.myPort(i);
          if (port->fullName() == *name)
          {
            dependents += port;
          }
        }
      }
      break;

    case OUTPUT:
      // Return internal connections
      break;
    }
    return dependents;
  }

} // Unison

// vim: et ts=8 sw=2 sts=2 noai
