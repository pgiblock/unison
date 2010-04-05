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

#include <iostream> // for cout
#include <QSet>
#include "unison/Lv2Plugin.h"
#include "unison/ProcessingContext.h"
#include "unison/BufferProvider.h"

using namespace Unison;


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
	std::cout << "REMOVE ME:  Detroying Lv2World!\n";
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


Lv2Port::Lv2Port (const Lv2World & world, Lv2Plugin * plugin, uint32_t index) :
	Port(),
	m_world(world),
	m_plugin(plugin),
	m_index(index),
	m_value(0),
	m_defaultValue(0),
	m_min(0),
	m_max(0) {

	// TODO: Error handling
	m_port = slv2_plugin_get_port_by_index( plugin->slv2Plugin(), index );

	// set range and default
	SLV2Value def, min, max;
	slv2_port_get_range( plugin->slv2Plugin(), m_port, &def , &min, &max );
	if (def) {
		m_defaultValue = slv2_value_as_float( def );
		slv2_value_free( def );
	}
	if (min) {
		m_min   = slv2_value_as_float( min );
		slv2_value_free( min );
	}
	if (max) {
		m_max   = slv2_value_as_float( max );
		slv2_value_free( max );
	}
}


Lv2Port::~Lv2Port () {
	// Is the LVPort just a handle to the Plugin?
}


const QSet<Node* const> Lv2Port::interfacedNodes () const {
	QSet<Node* const> p;
	p.insert( m_plugin );
	return p;
}


void Lv2Port::connectToBuffer() {
	slv2_instance_connect_port (m_plugin->slv2Instance(), m_index, buffer()->data());
}


void Lv2Port::aquireBuffer (BufferProvider & provider)
{
	int numConnections = dependencies().count();
	//std::cout << qPrintable(m_plugin->name()) << ": " << qPrintable(name()) << ": ";
	switch (direction()) {
	case INPUT:
		if (type() == AUDIO_PORT && numConnections == 0) {
			// Use silence
			//std::cout << " need silence!" << std::endl;
			m_buffer = provider.zeroAudioBuffer();
		}
		else if (numConnections == 1) {
			// Use the other port's buffer
			//std::cout << " need to share!" << std::endl;
			// TODO: ensure type is the same? or at least make sure on connect
			Port* other = (Port*) *(dependencies().begin());
			m_buffer = other->buffer();
		}

		if (!m_buffer) {
			// Return internal port
			//std::cout << " need to create (I)!" << std::endl;
			m_buffer = provider.aquire(type(), 1024);
		}

		// TODO: Remove this hack
		if (type() == CONTROL_PORT) {
			float * data = (float*)m_buffer->data();
			data[0] = maximum();
		}

		break;
	case OUTPUT:
		// Return internal port
		//std::cout << " need to create (O)!" << std::endl;
		if (numConnections == 1) {
			// Use the other port's buffer
			//std::cout << " need to share!" << std::endl;
			// TODO: ensure type is the same? or at least make sure on connect
			Port* other = (Port*) *(dependents().begin());
			m_buffer = other->buffer();
		}

		if (!m_buffer) {
			m_buffer = provider.aquire(type(), 1024);
		}
		break;
	}
}

Lv2Plugin::Lv2Plugin (Lv2World& world, SLV2Plugin plugin, nframes_t sampleRate) :
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


void Lv2Plugin::init () {
	m_activated = false;
	// TODO: Pass in features
	m_instance = slv2_plugin_instantiate( m_plugin, m_sampleRate, NULL );
	assert(m_instance);

	// TODO: this is ugly. write copy ctors and assignment operators and cp by val.
	int count = portCount();
	m_ports.resize( count );
	for (int i = 0; i < count; ++i) {
		m_ports[i] = new Lv2Port( m_world, this, i );
	}

	m_name = slv2_plugin_get_name( m_plugin );
	assert(m_name);

	m_authorName = slv2_plugin_get_author_name( m_plugin );
	m_authorEmail = slv2_plugin_get_author_email( m_plugin );
	m_authorHomepage = slv2_plugin_get_author_homepage( m_plugin );
}


Lv2Plugin::~Lv2Plugin () {
	deactivate();
	slv2_instance_free( m_instance );
	slv2_value_free( m_name );
	slv2_value_free( m_authorName );
	slv2_value_free( m_authorEmail );
	slv2_value_free( m_authorHomepage );
	//slv2_value_free( m_copyright );
	for (int i=0; i<m_ports.count(); ++i) {
		delete m_ports[i];
	}

	m_instance = NULL;
}


Port* Lv2Plugin::port (int idx) const {
	return m_ports[idx];
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


void Lv2Plugin::process (const ProcessingContext & context) {
	slv2_instance_run(m_instance, context.bufferSize());
}


const QSet<Node* const> Lv2Plugin::dependencies () const {
	QSet<Node* const> n;
	int count = portCount();
	for (int i=0; i<count; ++i) {
		Port * p  = port(i);
		if (p->direction() == INPUT) { n += p; }
	}
	return n;
}


const QSet<Node* const> Lv2Plugin::dependents () const {
	QSet<Node* const> n;
	int count = portCount();
	for (int i=0; i<count; ++i) {
		Port * p  = port(i);
		if (p->direction() == OUTPUT) { n += p; }
	}
	return n;
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


PluginPtr Lv2PluginDescriptor::createPlugin (nframes_t sampleRate) const {
	return PluginPtr( new Lv2Plugin( m_world, m_plugin, sampleRate ) );
}



