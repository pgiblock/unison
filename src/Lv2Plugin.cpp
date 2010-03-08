/*
 * Lv2Plugin.h
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

#include "unison/Lv2Plugin.h"


Lv2World::Lv2World () {
	world=slv2_world_new();
	slv2_world_load_all( world );

	// Hold on to these classes for performance
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



Lv2Port::Lv2Port (const Lv2World & world, SLV2Plugin plugin, uint32_t index) :
	Port(),
	m_world(world),
	m_plugin(plugin) {

	// TODO: Error handling
	m_port = slv2_plugin_get_port_by_index( plugin, index );

	// set range and default
	SLV2Value def, min, max;
	slv2_port_get_range( m_plugin, m_port, &def , &min, &max );
	m_defaultValue = slv2_value_as_float( def );
	m_min   = slv2_value_as_float( min );
	m_max   = slv2_value_as_float( max );
	slv2_value_free( def );
	slv2_value_free( min );
	slv2_value_free( max );
}


Lv2Port::~Lv2Port () {
}



Lv2Plugin::Lv2Plugin (Lv2World& world, SLV2Plugin plugin, nframe_t sampleRate) :
	Plugin(),
	m_world(world),
	m_plugin(plugin),
	m_sampleRate(sampleRate) {

	init();
}


Lv2Plugin::Lv2Plugin (const Lv2Plugin& other) :
	Plugin(other),
	m_world(other.m_world),
	m_plugin(other.m_plugin),
	m_sampleRate(other.m_sampleRate) {

	init();
}


Lv2Plugin::~Lv2Plugin () {
	deactivate();
	slv2_instance_free( m_instance );
	slv2_value_free( m_name );
	slv2_value_free( m_authorName );
	slv2_value_free( m_authorEmail );
	slv2_value_free( m_authorHomepage );
	m_instance = NULL;
}


void Lv2Plugin::init () {
	m_activated = false;
	m_instance = slv2_plugin_instantiate( m_plugin, m_sampleRate, NULL );
	assert(m_instance);

	m_name = slv2_plugin_get_name( m_plugin );
	assert(m_name);

	m_authorName = slv2_plugin_get_author_name( m_plugin );
	m_authorEmail = slv2_plugin_get_author_email( m_plugin );
	m_authorHomepage = slv2_plugin_get_author_homepage( m_plugin );
}


void Lv2Plugin::activate () {
	if (!m_activated) {
		slv2_instance_activate( m_instance );
		m_activated = true;
	}
}


void Lv2Plugin::deactivate () {
	if (m_activated) {
		slv2_instance_deactivate( m_instance );
		m_activated = false;
	}
}


Lv2PluginDescriptor::Lv2PluginDescriptor (Lv2World& world, SLV2Plugin plugin) :
	m_world(world),
	m_plugin(plugin) {

	SLV2Value data;

	m_uniqueId = QString( slv2_value_as_uri( slv2_plugin_get_uri( plugin ) ) );

	data = slv2_plugin_get_name( plugin );
	m_name = QString( slv2_value_as_string( data ) );
	slv2_value_free( data );

	data = slv2_plugin_get_author_name( plugin );
    m_author = QString( slv2_value_as_string( data ) );
    slv2_value_free( data );

	m_audioInputs = slv2_plugin_get_num_ports_of_class( plugin,
		world.inputClass, world.audioClass, NULL );

	m_audioOutputs = slv2_plugin_get_num_ports_of_class( plugin,
		world.outputClass, world.audioClass, NULL );

	// TODO: break type-calculation into private Plugin helper fn
	if (m_audioInputs > 0) {
		if (m_audioOutputs > 0) {
			m_type = TRANSFER;
		}
		else {
			m_type = SINK;
		}
	}
	else if (m_audioOutputs > 0) {
		m_type = SOURCE;
	}
	else {
		m_type = OTHER;
	}

}


Lv2PluginDescriptor::Lv2PluginDescriptor (const Lv2PluginDescriptor& descriptor) :
	PluginDescriptor(descriptor),
	m_world(descriptor.m_world),
	m_plugin(descriptor.m_plugin) {
}


PluginPtr Lv2PluginDescriptor::createPlugin (nframe_t sampleRate) const {
	return PluginPtr( new Lv2Plugin( m_world, m_plugin, sampleRate ) );
}



