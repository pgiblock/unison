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
#include "ISampleBufferReader.h"

// For Engine
#include "Engine.h"
#include <unison/Backend.h>
#include <unison/BufferProvider.h>
#include <unison/Commander.h>
#include <unison/Patch.h>
#include <unison/PooledBufferProvider.h>
#include <unison/SampleBuffer.h>

// For connection frenzy
#include "FxLine.h"
#include "StupidSamplerDemo.h"
#include "PluginManager.h"
#include <unison/Plugin.h>

#include <extensionsystem/ExtensionManager.h>

#include <QtDebug>
#include <QtPlugin>
#include <QApplication>
#include <QTimer>

using namespace ExtensionSystem;
using namespace Unison;

namespace Core {
  namespace Internal {

CoreExtension::CoreExtension() :
  m_lineCount(4)
//  m_mainWindow(new MainWindow), m_editMode(0)
{
}


CoreExtension::~CoreExtension()
{
  qDebug() << "CORE dtor";
  if (Engine::backend()) {
    delete Engine::backend();
  }
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


void CoreExtension::parseArguments(const QStringList& arguments)
{
  for (int i = 0; i < arguments.size() - 1; i++) {
    if (arguments.at(i) == QLatin1String("--infile")) {
      i++; // skip to argument
      m_sampleInfile = arguments.at(i);
    }
    if (arguments.at(i) == QLatin1String("--seconds")) {
      bool ok;
      float timeout = arguments.at(i + 1).toFloat(&ok);
      if (ok) {
        int timeoutMS = timeout*1000;
        QTimer::singleShot(timeoutMS, qApp, SLOT(quit()));
      }
      i++; // skip the value
    }
    if (arguments.at(i) == QLatin1String("--lines")) {
      bool ok;
      int count = arguments.at(i + 1).toInt(&ok);
      if (ok) {
        m_lineCount = count;
      }
      i++; // skip the value
    }
  }
}


bool CoreExtension::initialize(const QStringList& arguments, QString* errorMessage)
{
  Q_UNUSED(errorMessage);
  parseArguments(arguments);

  PooledBufferProvider* bufProvider = new PooledBufferProvider();
  bufProvider->setBufferLength(1024);
  Engine::setBufferProvider(bufProvider);

  PluginManager::initializeInstance();

  Unison::Internal::Commander::initialize();

  /*
  const bool success = m_mainWindow->init(errorMessage);
  if (success) {
    EditorManager* editorManager = m_mainWindow->editorManager();
    m_editMode = new EditMode(editorManager);
    addObject(m_editMode);

    m_designMode = new DesignMode(editorManager);
    addObject(m_designMode);
  }
  return success;
  */
  return true;
}


/**
 * TODO: just about everything allocated here is leaked. It is quick-nasty demo code
 * at the moment, but should be cleaned-up anyways */
void CoreExtension::extensionsInitialized()
{
  ExtensionManager* extMgr = ExtensionManager::instance();

  // Loading a mix of LADSPA and LV2
  QStringList plugins;
  plugins //<< "http://ladspa.org/plugins/2143"
          << "http://calf.sourceforge.net/plugins/Reverb"
          << "http://calf.sourceforge.net/plugins/Phaser";

  // Find backends, load the first one
  QList<IBackendProvider*> backends = extMgr->getObjects<IBackendProvider>();

  if (backends.count() == 0) {
    qWarning("No backends found, I guess we aren't doing anything");
    return;
  }
  qDebug("Found Backends:");
  foreach (IBackendProvider* bep, backends) {
    qDebug() << bep->displayName();
  }

  // We gain control of created backends
  Backend* backend = backends.at(0)->createBackend();

  Patch* root = new Patch();
  backend->setRootProcessor(root);

  Engine::setBackend(backend);

  backend->activate();
  
  const int effects = 2; // * 5 * 2

  for (int l = 1; l <= m_lineCount; ++l) {

    FxLine* fxLine = new FxLine(*root, QString("Super Duper Fx-Line %1").arg(l));

    for (int i = 0; i < plugins.size(); ++i) {
      int j = 0;
      QString plugin = plugins.at(i);
      PluginInfoPtr desc = PluginManager::instance()->info(plugin);
      // Add N of each
      for (int cnt = 0; cnt < effects; ++cnt) {
        if (desc) {
          fxLine->addPlugin(desc, j++);
        }
        else {
          qWarning() << "Could not load plugin: " << plugin;
        }
      }
    }
  }

  // Stupid Sampler
  Demo::StupidSamplerDemo *ssd = new Demo::StupidSamplerDemo(root, "Stupid sampler");
  SampleBuffer *buf = NULL;
  if (!m_sampleInfile.isNull()) {
    QList<ISampleBufferReader *> readers = extMgr->getObjects<ISampleBufferReader>();
    QListIterator<ISampleBufferReader *> i(readers);
    while (i.hasNext() && buf == NULL) {
      buf = i.next()->read(m_sampleInfile);
    }
  }
  else {
    // Fall-back Sawtooth oscillator
    int length = 48000.0f / 440.0f;
    sample_t *samples = new sample_t[length*2]; // 2 chans 
    sample_t *s = samples;
    sample_t val;
    for(int i=0; i<length; ++i) {
      val = (2.0f*i/length) - 1.0f;  // Range is -1,1
      *(s++) = val; // clone left
      *(s++) = val; //   and right
    }
    buf = new SampleBuffer(samples, length, 2, 48000.0f);
  }
  if (buf) {
    ssd->setSampleBuffer(buf);
  }

  //m_mainWindow->extensionsInitialized();
}


void CoreExtension::remoteCommand(const QStringList& options, const QStringList& args)
{
  Q_UNUSED(options)
  Q_UNUSED(args)
  //m_mainWindow->openFiles(args);
  //m_mainWindow->activateWindow();
}

/*
void CoreExtension::fileOpenRequest(const QString& f)
{
  remoteCommand(QStringList(), QStringList(f));
}
*/

void CoreExtension::shutdown()
{
  qDebug() << "CORE shutdown";

  // We don't need to be processing while shutting down
  // TODO: Probably end up killing the whole engine here
  if (Engine::backend()) {
    Engine::backend()->deactivate();
  }

  //m_mainWindow->shutdown();
}

EXPORT_EXTENSION(CoreExtension)

  } // Internal
} // Core

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
