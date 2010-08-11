/*
 * LadspaPluginProvider.cpp
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

#include "LadspaPluginProvider.h"
  
using namespace Unison;

namespace Ladspa {
  namespace Internal {

LadspaPluginProvider::LadspaPluginProvider() :
  m_ladspaWorld(),
  m_ladspaDescriptorMap()
{
  qDebug( "Initializing Ladspa Plugin Provider" );
  if (m_ladspaWorld.world == NULL) {
    qWarning(  "Failed to Initialize slv2_world" );
    return;
  }

  // Do Ladspa-Plugin discovery

  // slv2_world_load_all( m_world ); // No special path apart from LADSPA_PATH
  SLV2Plugins ladspaPluginList = slv2_world_get_all_plugins( m_ladspaWorld.world );
  size_t ladspaPluginListSize = slv2_plugins_size( ladspaPluginList );

  // TODO-NOW: Allow user to choose multiple paths
  for (unsigned i=0; i < ladspaPluginListSize; ++i) {
    SLV2Plugin p = slv2_plugins_get_at( ladspaPluginList, i );
    addLadspaPlugin( p );
  }

  qDebug() << "Found" << ladspaPluginListSize << "Ladspa plugins.";

  slv2_plugins_free( m_ladspaWorld.world, ladspaPluginList );
  qDebug( "Done initializing Ladspa Plugin Provider" );
}


LadspaPluginProvider::~LadspaPluginProvider ()
{}


PluginDescriptorPtr LadspaPluginProvider::descriptor (const QString uniqueId)
{
  // returns null on fail
  return m_ladspaDescriptorMap.value( uniqueId );
}


/*
void LadspaManager::ensureLADSPADataExists (LadspaPluginDescriptor* desc) {
  if (desc->plugin == NULL) {
    printf( " Need to load actual plugin data for '%s'\n",
            (desc->uri).toAscii().constData() );

    // use uri to get data
    SLV2Value uri = slv2_value_new_uri(
        m_ladspaWorld.world, (desc->uri).toAscii().constData() );

    desc->plugin = slv2_plugins_get_by_uri( m_pluginList, uri );
    slv2_value_free( uri );

    // Still empty??
    if( desc->plugin == NULL ) {
            printf( " Failed to load actual plugin data for '%s'\n", (desc->uri).toAscii().constData() );
    }
  }
}
*/


void LadspaPluginProvider::addLadspaPlugin (SLV2Plugin plugin)
{
  QString key = slv2_value_as_uri( slv2_plugin_get_uri( plugin ) );

  if (m_ladspaDescriptorMap.contains( key )) {
    return;
  }

  PluginDescriptorPtr descriptor(new LadspaPluginDescriptor(m_ladspaWorld, plugin));

  // This always seems to return 'Plugin', which isn't so useful to us
  //	SLV2PluginClass pclass = slv2_plugin_get_class( _plugin );
  //	SLV2Value label = slv2_plugin_class_get_label( pclass );
  //	printf( "Plugin Class is : '%s'\n", slv2_value_as_string( label ) );

  //printf("  Audio (input, output)=(%d,%d)\n",
  //        descriptor->audioInputCount(), descriptor->audioOutputCount());

  m_ladspaDescriptorMap.insert(key, descriptor);

  //printf("  Type=%d\n", (int)descriptor->type());
}

  } // Internal
} // Ladspa

// vim: ts=8 sw=2 sts=2 et sta noai
