/*
 * Engine.cpp
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

#include "Engine.hpp"

#include <ingen/shared/World.hpp>
#include <ingen/client/ClientStore.hpp>

namespace Core {

Engine::~Engine ()
{
  if (m_store) {
    delete m_store;
  }

  if (m_world) {
    delete m_world;
  }
}

void Engine::setIngenWorld (Ingen::Shared::World* world)
{
  m_world = world;
}


void Engine::setStore (Ingen::Client::ClientStore* store)
{
  m_store = store;
}

} // Core

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
