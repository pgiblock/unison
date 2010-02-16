/*
 * Lv2Manager.cpp - a class to manage loading and instantiation of lv2 plugins
 *
 * Copyright (c) 2009 Martin Andrews <mdda/at/users.sourceforge.net>
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


#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QLibrary>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <math.h>

#include "unison/Lv2Manager.h"

// There is only one of these...
Lv2Manager * Lv2Manager::m_instance = (Lv2Manager *)NULL;



Lv2World::Lv2World () {
	world=slv2_world_new();
	slv2_world_load_all( world );

	inputClass = slv2_value_new_uri( world, SLV2_PORT_CLASS_INPUT );
	outputClass = slv2_value_new_uri(world, SLV2_PORT_CLASS_OUTPUT );
	controlClass = slv2_value_new_uri( world, SLV2_PORT_CLASS_CONTROL );
	audioClass = slv2_value_new_uri( world, SLV2_PORT_CLASS_AUDIO );
	eventClass = slv2_value_new_uri( world, SLV2_PORT_CLASS_EVENT );
	midiClass = slv2_value_new_uri( world, SLV2_EVENT_CLASS_MIDI );
	inPlaceBroken = slv2_value_new_uri( world, SLV2_NAMESPACE_LV2 "inPlaceBroken" );
	integer = slv2_value_new_uri( world, SLV2_NAMESPACE_LV2 "integer" );
	toggled = slv2_value_new_uri( world, SLV2_NAMESPACE_LV2 "toggled" );
	sampleRate = slv2_value_new_uri( world, SLV2_NAMESPACE_LV2 "sampleRate" );
	gtkGui = slv2_value_new_uri( world, "http://lv2plug.in/ns/extensions/ui#GtkUI" );
}



Lv2World::~Lv2World () {
	slv2_value_free( inputClass );
	slv2_value_free( outputClass );
	slv2_value_free( controlClass );
	slv2_value_free( audioClass );
	slv2_value_free( eventClass );
	slv2_value_free( midiClass );
	slv2_value_free( inPlaceBroken );
	slv2_value_free( integer );
	slv2_value_free( toggled );
	slv2_value_free( sampleRate );
	slv2_value_free( gtkGui );

	slv2_world_free( world );
}



Lv2Manager::Lv2Manager()
{
	if (m_lv2Bundle.world == NULL) {
		printf( "Failed to Initialize slv2_world\n" );
		return;
	}
	printf( "Initialized slv2_world\n" );

	// TODO: Replace with SQL?
	loadFromCacheFile();

	// slv2_world_load_all( m_world ); // No special path apart from LV2_PATH
	m_pluginList = slv2_world_get_all_plugins( m_lv2Bundle.world );

	// TODO: Allow user to choose multiple paths
	for (unsigned i=0; i < slv2_plugins_size( m_pluginList ); ++i) {
		SLV2Plugin p = slv2_plugins_get_at( m_pluginList, i );
		addPlugins( p );  // This will just return if the plugin information is cached (after being loaded)
	}

	saveToCacheFile();
}



Lv2Manager::~Lv2Manager () {
	/*
	for( Lv2ManagerMap::iterator it = m_lv2ManagerMap.begin();
					it != m_lv2ManagerMap.end(); ++it )
	{
		delete it.value();
	}
	*/

	slv2_plugins_free( m_lv2Bundle.world, m_pluginList );
}



Lv2PluginDescriptor Lv2Manager::getDescriptor (const lv2_key_t & _plugin) {
	return m_lv2DescriptorMap.value( _plugin, NULL );
}



void Lv2Manager::ensureLV2DataExists (Lv2PluginDescriptor *desc) {
	if (desc->plugin == NULL) {
		printf( " Need to load actual plugin data for '%s'\n", (desc->uri).toAscii().constData() );

		// use uri to get data
		SLV2Value uri = slv2_value_new_uri(
				m_lv2_bundle.world, (desc->uri).toAscii().constData() );

		desc->plugin = slv2_plugins_get_by_uri( m_pluginList, uri );
		slv2_value_free( uri );

		// Still empty??
		if( desc->plugin == NULL ) {
			printf( " Failed to load actual plugin data for '%s'\n", (desc->uri).toAscii().constData() );
		}
	}
}




void Lv2Manager::addPlugins (SLV2Plugin _plugin) {
	QString uri = QString( slv2_value_as_uri( slv2_plugin_get_uri( _plugin ) ) );
	lv2_key_t key = lv2_key_t( uri );

	if (m_lv2DescriptorMap.contains( key )) {
		printf( "Already in Cache LV2 plugin URI : '%s'\n", uri.toAscii().constData() );
		return;
	}

	printf( "Examining LV2 plugin URI : '%s'\n", uri.toAscii().constData() );

	Lv2PluginDescriptorPtr descriptor( new Lv2PluginDescriptor );
	descriptor->plugin = _plugin;

	// Investigate this plugin
	descriptor->uri = uri;

	// Any data accessed within the plugin itself causes the whole thing to be 'unwrapped'
	SLV2Value data = slv2_plugin_get_name( _plugin );
	descriptor->name = QString( slv2_value_as_string( data ) );
	// printf( " LV2 plugin Name : '%s'\n", slv2_value_as_string( data ) );
	slv2_value_free( data );

	descriptor->inputChannels = slv2_plugin_get_num_ports_of_class( _plugin,
	m_lv2Bundle.inputClass, m_lv2Bundle.audioClass, NULL);

	descriptor->outputChannels = slv2_plugin_get_num_ports_of_class( _plugin,
	m_lv2Bundle.outputClass, m_lv2Bundle.audioClass, NULL);

	// This always seems to return 'Plugin', which isn't so useful to us
	//	SLV2PluginClass pclass = slv2_plugin_get_class( _plugin );
	//	SLV2Value label = slv2_plugin_class_get_label( pclass );
	//	printf( "Plugin Class is : '%s'\n", slv2_value_as_string( label ) );

	printf( "  Audio (input, output)=(%d,%d)\n", descriptor->inputChannels, descriptor->outputChannels );

	if (descriptor->outputChannels > 0) {
		if (descriptor->inputChannels > 0) {
			descriptor->type = TRANSFER;
		}
		else {
			descriptor->type = SOURCE;
		}
	}
	else if (descriptor->inputChannels > 0) {
		descriptor->type = SINK;
	}
	else {
		descriptor->type = OTHER;
	}

	m_lv2DescriptorMap.insert( key, descriptor );

	printf( "  Finished that plugin : type=%d\n", (int)description->type );

	/*
	const LADSPA_Descriptor * descriptor;

	for( long pluginIndex = 0;
		( descriptor = _descriptor_func( pluginIndex ) ) != NULL;
								++pluginIndex )
	{
		ladspa_key_t key( _file, QString( descriptor->Label ) );
		if( m_ladspaManagerMap.contains( key ) )
		{
			continue;
		}

		plugIn->index = pluginIndex;

		m_ladspaManagerMap[key] = plugIn;
	}
	*/
}



void Lv2Manager::saveToCacheFile () {
	QFile file( m_cacheFile );
	QString line;

	if (!file.open( QIODevice::WriteOnly )) {
		return;
	}

	QTextStream stream( &file );
	stream << "; This file is auto-regenerated by Unison\n\n";

	l_lv2_key_t keys = m_lv2DescriptorMap.keys();
	for (Lv2PluginDescriptorMap::iterator i = m_lv2DescriptorMap.begin();
			i != m_lv2DescriptorMap.end(); ++i) {

		stream << "[" << i.key() << "]\n"; // Plugin URI

		Lv2PluginDescriptorPtr description( *i );
		stream << "name=" << description->name << "\n"; // Plugin name
		stream << "type=" << (int)description->type << "\n";
		stream << "channels.audio.input=" << description->inputChannels << "\n";
		stream << "channels.audio.output=" << description->outputChannels << "\n";

		stream << "\n";
	}

	file.close();
}



void Lv2Manager::loadFromCacheFile () {
	QFile file( m_cacheFile );
	QString line;
	if ( file.open( QIODevice::ReadOnly ) ) { // file opened successfully
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {    // until end of file...
			line = stream.readLine().trimmed();

			// Skip lines that don't match ^[.*]$
			if( line.startsWith( "[" ) && line.endsWith( "]" ) ) {
				loadCacheEntry(stream);
			}
		}
		// Close the file
		file.close();
	}
	return;
}



void Lv2Manager::loadCacheEntry (const QTextStream& stream) {
	Lv2PluginDescriptorPtr desc =
			Lv2PluginDescriptorPtr( new Lv2PluginDescriptor );

	desc->plugin = NULL; // To ensure that we don't attempt to use it
	desc->uri = line.mid( 1, line.length()-2 ); // Extract the URI
	printf( "Reading Data for '%s' from the Cache\n", desc->uri.toAscii().data() );

	// Read in data for this uri - until we get to a blank line
	bool finished = false;
	while (!stream.atEnd() && !finished) {
		line = stream.readLine().trimmed();
		if (line.length() == 0) {
			finished = true;
			continue;
		}

		int equals = line.indexOf( "=" );
		if (equals > 0) {
			QString name = line.left(equals);
			QString value = line.mid(equals+1);
			printf( " Found '%s' = '%s'\n", name.toAscii().data(), value.toAscii().data() );

			if (name == "name") {
				desc->name = value;
			}
			else if (name == "type") {
				desc->type = (Lv2PluginType)value.toInt();
			}
			else if (name == "channels.audio.input") {
				desc->inputChannels = value.toInt();
			}
			else if (name == "channels.audio.output") {
				desc->outputChannels = value.toInt();
			}
			else {
				printf( "Could not parse line : '%s' in lv2.cache\n", line.toAscii().data() );
			}
		}
	}
	// Dump the discovered data into the manager
	lv2_key_t key = lv2_key_t( desc->uri );
	m_lv2DescriptorMap.insert(key, desc);
}


#endif
