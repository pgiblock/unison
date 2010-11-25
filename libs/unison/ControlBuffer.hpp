/*
 * ControlBuffer.hpp
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

#ifndef UNISON_CONTROL_BUFFER_HPP_
#define UNISON_CONTROL_BUFFER_HPP_

#include "Buffer.hpp"

namespace Unison {

/**
 * ControlBuffer represents a buffer used by a @c Port with type ControlPort.
 * ControlBuffers have a fixed size of 1 value.  They can be used to communicate
 * control-port values across connections, but a Port must still maintain a shadow of the
 * current-value should the buffer ever be reassigned.  Additionally, a current @c value
 * is important for saving non-controlled values (from unconnected Ports) to a project
 * file.
 */
class ControlBuffer : public Buffer
{
  public:
    /**
     * Construct a buffer under the specified provider.  This must be called by a
     * BufferProvider to ensure proper memory management.
     */
    ControlBuffer (BufferProvider& provider) :
      Buffer(provider, ControlPort),
      m_data(0.0f)
    {}

    ~ControlBuffer()
    {}

    void* data()
    {
      return &m_data;
    }

    const void* data() const
    {
      return &m_data;
    }

    /**
     * @returns The data as a float value for convenience
     */
    inline float value() const
    {
      return m_data;
    }

  private:
    float m_data; ///< Current (shadowed) value
};

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
