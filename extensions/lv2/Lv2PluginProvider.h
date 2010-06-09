/*
 * Lv2PluginProvider.h
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

#ifndef LV2_PLUGIN_PROVIDER_H
#define LV2_PLUGIN_PROVIDER_H

#include "Lv2Plugin.h"
#include "core/IPluginProvider.h"

#include <slv2/world.h>
#include <slv2/plugin.h>
#include <slv2/scalepoint.h>

#ifdef HAVE_SLV2_SCALEPOINTS_H
#include <slv2/scalepoints.h>
#endif

#include <assert.h>
#include <stdint.h>

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Lv2 {
namespace Internal {

/** Provides a database of LV2 plug-ins.  Upon instantiation,
 *  it loads all of the plug-ins found in the LV2_PATH environmental variable
 *  and stores their access descriptors according in a dictionary keyed on
 *  the filename the plug-in was loaded from and the label of the plug-in.
 *  Can be retrieved by using lv2_key_t (which is really just the LV2 URI) */
//class Lv2PluginProvider : public Core::IPluginProvider
class Lv2PluginProvider : public Core::IPluginProvider
{
  Q_OBJECT
  public:
    Lv2PluginProvider();
    ~Lv2PluginProvider();

    QString displayName()
    {
      return tr("Lv2 Plugin Provider");
    }

    /** Describes the requested plugin.  Right now, only LV2 plugins are
     *  supported. This could take a param, or split into multiple functions
     *  for distinguishing between LADSPA, VST, etc..
     *  @param plugin  The URI of the plugin to describe
     *  @return The Plugin Descriptor */
    Unison::PluginDescriptorPtr descriptor (const QString plugin);

  private:
    void addLv2Plugin (SLV2Plugin _plugin);

    Lv2World m_lv2World;

    typedef QMap<QString, Unison::PluginDescriptorPtr> Lv2PluginDescriptorMap;
    Lv2PluginDescriptorMap m_lv2DescriptorMap;
};

} // Internal
} // Lv2

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
