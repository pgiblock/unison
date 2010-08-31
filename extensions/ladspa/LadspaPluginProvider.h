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

#include "core/IPluginProvider.h"

#include <QMap>
#include <QString>

class QLibrary;

namespace Ladspa {
  namespace Internal {

/** Provides a database of LADSPA plug-ins. */
class LadspaPluginProvider : public Core::IPluginProvider
{
  Q_OBJECT
  public:
    LadspaPluginProvider();
    ~LadspaPluginProvider();

    QString displayName()
    {
      return tr("LADSPA Plugin Provider");
    }

    void discoverPlugins ();

    /** Describes the requested plugin.  
     *  @param plugin  The Name of the plugin to describe
     *  @return The Plugin Descriptor */
    Unison::PluginDescriptorPtr descriptor (const QString plugin);

  private:
    void discoverFromDirectory (const QString &path);
    int discoverFromLibrary (const QString &path);

    typedef QMap<long unsigned int, Unison::PluginDescriptorPtr> LadspaPluginDescriptorMap;
    LadspaPluginDescriptorMap m_descriptorMap;
};

  } // Internal
} // Ladspa

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
