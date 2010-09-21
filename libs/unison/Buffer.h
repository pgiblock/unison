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

#ifndef UNISON_BUFFER_H_
#define UNISON_BUFFER_H_

#include "types.h"

#include <QtCore/QtGlobal>

namespace Unison {

  class BufferProvider;

/**
 * The buffer class represents a data buffer used to communicate between two
 * ports.  This could potentially be used within Processors, but right now
 * usage should be restricted to Ports.
 *
 * Buffers cannot be copied.  Buffers are managed by a BufferProvider; this
 * allows buffers to be reused in a transparent fashion.  If copying ends up
 * being a required feature, then the Buffer *could* call a new
 * BufferProvider.clone() function to duplicate the buffer. Clients must never
 * delete a buffer, they are automatically freed by the BufferProvider through
 * the SharedBufferPtr smart pointer.
 */
class Buffer
{
  Q_DISABLE_COPY(Buffer)
  public:
    /**
     * This constructor, at the moment, should generally only be called by BufferProvider
     * @param provider The BufferProvider to pass ownership to
     */
    Buffer (BufferProvider& provider, PortType type) :
      m_provider(provider),
      m_type(type)
    {}

    virtual ~Buffer ()
    {};

    /**
     * @returns the type of buffer.  Buffers are designed to only work with a particular
     * kind of port and the PortType must match.
     */
    PortType type () const
    {
      return m_type;
    }

    /**
     * @returns raw data for this buffer.  Subclasses should provide more useful
     * accessors, but this can be used to access the data in a generic way.
     */
    virtual void* data () = 0;
    virtual const void* data () const = 0;

  private:
    BufferProvider& m_provider;     ///< The owning BufferProvider
    PortType m_type;                ///< Fixed type of the buffer

    friend class SharedBufferPtr;
};

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
