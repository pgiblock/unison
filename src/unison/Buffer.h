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


class Buffer : PRG::Uncopyable
{
  public:
    Buffer (BufferProvider& provider) :
      m_provider(provider)
    {}

    virtual ~Buffer ()
    {};

    virtual PortType type () = 0;

    virtual void* data () = 0;

    virtual const void* data () const = 0;

  protected:
    BufferProvider& m_provider;

    friend class SharedBufferPtr;
};

} // Unison


#endif

// vim: et ts=8 sw=2 sts=2 noai
