/*
 * PluginManager.h
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

#ifndef UNISON_PLUGIN_MANAGER_H
#define UNISON_PLUGIN_MANAGER_H

#include <unison/PluginInfo.h>

#include <QString>

namespace Core {

class PluginManager
{
  public:

    /** Describes the requested plugin.  
     *  @param plugin  The URI of the plugin to describe
     *  @return The PluginInfo */
    Unison::PluginInfoPtr info (const QString& plugin);

    /** Creates our singleton instance.  Must be called during application
     *  boot. */
    static void initializeInstance ()
    {
      if (m_instance == NULL) {
        m_instance = new PluginManager();
      }
    }

    /** @return The Lv2Manager instance */
    static PluginManager* instance () {
      Q_ASSERT(m_instance != NULL);
      return m_instance;
    }

    /** TODO-NOW: Something better */
    static void cleanupHack () {
      delete m_instance;
      m_instance = NULL;
    }

    /* TODO: some means of caching plugin details to a datastore QtSQL */

  protected:
    PluginManager ();
    virtual ~PluginManager ();

  private:

    static PluginManager* m_instance;
};

} // Core

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
