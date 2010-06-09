/*
 * JackBufferProvider.h
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

#ifndef JACK_BUFFER_PROVIDER_H
#define JACK_BUFFER_PROVIDER_H

#include <jack/jack.h>

#include "unison/BufferProvider.h"

namespace Jack {
namespace Internal {

class JackPort;

/**
 * JackBufferProvider acquires buffers that backed by memory managed by Jack.
 * A new acquire() function is used to return these special Buffers.  Calling
 * the overloaded acquire() function is an error. */
class JackBufferProvider : public Unison::BufferProvider
{
  public:
    JackBufferProvider ()
    {}

    ~JackBufferProvider()
    {}

    /**
     * Called by Jack's process callback to connect JackPorts to the
     * appropriate buffer.  This must be called in the processing thread.
     * @returns wrapped buffer provided by Jack for the given port */
    Unison::SharedBufferPtr acquire (const JackPort * port, Unison::nframes_t nframes);

    /**
     * Results in an error as JackBufferProvider must not be used by Ports
     * other than JackPort */
    Unison::SharedBufferPtr acquire (Unison::PortType, Unison::nframes_t);

    /**
     * Results in an error as JackBufferProvider must not be used by Ports
     * other than JackPort */
    Unison::SharedBufferPtr zeroAudioBuffer () const;

  protected:
    void release (Unison::Buffer * buf)
    {
      delete buf;
    }
};

} // Internal
} // Jack

#endif

// vim: et ts=8 sw=2 sts=2 noai
