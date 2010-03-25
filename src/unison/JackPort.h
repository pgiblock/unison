/*
 * JackPort.h
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

#ifndef JACK_PORT_H
#define JACK_PORT_H

#include <jack/jack.h>

#include "unison/Port.h"

namespace Unison {

  class JackEngine;

  class JackPort : public Port
  {
  public:
    JackPort(JackEngine & engine, jack_port_t * port) :
        Port(),
        m_engine(engine),
        m_port(port)
    {
    }

    QString name (size_t maxLength) const
    {
      return jack_port_short_name( m_port );
    }

    QString fullName () const
    {
      return jack_port_name( m_port );
    }

    Port::Direction direction() const
    {
      JackPortFlags flags = (JackPortFlags)jack_port_flags( m_port );
      if (flags & JackPortIsInput)  { return Port::INPUT;  }
      if (flags & JackPortIsOutput) { return Port::OUTPUT; }
      // TODO: It is a programmer error to reach this line
    }

    Type type () const
    {
      return Port::AUDIO; // TODO!
    }

    void connectToBuffer (float * buf)
    {
      // TODO
    }

    float value () const
    {
      return 0.0f;
    }

    void setValue (float value)
    {
    }

    float defaultValue () const
    {
      return 0.0f;
    }

    bool isBounded () const
    {
      return false;
    }

    float minimum () const
    {
      return 0.0f;
    }

    float maximum () const
    {
      return 0.0f;
    }

    bool isToggled () const{
      return false;
    }

    const QSet<Node*> dependencies () const;

    const QSet<Node*> dependents () const;

  private:
    JackEngine& m_engine;
    jack_port_t* m_port;
  };


} // Unison


#endif // JACK_PORT_H

// vim: et ts=8 sw=2 sts=2 noai
