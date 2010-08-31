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

#ifndef UNISON_JACK_PORT_H
#define UNISON_JACK_PORT_H

#include <unison/BackendPort.h>
#include <unison/ProcessingContext.h>
#include <jack/jack.h>

namespace Jack {
  namespace Internal {

class JackBufferProvider;
class JackBackend;

/**
 * Encapsulates a registered port of the jack-client. */
class JackPort : public Unison::BackendPort
{
  public:
    JackPort(JackBackend & backend, jack_port_t * port) :
        BackendPort(),
        m_backend(backend),
        m_port(port)
    {
    }

    Unison::Node *parent () const
    {
      return NULL;
    }

    JackBackend &backend () const
    {
      return m_backend;
    }


    QString id () const
    {
      return jack_port_short_name( m_port );
    }

    QString name () const
    {
      return jack_port_name( m_port );
    }

    Unison::PortDirection direction() const
    {
      JackPortFlags flags = (JackPortFlags)jack_port_flags( m_port );
      if (flags & JackPortIsInput)  { return Unison::OUTPUT;  }
      if (flags & JackPortIsOutput) { return Unison::INPUT; }
      Q_ASSERT_X(0, "JackPort", "direction is neither Input or Output.");
      return (Unison::PortDirection)0;
    }

    Unison::PortType type () const
    {
      return Unison::AUDIO_PORT;
    }

    float value () const
    {
      return 0.0f;
    }

    void setValue (float)
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

    bool isToggled () const
    {
      return false;
    }

    const QSet<Unison::Node* const> interfacedNodes () const;

    jack_port_t* jackPort () const
    {
      return m_port;
    }

    void connectToBuffer ();

  private:
    JackBackend& m_backend;
    jack_port_t* m_port;

    static JackBufferProvider * m_jackBufferProvider;
};

  } // Internal
} // Jack

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
