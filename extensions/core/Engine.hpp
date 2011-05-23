/*
 * Engine.hpp
 *
 * Copyright (c) 2010-2011 Paul Giblock <pgib/at/users.sourceforge.net>
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

#include <QObject>

namespace Ingen {
namespace Client {
class ClientStore;
} // namespace Client

namespace Shared {
class World;
} // namespace Shared
} // namespace Ingen

namespace Core {

class CORE_EXPORT Engine : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(Engine)

  public:
    Engine () {};
    virtual ~Engine ();

    inline Ingen::Shared::World* ingenWorld ()
    {
      return m_world;
    }

    inline Ingen::Client::ClientStore* store ()
    {
      return m_store;
    }

    /**
     * Not public API - do not call
     */
    void setIngenWorld (Ingen::Shared::World* world);

    /**
     * Not public API - do not call
     */
    void setStore (Ingen::Client::ClientStore* store);

  private:
    Ingen::Shared::World*       m_world;
    Ingen::Client::ClientStore* m_store;
};

} // Core

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
