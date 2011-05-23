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

#define USE_QT_SIGNALS

#include "CoreExtension.hpp"

#include "IDriverProvider.hpp"
#include "ISampleBufferReader.hpp"

#include "ingen/EngineBase.hpp"
#include "ingen/ServerInterface.hpp"
#include "ingen/client/ClientStore.hpp"
#include "ingen/shared/World.hpp"

// For Engine
#include "BackgroundStuff.hpp"
#include "Engine.hpp"
#include "ingen/client/PluginModel.hpp"
#include "ingen/client/SigClientInterface.hpp"
#include "ingen/shared/runtime_paths.hpp"
#include <extensionsystem/ExtensionManager.hpp>

// For Silly demo
#include "raul/SharedPtr.hpp"                                                                                      
#include "ingen/Resource.hpp"                                                                                      
#include "ingen/ServerInterface.hpp"                                                                               

#include <QtDebug>
#include <QtPlugin>
#include <QApplication>
#include <QTimer>

using namespace ExtensionSystem;
using namespace Raul;

namespace Core {
  namespace Internal {

void ingen_interrupt (Ingen::Shared::World* world, int)
{
  qWarning("ingen: Interrupted");
  if (world->local_engine())
    world->local_engine()->quit();
  delete world;
  exit(EXIT_FAILURE);
}


void ingen_try (Ingen::Shared::World* world, bool cond, const char* msg)
{
  if (!cond) {
    qCritical("ingen: Error: %s", msg);
    delete world;
    exit(EXIT_FAILURE);
  }
}


CoreExtension::CoreExtension()
{
}


CoreExtension::~CoreExtension()
{
  qDebug() << "CORE dtor";

  if (m_engine) {
    removeObject(m_engine);
    delete m_engine;
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
    if (arguments.at(i) == QLatin1String("--seconds")) {
      bool ok;
      float timeout = arguments.at(i + 1).toFloat(&ok);
      if (ok) {
        int timeoutMS = timeout*1000;
        QTimer::singleShot(timeoutMS, qApp, SLOT(quit()));
      }
      i++; // skip the value
    }
  }
}


bool CoreExtension::initialize(const QStringList& arguments, QString* errorMessage)
{
  Q_UNUSED(errorMessage);
  parseArguments(arguments);

  int         ingen_argc = 3;
  const char* ingen_argv_data[] = {"ingen", "-n", "Unison"};
  char**      ingen_argv = new char*[ingen_argc];
  for (int i = 0; i < ingen_argc; ++i) {
    ingen_argv[i] = strdup(ingen_argv_data[i]);
  }

  try {
    conf.parse(ingen_argc, ingen_argv);
  }
  catch (std::exception& e) {
    qFatal("ingen: %s\n", e.what());
  }

  // FIXME: Don't hardcode path like a noob
  Ingen::Shared::set_bundle_path("/usr/local/bin");

  Ingen::Shared::World* world = new Ingen::Shared::World(&conf, ingen_argc, ingen_argv);

  ingen_try(world, world->load_module("server"),
            "Unable to load server module");

  ingen_try(world, world->local_engine(),
            "Unable to create engine");

  m_engine = new Engine();
  m_engine->setIngenWorld(world);
  addObject(m_engine);

  /*
  for (int i = 0; i < ingen_argc; ++i) {
    free(ingen_argv[i]);
  }
  delete[] ingen_argv;
  */

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
  Ingen::Shared::World* world = m_engine->ingenWorld();
  SharedPtr<Ingen::ServerInterface> server = world->engine();

  // Activate the engine, if we have one (only allow for one right now)
  IDriverProvider* driverPrvdr = ExtensionManager::instance()->getObject<IDriverProvider>();
  ingen_try(world, driverPrvdr->loadDriver(),
            "Unable to load jack module");

  world->set_engine(server);

  // Activate
  world->local_engine()->activate();

  SharedPtr<Ingen::Client::SigClientInterface> client(new Ingen::Client::SigClientInterface());
  server->register_client(client.get());

  // Just playing around for now...

  BackgroundStuff* bkgrnd = new BackgroundStuff(this, world);

  Ingen::Client::ClientStore* store =
      new Ingen::Client::ClientStore(world->uris(), world->engine(), client);
  PluginModel::set_lilv_world(world->lilv_world());
  PluginModel::set_rdf_world(*world->rdf_world());
  m_engine->setStore(store);

  ////////////////////////////////////
  // Silly Demo:

  // Is this even needed?
  server->get("ingen:plugins"); // TODO: QIngen::Server or QIngen::ServerInterface

  server->bundle_begin();

  Ingen::Resource::Properties props;
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#AudioPort")));
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#InputPort")));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#index",  Atom(int32_t(2))));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#name",   "Wire L"));
  server->put("path:/wire_l_in", props);

  props = Ingen::Resource::Properties();
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#AudioPort")));
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#OutputPort")));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#index",  Atom(int32_t(3))));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#name",   "Wire L"));
  server->put("path:/wire_l_out", props);

  props = Ingen::Resource::Properties();
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#AudioPort")));
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#InputPort")));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#index",  Atom(int32_t(4))));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#name",   "Wire R"));
  server->put("path:/wire_r_in", props);

  props = Ingen::Resource::Properties();
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#AudioPort")));
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#OutputPort")));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#index",  Atom(int32_t(5))));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#name",   "Wire R"));
  server->put("path:/wire_r_out", props);

  server->connect("path:/wire_l_in", "path:/wire_l_out");
  server->connect("path:/wire_r_in", "path:/wire_r_out");

  server->bundle_end();

  // Do this after the event loop has started

  /*
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
*/
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

  //m_mainWindow->shutdown();
}

EXPORT_EXTENSION(CoreExtension)

  } // Internal
} // Core

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
