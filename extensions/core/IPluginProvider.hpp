/*
 * IPluginProvider.hpp
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

#ifndef UNISON_IPLUGIN_PROVIDER_H
#define UNISON_IPLUGIN_PROVIDER_H

#include "Core_global.hpp"

#include <unison/PluginInfo.hpp>

#include <QObject>

namespace Core {

/**
 * Provides a database of Plugins.  The PluginProvider interface abstracts the
 * features needed for listing and querying plugins for a specific plugin
 * technology.  Any extensions wishing to implement this functionality must add
 * their implemenation to ExtensionManager with addObject(), the implementation
 * will then be used by PluginManager.
 */
class CORE_EXPORT IPluginProvider : public QObject
{
  Q_OBJECT
  public:
    IPluginProvider (QObject* parent = 0) : QObject(parent) {};
    virtual ~IPluginProvider () {};

    virtual QString displayName () = 0;

    /** Describes the requested plugin.  
     *  @param plugin  The Name of the plugin to describe
     *  @return The Plugin info
     */
    virtual Unison::PluginInfoPtr info (const QString& uniqueId) = 0;
};

} // Core


#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
