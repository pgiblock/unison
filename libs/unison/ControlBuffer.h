/*
 * ControlBuffer.h
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

#ifndef UNISON_CONTROL_BUFFER_H
#define UNISON_CONTROL_BUFFER_H

#include "unison/Buffer.h"

namespace Unison
{

/**
 * ControlBuffer represents a buffer used by Ports with type = CONTROL_PORT.
 * ControlBuffers have a fixed size of 1 value.  They can be used to
 * communicate control-port values across connections, but a Port must
 * still maintain a shadow of the current-value should the buffer ever be
 * reassigned.  Additionally, a currentValue is important for saving
 * non-controlled values to a project file. */
class ControlBuffer : public Buffer
{
  public:
    /**
     * Construct a buffer under the specified provider.  This must be called
     * by a BufferProvider to ensure proper memory management. */
    ControlBuffer (BufferProvider& provider) :
      Buffer(provider),
      m_data(0.0f)
    {}

    ~ControlBuffer()
    {}

    /** @returns the type of buffer.  Always CONTROL_PORT. */
    PortType type () const
    {
      return CONTROL_PORT;
    }

    void* data()
    {
      return &m_data;
    }

    const void* data() const
    {
      return &m_data;
    }

    /** @returns The data as a float value for convenience */
    float value() const
    {
      return m_data;
    }

  protected:
    float m_data;
};

} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
