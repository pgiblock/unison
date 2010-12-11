/*
 * AlsaExtension.hpp
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


#ifndef UNISON_ALSAEXTENSION_H
#define UNISON_ALSAEXTENSION_H

#include <extensionsystem/IExtension.hpp>

namespace Alsa {
  namespace Internal {

class AlsaExtension : public ExtensionSystem::IExtension
{
  Q_OBJECT

public:
  AlsaExtension();
  ~AlsaExtension();

  virtual bool initialize(const QStringList& arguments, QString* errorMessage = 0);
  virtual void extensionsInitialized();
  virtual void shutdown();
  virtual void remoteCommand(const QStringList& options, const QStringList& args);

private:
  QString m_inputDevice;
  QString m_outputDevice;

  void parseArguments(const QStringList & arguments);

};

  } // namespace Internal
} // namespace Alsa

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
