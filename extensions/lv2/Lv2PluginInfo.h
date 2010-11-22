/*
 * Lv2PluginInfo.h
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

#ifndef UNISON_LV2_PLUGIN_INFO_H
#define UNISON_LV2_PLUGIN_INFO_H

#include <unison/Plugin.h>
#include <unison/PluginInfo.h>
#include <unison/types.h>

#include <slv2/slv2.h>

namespace Lv2 {
  namespace Internal {

class Lv2World;

/**
 * A description of a LV2 plugin.  This descriptor allows us to query LV2 plugins without
 * actually instantiating them.
 */
class Lv2PluginInfo : public Unison::PluginInfo
{
  public:
    Lv2PluginInfo (Lv2World& world, SLV2Plugin plugin);
    Lv2PluginInfo (const Lv2PluginInfo& descriptor);

    Unison::PluginPtr createPlugin (Unison::nframes_t sampleRate) const;

  private:
    Lv2World& m_world;
    SLV2Plugin m_plugin;
};

  } // Internal
} // Lv2

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
