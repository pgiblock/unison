/*
 * Engine.hpp
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

#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include "Core_global.hpp"

#include <unison/types.hpp>

#include <QObject>

namespace Unison {
  class Backend;
  class BufferProvider;
}

namespace Core {

class CORE_EXPORT Engine : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(Engine);

  public:
    Engine () {};
    virtual ~Engine () {};

    inline static Unison::Backend* backend ()
    {
      return m_backend;
    }

    inline static Unison::BufferProvider* bufferProvider ()
    {
      return m_bufferProvider;
    }

    /**
     * Not public API - do not call
     */
    static void setBackend (Unison::Backend *backend);

    /**
     * Not public API - do not call
     */
    static void setBufferProvider (Unison::BufferProvider* bufferProvider);

  private:
    static Unison::Backend* m_backend;
    static Unison::BufferProvider* m_bufferProvider;
};

} // Core

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
