/*
 * JackExtension.cpp
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

#include "JackBackend.h"
#include "JackExtension.h"

#include <extensionsystem/ExtensionManager.h>

#include <QtPlugin>
#include <QtDebug>

/*!
    \namespace Jack
    \brief Provides classes that implement and expose Jack functionality.

    Primarily provides an implementation of Unison::Backend to be used by
    Core::Engine.  Currently everything is internal, but, we may expose
    Jack-Specific functionality into the public API at some point.
*/

/*!
    \namespace Jack::Internal
    \internal
    \brief BLAH
*/

namespace Jack {
  namespace Internal {

JackExtension::JackExtension() :
  m_workerCount(0)
{
}


JackExtension::~JackExtension()
{
  qDebug() << "JACK dtor";
  // BackendProvider is auto-released
}


void JackExtension::parseArguments(const QStringList& arguments)
{
  for (int i = 0; i < arguments.size() - 1; i++) {
    if (arguments.at(i) == QLatin1String("--workers")) {
      bool ok;
      int workers = arguments.at(i + 1).toInt(&ok);

      if (ok) {
        m_workerCount = workers;
      }
      i++; // skip the value
    }
  }
}


bool JackExtension::initialize(const QStringList& arguments, QString* errorMessage)
{
  Q_UNUSED(errorMessage)

  parseArguments(arguments);
  
  // Sanity
  if (m_workerCount <= 0) {
    m_workerCount = 1;
  }

  addAutoReleasedObject(new JackBackendProvider(0, m_workerCount));
  return true;
}


void JackExtension::extensionsInitialized()
{
}


void JackExtension::remoteCommand(const QStringList& options, const QStringList& args)
{
  Q_UNUSED(options)
  Q_UNUSED(args)
}


void JackExtension::shutdown()
{
  qDebug() << "JACK shutdown";
}

EXPORT_EXTENSION(JackExtension)

  } // Internal
} // Jack

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
