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

#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>

namespace Lv2 {
  namespace Internal {

/** Provides a database of LV2 plug-ins.  Upon instantiation,
 *  it loads all of the plug-ins found in the LV2_PATH environmental variable
 *  and stores their access Infos according in a dictionary keyed on
 *  the filename the plug-in was loaded from and the label of the plug-in.
 *  Can be retrieved by using lv2_key_t (which is really just the LV2 URI) */
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

    Unison::PluginInfoPtr info (const QString& plugin);

  private:
    void addLv2Plugin (SLV2Plugin _plugin);

    /**
     * the lv2World. If we want to provide Lv2-support to other Extensions, then we
     * will need to make expose lv2World
     */
    Lv2World m_lv2World;

    /* We just hold on to all Infos here. We will probably store our information in an SQL
     * or RDF database later for quicker searching etc..  However, we will still need to
     * keep a Map of all plugins loaded this session - at least to encourage reuse of the
     * already constructed PluginInfos */
    typedef QMap<QString, Unison::PluginInfoPtr> Lv2PluginInfoMap;
    Lv2PluginInfoMap m_lv2InfoMap;
};

  } // Internal
} // Lv2

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
