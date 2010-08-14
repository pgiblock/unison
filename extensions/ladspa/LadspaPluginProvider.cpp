/*
 * LadspaPluginProvider.cpp
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

#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QFile>
#include <QtDebug>

#include "LadspaPluginProvider.h"
#include "LadspaPlugin.h"
#include "ladspa.h"

#ifdef Q_WS_WIN
#  define PATH_SEPERATOR ';'
#else
#  define PATH_SEPERATOR ':'
#endif
  
using namespace Unison;

namespace Ladspa {
  namespace Internal {

LadspaPluginProvider::LadspaPluginProvider ()
//  m_ladspaDescriptorMap()
{
  qDebug( "Initializing LADSPA Plugin Provider" );
  discoverPlugins();
  qDebug() << "Found" << m_descriptorMap.size() << "LADSPA plugins.";
  qDebug( "Done initializing LADSPA Plugin Provider" );
}


void LadspaPluginProvider::discoverPlugins ()
{
  // Figure out the search paths
  QStringList directories;

  const char *envPath = getenv("LADSPA_PATH");
  if (envPath) {
    directories << QString(envPath).split(PATH_SEPERATOR);
  }
  else {
    // Create a sensible default if LADSPA_PATH is unset
    const char *envHome = getenv("HOME");
    if (envHome) {
      directories << QString(envHome);
    }

    directories
#ifdef Q_WS_MAC
        << "/Library/Audio/Plug-Ins/LADSPA"
#endif
        << "/usr/local/lib/ladspa"
        << "/usr/local/lib64/ladspa"
        << "/usr/lib/ladspa"
        << "/usr/lib64/ladspa";
  }

  foreach (QString path, directories) {
    discoverFromDirectory(path);
  }
}


void LadspaPluginProvider::discoverFromDirectory (const QString &path)
{
  QDir directory(path);
  QFileInfoList files = directory.entryInfoList( QDir::Files );

  foreach (QFileInfo file, files) {
    if (QLibrary::isLibrary(file.absoluteFilePath())) {
      discoverFromLibrary(file.absoluteFilePath());
    }
  }
}


int LadspaPluginProvider::discoverFromLibrary (const QString &path)
{
  QLibrary lib(path);
  if (!lib.load()) {
    qWarning() << "Could not open library"
               << lib.fileName() << "for LADSPA discovery";
    return 0;
  }

  LADSPA_Descriptor_Function descriptorFunction = 
      (LADSPA_Descriptor_Function) lib.resolve("ladspa_descriptor");
    
  if (descriptorFunction == NULL) {
    // Couldn't find function, no worries, maybe the SO isn't a LADSPA
    lib.unload();
    return 0;
  }

  // Start loading plugins from the library
  int i;
  for (i=0; ; ++i) {
    const LADSPA_Descriptor *descriptor = descriptorFunction(i);
    if (descriptor == NULL) {
      break; // Nothing left
    }

    PluginDescriptorPtr desc(new LadspaPluginDescriptor(path, descriptor));

    m_descriptorMap.insert(desc->uniqueId(), desc);
  }
  return i;
}



LadspaPluginProvider::~LadspaPluginProvider ()
{}


PluginDescriptorPtr LadspaPluginProvider::descriptor (const QString uniqueId)
{
  // returns null on fail
  return m_descriptorMap.value( uniqueId );
}


  } // Internal
} // Ladspa

// vim: ts=8 sw=2 sts=2 et sta noai
