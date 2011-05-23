/*
 * JackDriverProvider.cpp
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


#include <QDebug>

#include "JackDriverProvider.hpp"
#include "ingen/shared/World.hpp"

#include "extensionsystem/ExtensionManager.hpp"
#include "core/Engine.hpp"

using namespace ExtensionSystem;

namespace Jack {
  namespace Internal {

bool JackDriverProvider::loadDriver ()
{
  ExtensionManager* em = ExtensionManager::instance();
  Core::Engine* engine = em->getObject<Core::Engine>();

  Ingen::Shared::World* world = engine->ingenWorld();
  if (world->local_engine()) {
    return world->load_module("jack");
  }
  return false;
}

  } // Internal
} // Jack


// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
