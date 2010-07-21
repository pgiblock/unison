/*
 * Lv2PluginProvider.cpp
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

#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

#include <math.h>

#include "Lv2PluginProvider.h"
  
using namespace Unison;

namespace Lv2 {
  namespace Internal {

Lv2PluginProvider::Lv2PluginProvider() :
  m_lv2World(),
  m_lv2DescriptorMap()
{
  qDebug( "Initializing Lv2 Plugin Provider" );
  if (m_lv2World.world == NULL) {
    qWarning(  "Failed to Initialize slv2_world" );
    return;
  }

  // Do Lv2-Plugin discovery

  // slv2_world_load_all( m_world ); // No special path apart from LV2_PATH
  SLV2Plugins lv2PluginList = slv2_world_get_all_plugins( m_lv2World.world );
  size_t lv2PluginListSize = slv2_plugins_size( lv2PluginList );

  // TODO-NOW: Allow user to choose multiple paths
  for (unsigned i=0; i < lv2PluginListSize; ++i) {
    SLV2Plugin p = slv2_plugins_get_at( lv2PluginList, i );
    addLv2Plugin( p );
  }

  qDebug() << "Found" << lv2PluginListSize << "Lv2 plugins.";

  slv2_plugins_free( m_lv2World.world, lv2PluginList );
  qDebug( "Done initializing Lv2 Plugin Provider" );
}


Lv2PluginProvider::~Lv2PluginProvider ()
{}


PluginDescriptorPtr Lv2PluginProvider::descriptor (const QString uniqueId)
{
  // returns null on fail
  return m_lv2DescriptorMap.value( uniqueId );
}


/*
void Lv2Manager::ensureLV2DataExists (Lv2PluginDescriptor* desc) {
  if (desc->plugin == NULL) {
    printf( " Need to load actual plugin data for '%s'\n",
            (desc->uri).toAscii().constData() );

    // use uri to get data
    SLV2Value uri = slv2_value_new_uri(
        m_lv2World.world, (desc->uri).toAscii().constData() );

    desc->plugin = slv2_plugins_get_by_uri( m_pluginList, uri );
    slv2_value_free( uri );

    // Still empty??
    if( desc->plugin == NULL ) {
            printf( " Failed to load actual plugin data for '%s'\n", (desc->uri).toAscii().constData() );
    }
  }
}
*/


void Lv2PluginProvider::addLv2Plugin (SLV2Plugin plugin)
{
  QString key = slv2_value_as_uri( slv2_plugin_get_uri( plugin ) );

  if (m_lv2DescriptorMap.contains( key )) {
    return;
  }

  PluginDescriptorPtr descriptor(new Lv2PluginDescriptor(m_lv2World, plugin));

  // This always seems to return 'Plugin', which isn't so useful to us
  //	SLV2PluginClass pclass = slv2_plugin_get_class( _plugin );
  //	SLV2Value label = slv2_plugin_class_get_label( pclass );
  //	printf( "Plugin Class is : '%s'\n", slv2_value_as_string( label ) );

  //printf("  Audio (input, output)=(%d,%d)\n",
  //        descriptor->audioInputCount(), descriptor->audioOutputCount());

  m_lv2DescriptorMap.insert(key, descriptor);

  //printf("  Type=%d\n", (int)descriptor->type());
}

  } // Internal
} // Lv2

// vim: ts=8 sw=2 sts=2 et sta noai
