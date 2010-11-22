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

namespace Unison {
  class BufferProvider;
}

namespace Jack {
  namespace Internal {

class JackBackend;

/**
 * Encapsulates a registered port of the jack-client. */
class JackPort : public Unison::BackendPort
{
  public:
    JackPort (JackBackend& backend, const QString &name, Unison::PortDirection direction);

    bool registerPort ();

    bool isRegistered () const
    {
      return m_port;
    }

    Unison::Node* parent () const;

    JackBackend& backend () const
    {
      return m_backend;
    }


    QString id () const
    {
      return m_id;
    }

    QString name () const
    {
      return QString::fromLocal8Bit( jack_port_name(m_port) );
    }

    Unison::PortDirection direction() const
    {
      return m_direction;
    }

    Unison::PortType type () const
    {
      return Unison::AudioPort;
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

    void activate (Unison::BufferProvider& bp);
    void preProcess ();
    void postProcess ();

    static Unison::PortDirection directionFromFlags (JackPortFlags flags);
    
    /**
     * Build Jack flags, currently just direction. Which, is reversed since
     * our Ports' directions are relative to Unison's connections but
     * jack_port_t's direction is relative to Jack's graph. */
    static JackPortFlags  flagsFromDirection (Unison::PortDirection dir);

  private:
    JackBackend& m_backend;
    jack_port_t* m_port;

    // Need to shadow ID and direction so we can re-register
    QString m_id;
    Unison::PortDirection m_direction;
};

  } // Internal
} // Jack

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
