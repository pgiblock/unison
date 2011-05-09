/*
 * World.hpp
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

#ifndef QINGEN_WORLD_HPP
#define QINGEN_WORLD_HPP

#include <map>
#include <string>

#include "raul/SharedPtr.hpp"

namespace Ingen {
  class EngineBase;
  class ServerInterface;

  namespace Shared {
    class Configuration;
    class World;
  }
}


namespace QIngen {

/**
 * A client-side world of certain Ingen components.
 *
 * QIngen is a client interface. Therefore, it doesn't make sense to expose
 * many of Ingen's components.  This serves as a handle to an Ingen instance.
 */
class World
{
public:

  /**
   * TODO: Provide configuration options
   */
  World();
  ~World();

  static void setBundlePath(const char* path);
  bool loadModule(const char* name);
  void unloadModules();

  SharedPtr<Ingen::EngineBase>      engine();
  SharedPtr<Ingen::ServerInterface> server();

  // Stuff that can be exposed as needed:
  //virtual QSharedPointer<Ingen::Store> store();

  //virtual void set_jack_uuid(const std::string& uuid);
  //virtual std::string jack_uuid();
  //virtual Sord::World* rdf_world();
  //virtual SharedPtr<LV2URIMap> uris();
  //virtual bool run(const std::string& mime_type,
  //                 const std::string& filename);
  //virtual QSharedPointer<Serialisation::Serialiser> serialiser();
  //virtual QSharedPointer<Serialisation::Parser>     parser();
private:
  Ingen::Shared::World*         m_world;
  Ingen::Shared::Configuration* m_conf;
};

} // namespace QIngen

#endif // QINGEN_WORLD_HPP

// vim: tw=90 ts=8 sw=2 sts=4 et ci pi cin cino=l1,g0,+2s,\:0,(0,u0,U0,W2s
