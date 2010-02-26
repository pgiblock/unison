/*
 * Lv2Manager.h
 *
 * Copyright (c) 2010 Paul Giblock <pgib/at/users.sourceforge.net>
 * Copyright (c) 2009 Martin Andrews <mdda/at/users.sourceforge.net>
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


#ifndef LV2_MANAGER_H
#define LV2_MANAGER_H

#include <assert.h>
#include <stdint.h>

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <slv2/world.h>
#include <slv2/plugin.h>
#include <slv2/scalepoint.h>

#ifdef HAVE_SLV2_SCALEPOINTS_H
#include <slv2/scalepoints.h>
#endif

#include "unison/Lv2Plugin.h"

const float NOHINT = -99342.2243f;

//typedef QPair<QString, QString> lv2_key_t;
typedef QString lv2_key_t; // Just need the URI for LV2...
typedef QList<lv2_key_t> l_lv2_key_t;

class QTextStream;


/** Provides a database of LV2 plug-ins.  Upon instantiation,
 *  it loads all of the plug-ins found in the LV2_PATH environmental variable
 *  and stores their access descriptors according in a dictionary keyed on
 *  the filename the plug-in was loaded from and the label of the plug-in.
 *  Can be retrieved by using lv2_key_t (which is really just the LV2 URI) */
class Lv2Manager {
public:
	void loadFromCacheFile ();
	void saveToCacheFile ();


	/** Describes the requested plugin
	 * @param plugin  The URI of the plugin to describe
	 * @return The Plugin Descriptor */
	Lv2PluginDescriptorPtr getDescriptor (const lv2_key_t & plugin);


	/** Creates our singleton instance */
	static void initializeInstance() {
		if (m_instance == NULL) {
			m_instance = new Lv2Manager();
		}
	}


	/** @return The Lv2Manager instance */
	static Lv2Manager* instance() {
		assert(m_instance != NULL);
		return m_instance;
	}

protected:
	Lv2Manager ();
	virtual ~Lv2Manager ();

private:
	void addPlugins (SLV2Plugin _plugin);

	SLV2Plugins m_pluginList;

	typedef QMap<lv2_key_t, Lv2PluginDescriptorPtr> Lv2PluginDescriptorMap;
	Lv2PluginDescriptorMap m_lv2DescriptorMap;

	Lv2World m_lv2World;

	void loadCacheEntry (QTextStream& stream);

	void ensureLV2DataExists (Lv2PluginDescriptor *desc);

	/* TODO: Add similiar function to Lv2Plugin classes

	bool isPortClass (const lv2_key_t & _plugin,
				uint32_t _port, SLV2Value _class);

	void getRanges (const lv2_key_t & _plugin,
			uint32_t _port, float * _def, float * _min, float * _max);

	//List<
	//l_sortable_plugin_t m_sortedPlugins;
	*/

	QString m_cacheFile;

	static Lv2Manager * m_instance;
} ;

#endif // LV2MANAGER_H
