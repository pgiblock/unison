/*
 * AudioBuffer.hpp
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

#ifndef UNISON_AUDIO_BUFFER_HPP_
#define UNISON_AUDIO_BUFFER_HPP_

#include "Buffer.hpp"

#include <malloc.h>

namespace Unison {

/**
 * AudioBuffer is a Buffer designed specifically for Audio.  Can be used to connect two
 * Ports with type=AudioPort.  AudioBuffer can be constructed two ways, details are
 * available in the constructor documentation.
 */
class AudioBuffer : public Buffer
{
  public:
    /**
     * Construct a new AudioBuffer and pass ownership to the specified BufferProvider.
     * This constructor should only be called by the BufferProvider when the pool
     * underruns.  This function itself is not RT-safe since malloc is called immediately.
     * The buffer is initialized to silence (all zero).
     * @param provider The BufferProvider to own this Buffer
     * @param length The length, in frames, of the Buffer
     */
    AudioBuffer (BufferProvider& provider, nframes_t length) :
      Buffer(provider, AudioPort),
      m_length(length),
      m_ownsData(true)
    {
      m_data = new float[length];
      for (nframes_t i=0; i< length; ++i) {
        m_data[i] = 0.0f;
      }
    }


    /**
     * Constructs an AudioBuffer from existing data.  The data itself is not managed by
     * AudioBuffer, that is, the client is still responsible for cleaning up the data.
     * AudioBuffer itself, on the otherhand, will be deleted by the Provider when no more
     * references exist.  This is used to provide an AudioBuffer over data which Unison
     * does not manage, for example, memory returned by jack_port_get_buffer().
     * @param provider The BufferProvider to own this Buffer
     * @param length The length, in frames, of the pre-existing Buffer
     * @param data pointer to the data
     * @deprecated
     */
    AudioBuffer (BufferProvider& provider, nframes_t length, void* data) :
      Buffer(provider, AudioPort),
      m_length(length),
      m_ownsData(false)
    {
      m_data = static_cast<float*>(data);
    }


    ~AudioBuffer ()
    {
      if (m_ownsData) {
        delete[] m_data;
      }
    }


    /**
     * @returns the length of the buffer in frames
     */
    inline nframes_t length () const
    {
      return m_length;
    }


    /**
     * Set the length of the buffer - currently unsupported.
     * @param len The new length, in samples
     */
    void setLength (nframes_t len)
    {
      if (m_ownsData) {
        m_length = len;
        delete[] m_data;
        m_data = new float[len];
      }
    }


    void* data ()
    {
      return m_data;
    }


    const void* data () const
    {
      return m_data;
    }

  private:
    int m_length;       ///< The length of the data buffer
    float* m_data;      ///< Pointer to actual data
    bool m_ownsData;    ///< true if we are responsible for freeing data
};

} // Unison


#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
