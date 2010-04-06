/*
 * BufferProvider.h
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

#ifndef BUFFER_PROVIDER_H
#define BUFFER_PROVIDER_H

#include <iostream>
#include <malloc.h>

#include <QStack>
#include <QSharedPointer>

#include "unison/Buffer.h"
#include "unison/types.h"

namespace Unison
{

class SharedBufferPtr;

class BufferProvider
{
  public:
    virtual ~BufferProvider ()
    {};

    virtual SharedBufferPtr acquire (PortType type, nframes_t nframes) = 0;
    virtual SharedBufferPtr zeroAudioBuffer () const = 0;

  protected:
    virtual void release (Buffer* buf) = 0;
    friend class SharedBufferPtr;
};



class SharedBufferPtr : public QSharedPointer<Buffer>
{
  public:
    SharedBufferPtr () :
      QSharedPointer<Buffer>()
    {}

    SharedBufferPtr (Buffer* buf) :
      QSharedPointer<Buffer>( buf, SharedBufferPtr::deleter )
    {}

  protected:
    static void deleter (Buffer * buf)
    {
      buf->m_provider.release(buf);
    }
};



class PooledBufferProvider : public BufferProvider
{
  public:
    PooledBufferProvider ();

    ~PooledBufferProvider()
    {}

    SharedBufferPtr zeroAudioBuffer () const;
    void setBufferLength (nframes_t nframes);
    nframes_t bufferLength ();

    SharedBufferPtr acquire (PortType type, nframes_t nframes);

  protected:
    void release (Buffer * buf);

    // TODO: Use something RT-safe, instead of QStack
    QStack<Buffer*> m_audioBuffers;
    QStack<Buffer*> m_controlBuffers;
    SharedBufferPtr m_zeroBuffer;
    nframes_t m_periodLength;
    int m_next;
};


} // Unison


#endif

// vim: et ts=8 sw=2 sts=2 noai
