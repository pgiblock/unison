/*
 * Lv2Extension.h
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


#ifndef UNISON_GUICOREEXTENSION_H
#define UNISON_GUICOREEXTENSION_H

#include <extensionsystem/IExtension.h>

namespace Lv2 {
//class DesignMode;
namespace Internal {

//class EditMode;
//class MainWindow;

class Lv2Extension : public ExtensionSystem::IExtension
{
  Q_OBJECT

public:
  Lv2Extension();
  ~Lv2Extension();

  virtual bool initialize(const QStringList &arguments, QString *errorMessage = 0);
  virtual void extensionsInitialized();
  virtual void shutdown();
  virtual void remoteCommand(const QStringList & /* options */, const QStringList &args);

//public slots:
//    void fileOpenRequest(const QString&);

private:
  void parseArguments(const QStringList & arguments);

//    MainWindow *m_mainWindow;
//    EditMode *m_editMode;
//    DesignMode *m_designMode;
};

} // namespace Internal
} // namespace Core

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
