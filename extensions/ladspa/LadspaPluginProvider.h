/*
 * LadspaPluginProvider.h
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

#ifndef LADSPA_PLUGIN_PROVIDER_H
#define LADSPA_PLUGIN_PROVIDER_H

#include "LadspaPlugin.h"
#include "core/IPluginProvider.h"

#include <slv2/world.h>
#include <slv2/plugin.h>
#include <slv2/scalepoint.h>

#ifdef HAVE_SLV2_SCALEPOINTS_H
#include <slv2/scalepoints.h>
#endif

#include <assert.h>
#include <stdint.h>

#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>

namespace Ladspa {
  namespace Internal {

/** Provides a database of LADSPA plug-ins.
 * TODO: Describe what you are doing to query all the ladspa plugin names etc,
 *       what is being initialized? */
class LadspaPluginProvider : public Core::IPluginProvider
{
  Q_OBJECT
  public:
    LadspaPluginProvider();
    ~LadspaPluginProvider();

    QString displayName()
    {
      return tr("Ladspa Plugin Provider");
    }

    /** Describes the requested plugin.  
     *  @param plugin  The Name of the plugin to describe
     *  @return The Plugin Descriptor */
    Unison::PluginDescriptorPtr descriptor (const QString plugin);

  private:
    void addLadspaPlugin (SLV2Plugin _plugin);

    typedef QMap<QString, Unison::PluginDescriptorPtr> LadspaPluginDescriptorMap;
    LadspaPluginDescriptorMap m_ladspaDescriptorMap;
};

  } // Internal
} // Ladspa

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
