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

#ifndef LV2PLUGIN_H
#define LV2PLUGIN_H

#include <QtCore/QSharedPointer>
#include <QtXml/QDomNode>
#include <slv2/slv2.h>

#include "unison/types.h"


/** The SLV2World, and various cached (as symbols, fast) URIs.
 *  This object represents everything Unison 'knows' about LV2
 *  (ie understood extensions/features/etc) */
struct Lv2World {
	Lv2World();
	~Lv2World();

	SLV2World world;
	SLV2Value inputClass;    ///< Input port
	SLV2Value outputClass;   ///< Output port
	SLV2Value audioClass;    ///< Audio port
	SLV2Value controlClass;  ///< Control port
	SLV2Value eventClass;    ///< Event port
	SLV2Value midiClass;     ///< MIDI event
	SLV2Value inPlaceBroken; ///< If the plugin requires seperate buffers
	SLV2Value integer;
	SLV2Value toggled;
	SLV2Value sampleRate;
	SLV2Value gtkGui;
};



/** The type of plugin, regarding I/O. */
enum Lv2PluginType {
	SOURCE,
	TRANSFER,
	VALID,
	INVALID,
	SINK,
	OTHER
};



class Lv2Plugin {
public:
	Lv2Plugin (Lv2World&, SLV2Plugin plugin, nframe_t sampleRate);
	Lv2Plugin (const Lv2Plugin &);

	~Lv2Plugin ();

	/* Plugin interface */
	QString uri() const {
		return QString::fromAscii(
			slv2_value_as_uri(slv2_plugin_get_uri(m_plugin)));
	}

	QString label() const {
		return QString::fromAscii(slv2_value_as_string(m_name));
	}

	QString name() const {
		return QString::fromAscii(slv2_value_as_string(m_name));
	}

	QString creator() const {
		return QString::fromAscii(m_author ?
			slv2_value_as_string(m_author) :
			"Unknown");
	}

	void activate ();
	void deactivate ();

	QDomNode& loadState();
	int      saveState (const QDomNode& node);

private:
	Lv2World&      m_world;
	SLV2Plugin     m_plugin;
	nframe_t      m_sampleRate;

	SLV2Instance   m_instance;
	SLV2Value      m_name;
	SLV2Value      m_author;

	bool           m_activated;


	void init ();
};

/** A Safe pointer to a plugin. */
typedef QSharedPointer<Lv2Plugin> Lv2PluginPtr;



/** A description of a LV2 plugin.  This descriptor allows us to query LV2
 *  plugins without actually instantiating them.  This can be abstracted
 *  into a PluginDescriptor if othe plugin types are ever needed. */
class Lv2PluginDescriptor {
public:
	Lv2PluginDescriptor(Lv2World& world, SLV2Plugin plugin);
	Lv2PluginDescriptor(const Lv2PluginDescriptor& descriptor);

	Lv2World& world;
	SLV2Plugin plugin;
	QString uri;

	// TODO: Not LV2-Specific add uniqueId
	QString creator;
	QString name;
	Lv2PluginType type;
	uint16_t inputChannels;
	uint16_t outputChannels;

	Lv2PluginPtr createPlugin(nframe_t sampleRate);
};

/** A Safe pointer to a plugin descriptor. */
typedef QSharedPointer<Lv2PluginDescriptor> Lv2PluginDescriptorPtr;


#endif // LV2PLUGIN_H
