/*
 * AlsaExtension.cpp
 *
 * Copyright (c) 2010 Andrew Kelley <superjoe30+unison@gmail.com>
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

#include "AlsaBackend.hpp"
#include "AlsaExtension.hpp"

#include <extensionsystem/ExtensionManager.hpp>

#include <QtPlugin>
#include <QtDebug>

/*!
    \namespace Alsa
    \brief Alsa audio backend

    Uses Alsa to provide an implementation of Unison::Backend to be used by
    Core::Engine.
*/

/*!
    \namespace Alsa::Internal
    \internal
    \brief BLAH
*/

namespace Alsa {
  namespace Internal {

AlsaExtension::AlsaExtension()
{
}


AlsaExtension::~AlsaExtension()
{
  qDebug() << "Alsa dtor";
  // BackendProvider is auto-released
}

bool AlsaExtension::initialize(const QStringList& arguments, QString* errorMessage)
{
  Q_UNUSED(errorMessage);
  parseArguments(arguments);

  addAutoReleasedObject(new AlsaBackendProvider(NULL, m_inputDevice, m_outputDevice));
  return true;
}

void AlsaExtension::parseArguments(const QStringList & arguments)
{
  // default options
  m_inputDevice = "default";
  m_outputDevice = "default";

  // TODO: override with configuration options

  // override with arguments
  for (int i = 0; i < arguments.size() - 1; i++) {
    bool has_value = arguments.size() - 1 >= i + 1;
    if (arguments.at(i) == "--input-device") {
      Q_ASSERT(has_value);
      if (! has_value)
        continue;
      if (arguments.size() < i)
      m_inputDevice = arguments.at(i + 1);
      i++; // skip the value
    } else if (arguments.at(i) == "--output-device") {
      Q_ASSERT(has_value);
      if (! has_value)
        continue;
      m_outputDevice = arguments.at(i + 1);
      i++; // skip the value
    }
  }
}

void AlsaExtension::extensionsInitialized()
{
}


void AlsaExtension::remoteCommand(const QStringList& options, const QStringList& args)
{
}


void AlsaExtension::shutdown()
{
  qDebug() << "Alsa shutdown";
}

EXPORT_EXTENSION(AlsaExtension)

  } // Internal
} // Alsa

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
