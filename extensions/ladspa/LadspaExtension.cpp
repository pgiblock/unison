/*
 * LadspaExtension.cpp
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

#include "LadspaExtension.hpp"
#include "LadspaPluginProvider.hpp"

#include <extensionsystem/ExtensionManager.hpp>

#include <QDebug>

namespace Ladspa {
  namespace Internal {

LadspaExtension::LadspaExtension() :
  m_pluginProvider(NULL)
{}


LadspaExtension::~LadspaExtension()
{
  qDebug() << "LADSPA dtor";
  if (m_pluginProvider) {
    removeObject(m_pluginProvider);
    delete m_pluginProvider;
  }
}


void LadspaExtension::parseArguments(const QStringList &arguments)
{
  Q_UNUSED(arguments)
}


bool LadspaExtension::initialize(const QStringList &arguments, QString *errorMessage)
{
  Q_UNUSED(errorMessage)
  parseArguments(arguments);

  m_pluginProvider = new LadspaPluginProvider();
  addObject(m_pluginProvider);
  
  return true;
}


void LadspaExtension::extensionsInitialized()
{
}


void LadspaExtension::remoteCommand(const QStringList &options, const QStringList &args)
{
  Q_UNUSED(options)
  Q_UNUSED(args)
}

void LadspaExtension::shutdown()
{
  qDebug() << "LADSPA shutdown";
}

EXPORT_EXTENSION(LadspaExtension)

  } // Internal
} // Ladspa

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
