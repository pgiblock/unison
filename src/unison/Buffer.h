/*
 * Buffer.h
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

#ifndef BUFFER_H
#define BUFFER_H

#include "prg/Uncopyable.h"
#include "unison/types.h"

namespace Unison
{

class BufferProvider;

/** The buffer class represents a data buffer used to communicate between two
 *  ports.  This could potentially be used within Processors, but right now
 *  usage should be restricted to Ports.
 *
 *  Buffers cannot be copied.  Buffers are managed by a BufferProvider; this
 *  allows buffers to be reused in a transparent fashion.  Clients should never
 *  delete a buffer, they are automatically freed by the BufferProvider though
 *  the SharedBufferPtr smart pointer. */
class Buffer : PRG::Uncopyable
{
  public:
    /** Construct a buffer and pass ownership to the specified BufferProvider.
     *  This constructor should generally only be called by BufferProvider */
    Buffer (BufferProvider& provider) :
      m_provider(provider)
    {}

    virtual ~Buffer ()
    {};

    /** @returns the type of buffer.  Buffers are designed to only work with
     *  a particular kind of port and the PortType must match. */
    virtual PortType type () const = 0;

    /** @returns raw data for this buffer.  Subclasses should provide more
     *  useful accessors, but this can be used to access the data in a
     *  generic way. */
    virtual void* data () = 0;

    /** @returns constant raw data.  Like data() but does not require
     *  non-const access to the buffer. */
    virtual const void* data () const = 0;

  protected:
    BufferProvider& m_provider;     ///< The owning BufferProvider

    friend class SharedBufferPtr;
};

} // Unison


#endif

// vim: et ts=8 sw=2 sts=2 noai
