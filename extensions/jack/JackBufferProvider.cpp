/*
 * JackBufferProvider.cpp
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

#include "JackBufferProvider.h"
#include "JackPort.h"

#include <unison/AudioBuffer.h>

#include <jack/jack.h>
#include <QDebug>

using namespace Unison;

namespace Jack {
  namespace Internal {

SharedBufferPtr JackBufferProvider::acquire (
    const JackPort* port, nframes_t nframes)
{
  // TODO: assert only called within process thread
  void* jackBuffer = jack_port_get_buffer(port->jackPort(), nframes);
  return new AudioBuffer( *this, nframes, jackBuffer );
}


SharedBufferPtr JackBufferProvider::acquire (PortType, nframes_t)
{
  qCritical() << "JackBufferProvider acquire called, programming error";
  return NULL;
}

SharedBufferPtr JackBufferProvider::zeroAudioBuffer () const
{
  qCritical() << "JackBufferProvider acquire called, programming error";
  return NULL;
}

  } // Internal
} // Jack

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
