/*
 * PluginManager.h
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


#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <assert.h>
#include <stdint.h>

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <slv2/world.h>
#include <slv2/plugin.h>
#include <slv2/scalepoint.h>

#include "unison/Lv2Plugin.h"

#ifdef HAVE_SLV2_SCALEPOINTS_H
#include <slv2/scalepoints.h>
#endif

namespace Unison {


const float NOHINT = -99342.2243f;

class QTextStream;


/** Provides a database of LV2 plug-ins.  Upon instantiation,
 *  it loads all of the plug-ins found in the LV2_PATH environmental variable
 *  and stores their access descriptors according in a dictionary keyed on
 *  the filename the plug-in was loaded from and the label of the plug-in.
 *  Can be retrieved by using lv2_key_t (which is really just the LV2 URI) */
class PluginManager {
public:

	/** Describes the requested plugin.  Right now, only LV2 plugins are
	 *  supported. This could take a param, or split into multiple functions
	 *  for distinguishing between LADSPA, VST, etc..
	 *  @param plugin  The URI of the plugin to describe
	 *  @return The Plugin Descriptor */
	PluginDescriptorPtr descriptor (const QString plugin);


	/** Creates our singleton instance.  Must be called during application
		boot. */
	static void initializeInstance() {
		if (m_instance == NULL) {
			m_instance = new PluginManager();
		}
	}


	/** @return The Lv2Manager instance */
	static PluginManager* instance() {
		assert(m_instance != NULL);
		return m_instance;
	}

	/** TODO: Something better */
	static void cleanupHack() {
		delete m_instance;
		m_instance = NULL;
	}

protected:
	PluginManager ();
	virtual ~PluginManager ();

private:
	void addLv2Plugin (SLV2Plugin _plugin);

	Lv2World m_lv2World;

	typedef QMap<QString, PluginDescriptorPtr> Lv2PluginDescriptorMap;
	Lv2PluginDescriptorMap m_lv2DescriptorMap;

	// TODO: Bring back
	// void ensureLV2DataExists (PluginDescriptor *desc);
	// l_sortable_plugin_t m_sortedPlugins;

	static PluginManager * m_instance;
} ;


} // Unison

#endif // PLUGIN_MANAGER_H
