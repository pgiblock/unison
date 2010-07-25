/*
 * CoreExtension.cpp
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

#include "CoreExtension.h"

#include "IBackendProvider.h"

// For Engine
#include "Engine.h"
#include <unison/Backend.h>
#include <unison/BufferProvider.h>
#include <unison/Commander.h>
#include <unison/Patch.h>
#include <unison/PooledBufferProvider.h>

// For connection frenzy
#include "FxLine.h"
#include "PluginManager.h"
#include <unison/Plugin.h>

#include <extensionsystem/ExtensionManager.h>

#include <QtPlugin>
#include <QtDebug>

using namespace ExtensionSystem;
using namespace Unison;

namespace Core {
  namespace Internal {

CoreExtension::CoreExtension()
//  m_mainWindow(new MainWindow), m_editMode(0)
{
}


CoreExtension::~CoreExtension()
{
  /*
  if (m_editMode) {
      removeObject(m_editMode);
      delete m_editMode;
  }

  if (m_designMode) {
      removeObject(m_designMode);
      delete m_designMode;
  }

  // delete FileIconProvider singleton
  delete FileIconProvider::instance();

  delete m_mainWindow;
  */
}


void CoreExtension::parseArguments(const QStringList &arguments)
{
  Q_UNUSED(arguments)
  /*
  for (int i = 0; i < arguments.size() - 1; i++) {
    if (arguments.at(i) == QLatin1String("-color")) {
      const QString colorcode(arguments.at(i + 1));
      m_mainWindow->setOverrideColor(QColor(colorcode));
      i++; // skip the argument
    }
  }
  */
}


bool CoreExtension::initialize(const QStringList &arguments, QString *errorMessage)
{
  Q_UNUSED(errorMessage);
  parseArguments(arguments);

  PooledBufferProvider *bufProvider = new PooledBufferProvider();
  bufProvider->setBufferLength(1024);
  Engine::setBufferProvider(bufProvider);

  PluginManager::initializeInstance();

  Unison::Internal::Commander::initialize();

  /*
  const bool success = m_mainWindow->init(errorMessage);
  if (success) {
    EditorManager *editorManager = m_mainWindow->editorManager();
    m_editMode = new EditMode(editorManager);
    addObject(m_editMode);

    m_designMode = new DesignMode(editorManager);
    addObject(m_designMode);
  }
  return success;
  */
  return true;
}


void CoreExtension::extensionsInitialized()
{
  ExtensionManager *extMgr = ExtensionManager::instance();
  
  // Find backends, load the first one
  QList<IBackendProvider *> backends = extMgr->getObjects<IBackendProvider>();
  qDebug("Found Backends:");
  foreach (IBackendProvider *bep, backends) {
    qDebug() << bep->displayName();
  }

  Backend *backend = backends.at(0)->createBackend();
  
  Patch *root = new Patch();
  backend->setRootProcessor(root);
  
  Engine::setBackend(backend);

  backend->activate();

  FxLine *fxLine = new FxLine(*root, "Super Duper Fx-Line");

  PluginDescriptorPtr desc;
  desc = PluginManager::instance()->descriptor("http://plugin.org.uk/swh-plugins/vynil");
  fxLine->addPlugin(desc, 0);
  desc = PluginManager::instance()->descriptor("http://calf.sourceforge.net/plugins/Reverb");
  fxLine->addPlugin(desc, 0);
  //desc = PluginManager::instance()->descriptor("http://calf.sourceforge.net/plugins/Phaser");
  //fxLine->addPlugin(desc, 0);

  // TODO: cleanup
  // Let these ports leak all over the place. This is a stupid demo
  //backend->deactivate();

  //m_mainWindow->extensionsInitialized();
}


void CoreExtension::remoteCommand(const QStringList &options, const QStringList &args)
{
  Q_UNUSED(options)
  Q_UNUSED(args)
  //m_mainWindow->openFiles(args);
  //m_mainWindow->activateWindow();
}

/*
void CoreExtension::fileOpenRequest(const QString &f)
{
  remoteCommand(QStringList(), QStringList(f));
}
*/

void CoreExtension::shutdown()
{
  //m_mainWindow->shutdown();
}

EXPORT_EXTENSION(CoreExtension)

  } // Internal
} // Core

// vim: ts=8 sw=2 sts=2 et sta noai
