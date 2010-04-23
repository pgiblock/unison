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

#include "unison/BufferProvider.h" // for JBP
#include "unison/Port.h"
#include "unison/ProcessingContext.h"

namespace Unison
{

class JackPort;
class JackEngine;

class JackBufferProvider : public BufferProvider
{
  public:
    JackBufferProvider ()
    {}

    ~JackBufferProvider()
    {}

    SharedBufferPtr acquire (const JackPort * port, nframes_t nframes);

    SharedBufferPtr acquire (PortType, nframes_t);
    SharedBufferPtr zeroAudioBuffer () const;

  protected:
    void release (Buffer * buf)
    {
      delete buf;
    }
};



class JackPort : public Port
{
  public:
    JackPort(JackEngine & engine, jack_port_t * port) :
        Port(),
        m_engine(engine),
        m_port(port)
    {
    }

    QString name () const
    {
      return jack_port_short_name( m_port );
    }

    QString fullName () const
    {
      return jack_port_name( m_port );
    }

    PortDirection direction() const
    {
      JackPortFlags flags = (JackPortFlags)jack_port_flags( m_port );
      if (flags & JackPortIsInput)  { return OUTPUT;  }
      if (flags & JackPortIsOutput) { return INPUT; }
      Q_ASSERT_X(0, "JackPort", "direction is neither Input or Output.");
      return (PortDirection)0;
    }

    PortType type () const
    {
      return AUDIO_PORT;
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

    const QSet<Node* const> interfacedNodes () const;

    jack_port_t* jackPort () const
    {
      return m_port;
    }

    void connectToBuffer (BufferProvider &)
    {
      // TODO use a callback for buffer-size (more JACK CBs in general.)
      nframes_t size = 1024; //jack_get_buffer_size(m_engine.jackClient());
      m_buffer = m_jackBufferProvider->acquire(this, size);
    }

  private:
    JackEngine& m_engine;
    jack_port_t* m_port;

    static JackBufferProvider * m_jackBufferProvider;
};

} // Unison

#endif // JACK_PORT_H

// vim: et ts=8 sw=2 sts=2 noai
