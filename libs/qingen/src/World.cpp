/*
 * World.cpp
 *
 * Copyright (c) 2011 Paul Giblock <pgib/at/users.sourceforge.net>
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

#include <QtGlobal>
#include <string.h>

#include "ingen/EngineBase.hpp"
#include "ingen/shared/Configuration.hpp"
#include "ingen/shared/World.hpp"
#include "ingen/shared/runtime_paths.hpp"

#include "World.hpp"

namespace QIngen {

World::World()
{
  // Prepare configuration for Ingen
  m_conf = new Ingen::Shared::Configuration();

  int         argc = 3;
  const char* argv_data[] = {"ingen", "-n", "Unison"};
  char**      argv = new char*[argc];
  for (int i = 0; i < argc; ++i) {
    argv[i] = strdup(argv_data[i]);
  }

  try {
    m_conf->parse(argc, argv);
  }
  catch (std::exception& e) {
    qFatal("ingen: %s\n", e.what());
  }

  m_world = new Ingen::Shared::World(m_conf, argc, argv);

  for (int i = 0; i < argc; ++i) {
    free(argv[i]);
  }
  delete[] argv;
}


World::~World()
{
  delete m_world;
  delete m_conf;
}


void World::setBundlePath(const char* path)
{
  Ingen::Shared::set_bundle_path(path);
}

bool World::loadModule(const char* name)
{
  return m_world->load_module(name);
}

void World::unloadModules()
{
  m_world->unload_modules();
}

SharedPtr<Ingen::EngineBase> World::engine()
{
  return m_world->local_engine();
}

SharedPtr<Ingen::ServerInterface> World::server()
{
  return m_world->engine();
}

} // namespace QIngen

// vim: tw=90 ts=8 sw=2 sts=4 et ci pi cin cino=l1,g0,+2s,\:0,(0,u0,U0,W2s
