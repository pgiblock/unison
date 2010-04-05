/*
 * AudioBuffer.h
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

#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include <iostream>
#include <malloc.h>

#include "unison/Buffer.h"

namespace Unison {


class AudioBuffer : public Buffer {
  public:
    AudioBuffer (BufferProvider& provider, nframes_t length) :
      Buffer(provider),
      m_length(length),
      m_ownsData(true)
    {
      m_data = (float*)malloc(length * sizeof(float));
      for (int i=0; i< length; ++i) {
        m_data[i] = 0.0f;
      }
    }

    AudioBuffer (BufferProvider& provider, nframes_t length, void * data) :
      Buffer(provider),
      m_length(length),
      m_ownsData(false)
    {
      m_data = (float*) data;
    }

    ~AudioBuffer ()
    {
      if (m_ownsData) { free(m_data); }
    }

    PortType type ()
    {
      return AUDIO_PORT;
    }

    nframes_t length ()
    {
      return m_length;
    }

    void setLength (nframes_t len)
    {
      if (m_ownsData) {
        m_length = len;
        free(m_data);
        m_data = (float*)malloc(len * sizeof(float));
      }
    }

    void* data()
    {
      return m_data;
    }

    const void* data() const
    {
      return m_data;
    }

  protected:
    int m_length;
    float* m_data;
    bool m_ownsData;
};

} // Unison


#endif

// vim: et ts=8 sw=2 sts=2 noai
