/*
 * Lv2Manager.h - declaration of class lv2Manager a class to manage lv2 plugins
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


#ifndef LV2_MANAGER_H_
#define LV2_MANAGER_H_

#include <stdint.h>

#include <slv2/world.h>
#include <slv2/plugin.h>
#include <slv2/scalepoint.h>

#ifdef HAVE_SLV2_SCALEPOINTS_H
#include <slv2/scalepoints.h>
#endif

#include <QtCore/QPointer>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>



const float NOHINT = -99342.2243f;

//typedef QPair<QString, QString> lv2_key_t;
typedef QString lv2_key_t; // Just need the URI for LV2...
typedef QList<lv2_key_t> l_lv2_key_t;


/**
 * The SLV2World, and various cached (as symbols, fast) URIs.
 *
 * This object represents everything Unison 'knows' about LV2
 * (ie understood extensions/features/etc)
 */
struct Lv2World {
	Lv2World();
	~Lv2World();

	SLV2World world;
	SLV2Value inputClass; ///< Input port
	SLV2Value outputClass; ///< Output port
	SLV2Value audioClass; ///< Audio port
	SLV2Value controlClass; ///< Control port
	SLV2Value eventClass; ///< Event port
	SLV2Value midiClass; ///< MIDI event
	SLV2Value inPlaceBroken;
	SLV2Value integer;
	SLV2Value toggled;
	SLV2Value srate;
	SLV2Value gtkGui;
};


enum Lv2PluginType {
	SOURCE,
	TRANSFER,
	VALID,
	INVALID,
	SINK,
	OTHER
};

typedef struct Lv2PluginDescriptor {
	Lv2World* world;
	SLV2Plugin* plugin;

	QString uri;
	QString name;
	QString creator;
	Lv2PluginType type;
	uint16_t inputChannels;
	uint16_t outputChannels;
};

typedef QSharedPointer<Lv2PluginDescriptor> Lv2PluginDescriptorPtr;



/* lv2Manager provides a database of LV2 plug-ins.  Upon instantiation,
it loads all of the plug-ins found in the LV2_PATH environmental variable
and stores their access descriptors according in a dictionary keyed on
the filename the plug-in was loaded from and the label of the plug-in.

The can be retrieved by using lv2_key_t (which is really just the LV2 URI) :

// lv2_key_t key( "lalala" )

as the plug-in key. */
class Lv2Manager {
public:
	Lv2Manager();
	virtual ~Lv2Manager();

	void loadFromCacheFile();
	void saveToCacheFile();

	Lv2PluginDescriptor * getDescriptor( const lv2_key_t & _plugin );


private:
	void addPlugins( SLV2Plugin _plugin );
	void ensureLV2DataExists( Lv2PluginDescriptor *desc );

	bool isPortClass( const lv2_key_t & _plugin,
				uint32_t _port, SLV2Value _class );

	void getRanges( const lv2_key_t & _plugin,
			uint32_t _port, float * _def, float * _min, float * _max );

	SLV2Plugins m_pluginList;

	typedef QMap<lv2_key_t, Lv2PluginDescriptorPtr> Lv2PluginDescriptorMap;
	Lv2PluginDescriptorMap m_lv2DescriptorMap;

	//List<
	//l_sortable_plugin_t m_sortedPlugins;

	Lv2World m_lv2Bundle;

	QString m_cacheFile;

	static Lv2Manager * m_instance;
} ;

#endif
