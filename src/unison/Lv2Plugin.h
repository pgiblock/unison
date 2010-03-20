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

#ifndef LV2_PLUGIN_H
#define LV2_PLUGIN_H

#include <slv2/slv2.h>

#include "unison/Plugin.h"
#include "unison/types.h"

namespace Unison {


/** The SLV2World, and various cached (as symbols, fast) URIs.
 *  This object represents everything Unison 'knows' about LV2
 *  (ie understood extensions/features/etc) */
struct Lv2World {
	Lv2World ();
	~Lv2World ();

	SLV2World world;         ///< The SLV2World itself

	SLV2Value inputClass;    ///< Input port
	SLV2Value outputClass;   ///< Output port

	SLV2Value audioClass;    ///< Audio port
	SLV2Value controlClass;  ///< Control port
	SLV2Value eventClass;    ///< Event port
	SLV2Value midiClass;     ///< MIDI event

	SLV2Value inPlaceBroken; ///< Plugin requires seperate buffers
	SLV2Value integer;       ///< Integer restrictions for control ports
	SLV2Value toggled;       ///< Boolean restriction for control ports
	SLV2Value sampleRate;    ///< Port values are multiplied by sampling rate
	SLV2Value gtkGui;        ///< GTK-based gui is available
};



/** Plugin implementation for an Lv2Plugin.  Most values are queried directly
 *  from slv2 on demand.  It will probably be wise to cache some values when
 *  it is safe to do so (like num-ports, port-descriptors, etc..) */
class Lv2Plugin : public Plugin {
public:
	Lv2Plugin (Lv2World&, SLV2Plugin plugin, nframe_t sampleRate);
	Lv2Plugin (const Lv2Plugin &);

	~Lv2Plugin ();

	QString name () const {
		return QString::fromAscii( slv2_value_as_string( m_name ) );
	}

	QString uniqueId () const {
		return QString::fromAscii(
			slv2_value_as_uri( slv2_plugin_get_uri( m_plugin ) ) );
	}

	// TODO: PluginType type();

	uint32_t audioInputCount () const {
		return slv2_plugin_get_num_ports_of_class(
			m_plugin, m_world.inputClass, m_world.audioClass, NULL );
	}

	uint32_t audioOutputCount () const {
		return slv2_plugin_get_num_ports_of_class(
			m_plugin, m_world.outputClass, m_world.audioClass, NULL );
	}

	QString authorName () const {
		return QString::fromAscii( m_authorName ?
			slv2_value_as_string( m_authorName ) :
			"Unknown" );
	}

	QString authorEmail () const {
		return QString::fromAscii( m_authorEmail ?
			slv2_value_as_string( m_authorEmail ) :
			NULL );
	}

	QString authorHomepage () const {
		return QString::fromAscii( m_authorHomepage ?
			slv2_value_as_string( m_authorHomepage ) :
			NULL );
	}

	QString copyright () const {
		return QString::fromAscii( m_copyright ?
			slv2_value_as_string( m_copyright ) :
			NULL );
	}

	uint32_t portCount () const {
		return slv2_plugin_get_num_ports(m_plugin);
	}

	Port* port (uint32_t idx) const;

	/** @returns The underlying SLV2Plugin */
	SLV2Plugin slv2Plugin() const {
		return m_plugin;
	}

	void activate ();
	void deactivate ();

	void process(const ProcessingContext & context);

	// TODO: loadState and saveState

private:
	Lv2World&      m_world;
	SLV2Plugin     m_plugin;
	nframe_t       m_sampleRate;

	SLV2Instance   m_instance;
	SLV2Value      m_name;
	SLV2Value      m_authorName;
	SLV2Value      m_authorEmail;
	SLV2Value      m_authorHomepage;
	SLV2Value      m_copyright;

	bool           m_activated;

	void init ();
};



/** A description of a LV2 plugin.  This descriptor allows us to query LV2
 *  plugins without actually instantiating them.  This can be abstracted
 *  into a PluginDescriptor if othe plugin types are ever needed. */
class Lv2PluginDescriptor : public PluginDescriptor {
public:
	Lv2PluginDescriptor (Lv2World& world, SLV2Plugin plugin);
	Lv2PluginDescriptor (const Lv2PluginDescriptor& descriptor);

	PluginPtr createPlugin (nframe_t sampleRate) const;

	// TODO: Lv2World and SLV2Plugin Ptr accessors?

private:
	Lv2World& m_world;
	SLV2Plugin m_plugin;
};



/** A Port on a plugin.  I wonder if we should be calling slv2 functions, or
	maybe we should just copy all the data into the class? */
class Lv2Port : public Port {
public:
	Lv2Port (const Lv2World & m_world, const Lv2Plugin * plugin, uint32_t index);

	~Lv2Port ();

	/* Port Interface */
	QString name (size_t maxLength) const {
		return QString::fromAscii( slv2_value_as_string(
			slv2_port_get_name( m_plugin->slv2Plugin(), m_port ) ) );
	}

	float value () const {
		return m_value;
	}

	void setValue (float value) {
		m_value = value;
	}

	float defaultValue () const {
		return m_defaultValue;
	}

	bool isBounded () const {
		return true;
	}

	float minimum () const {
		return m_min;
	}

	float maximum () const {
		return m_max;
	}

	bool isToggled () const {
		return slv2_port_has_property( m_plugin->slv2Plugin(), m_port,
									   m_world.toggled );
	}

	bool isInput () const {
		return slv2_port_is_a( m_plugin->slv2Plugin(), m_port,
							   m_world.inputClass );
	}

	bool isOutput () const {
		return slv2_port_is_a( m_plugin->slv2Plugin(), m_port,
							   m_world.outputClass );
	}

	const Node* node () const {
		return m_plugin;
	}

private:
	float m_value;
	float m_defaultValue;
	float m_min;
	float m_max;

	// Don't point to Lv2Plugin, a two-way rel is probably unwanted
	const Lv2World & m_world;
	const Lv2Plugin * m_plugin;
	SLV2Port m_port;
};


} // Unison


#endif // LV2_PLUGIN_H
