/*
 * Lv2PluginInfo.cpp
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

#include "Lv2PluginInfo.h"
#include "Lv2Plugin.h"

#include <QDebug>
#include <QSet>

using namespace Unison;

namespace Lv2 {
  namespace Internal {


Lv2PluginInfo::Lv2PluginInfo (Lv2World& world, SLV2Plugin plugin) :
  PluginInfo(),
  m_world(world),
  m_plugin(plugin)
{

  SLV2Value data;

  setUniqueId( QString( slv2_value_as_uri( slv2_plugin_get_uri( plugin ) ) ) );

  data = slv2_plugin_get_name( plugin );
  setName( QString( slv2_value_as_string( data ) ) );
  slv2_value_free( data );

  data = slv2_plugin_get_author_name( plugin );
  if (data) {
    setAuthorName( QString( slv2_value_as_string( data ) ) );
    slv2_value_free( data );
  }

  setAudioInputCount( slv2_plugin_get_num_ports_of_class(
        plugin, world.inputClass, world.audioClass, NULL ) );

  setAudioOutputCount( slv2_plugin_get_num_ports_of_class( plugin,
        world.outputClass, world.audioClass, NULL ) );
}


Lv2PluginInfo::Lv2PluginInfo (const Lv2PluginInfo& d) :
  PluginInfo(d),
  m_world(d.m_world),
  m_plugin(d.m_plugin)
{}


PluginPtr Lv2PluginInfo::createPlugin (nframes_t sampleRate) const
{
  return PluginPtr( new Lv2Plugin( m_world, m_plugin, sampleRate ) );
}

  } // Internal
} // Lv2

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
