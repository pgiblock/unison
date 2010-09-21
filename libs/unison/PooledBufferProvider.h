/*
 * PooledBufferProvider.h
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

#ifndef UNISON_POOLED_BUFFER_PROVIDER_H_
#define UNISON_POOLED_BUFFER_PROVIDER_H_

#include "BufferProvider.h"

#include <QtCore/QStack>

namespace Unison {
  
  class Buffer;

/**
 * PooledBufferProvider is the 'default' implementation for BufferProviders.  It isn't as
 * smart as it could be, but it at least provides reuse of released buffers.  The main
 * benefit of this class, besides memory reuse, is performing RT-unsafe mallocs outside of 
 * the processing thread.
 */
class PooledBufferProvider : public BufferProvider
{
  public:
    PooledBufferProvider ();

    ~PooledBufferProvider()
    {}

    /**
     * Change the buffer length of this BufferProvider.  What this means exactly isn't
     * well defined yet.  We need to finish buffersize and samplerate changes first
     * @param nframes the new buffer length
     */
    void setBufferLength (nframes_t nframes);

    /**
     * @returns the current buffer length used by this BufferProvider
     */
    nframes_t bufferLength ();

    SharedBufferPtr zeroAudioBuffer () const;

    SharedBufferPtr acquire (PortType type, nframes_t nframes);

  protected:
    void release (Buffer* buf);

  private:
    // TODO: Use something RT-safe, instead of QStack
    QStack<Buffer*> m_audioBuffers;
    QStack<Buffer*> m_controlBuffers;
    SharedBufferPtr m_zeroBuffer;
    nframes_t m_periodLength;
    int m_next;
};

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
