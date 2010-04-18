/*
 * main.cpp
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

#include <iostream>

#include <QtGui/QApplication>
#include <QSet>
#include "unison/JackEngine.h"
#include "unison/PluginManager.h"
#include "unison/ProcessingContext.h"
#include "unison/Session.h"

using namespace Unison;

void printLogo();
void printDisclaimer();

Session * session;

/** A stupid function to alias ports so we can connect to them */
/*
Port* fxin[2][2];
Port* fxout[2][2];
Processor* extraProcessor;
void referencePorts () {
  std::cout << "Referencing Ports" << std::endl;
  int f=0;
  foreach (Processor* n, processors) {
    int inCnt=0, outCnt =0;
    for (int i=0; i<n->portCount(); ++i) {
      Port* p = n->port(i);
      if (p->type() == AUDIO_PORT) {
        switch (p->direction()) {
          case INPUT:
            fxin[f][inCnt++] = p;
            break;
          case OUTPUT:
            fxout[f][outCnt++] = p;
            break;
          default:
            //TODO: Programming error!
            break;
        }
      }
    }
    f++;
  }
}*/


void bigCompile () {
  /* TODO-NOW: refactor this to someplace else
  std::cout << "Compiling" << std::endl;
  QList<CompiledProcessor>* compiledSwap = new QList<CompiledProcessor>();
  compile( processors, *compiledSwap );

  std::cout << "Aquiring 'fixed' buffers" << std::endl;
  foreach (CompiledProcessor cp, *compiledSwap) {
    for (int i=0; i<cp.processor->portCount(); ++i) {
      Port *port = cp.processor->port(i);
      std::cout << "Next port: " << qPrintable(port->name()) << std::endl;
      port->connectToBuffer(*pool);
    }
  }

  // FIXME: this is probably not sufficient.
  compiledSwap = compiled.fetchAndStoreRelaxed( compiledSwap );
  delete compiledSwap;
  */
}


int main (int argc, char ** argv) {
  bool createGui = false;

  /* No need to create this yet
  QCoreApplication * app = createGui ?
    new QApplication( argc, argv ) :
    new QCoreApplication( argc, argv );

  app->setApplicationName( "Unison" );
  app->setOrganizationDomain( "unison.sourceforge.net" );
    app->setOrganizationName( "Paul Giblock" );
  */

  printLogo();
  // If running in CLI mode, print a disclaimer
  printDisclaimer();

  // TODO: Obviously we wouldnt really (mis)manage these this way.
  JackEngine* engine = new JackEngine();
  session = new Session(*engine);

  JackPort* jackPorts[6];
  jackPorts[0] = engine->registerPort("Master/out 1",  INPUT);
  jackPorts[1] = engine->registerPort("Master/out 2",  INPUT);
  jackPorts[2] = engine->registerPort("Master/in 1",   OUTPUT);
  jackPorts[3] = engine->registerPort("Master/in 2",   OUTPUT);
  jackPorts[4] = engine->registerPort("Channel/out 1", INPUT);
  jackPorts[5] = engine->registerPort("Channel/out 2", INPUT);

  // Init
  /*
  PluginManager::initializeInstance();

  PluginManager * man = PluginManager::instance();

  std::cout << "Creating Plugins" << std::endl;
  processors.append(man->descriptor("http://plugin.org.uk/swh-plugins/vynil")
                  ->createPlugin(48000));
  extraProcessor = man->descriptor(
      //"http://calf.sourceforge.net/plugins/VintageDelay"
      //"http://calf.sourceforge.net/plugins/Reverb"
      //"http://calf.sourceforge.net/plugins/RotarySpeaker"
      //"http://calf.sourceforge.net/plugins/MultiChorus"
      "http://calf.sourceforge.net/plugins/Flanger"
      )->createPlugin(48000);

  std::cout << "Activating Plugins" << std::endl;
  foreach (Processor* n, processors) { n->activate(); }

  //referencePorts();

   TODO-NOW: Control Port stuff
  if (type() == CONTROL_PORT) {
    float * data = (float*)m_buffer->data();
    data[0] = maximum();
  }

  std::cout << "Connecting Ports" << std::endl;
  fxout[0][0]->connect(jackPorts[4]); // Vinyl to Channel-L
  jackPorts[5]->connect(fxout[0][1]); // Vinyl to Channel-R

  bigCompile();

  std::cout << "Processing Nodes" << std::endl;
  jackEngine->activate();

  //app->exec();
  char c;
  std::cin >> &c;

  // Add
  processors.append(extraProcessor);
  referencePorts();
  */

  // Rewire
  /*
  fxout[0][0]->disconnect(jackPorts[4]); // Vinyl from Channel-L
  fxout[0][1]->disconnect(jackPorts[5]); // Vinyl from Channel-R
  fxout[0][0]->connect(fxin[1][0]);  // Vinyl to Delay (L)
  fxout[0][1]->connect(fxin[1][1]);  // Vinyl to Delay (R)
  fxout[1][0]->connect(jackPorts[4]); // Delay to Channel-L
  fxout[1][1]->connect(jackPorts[5]); // Delay to Channel-R
  */

  // Recompile
  bigCompile();

  //app->exec();
  char c;
  std::cin >> &c;

  std::cout << "Disconnecting JACK" << std::endl;
  engine->deactivate();

  /*
  std::cout << "Deactivating Plugins" << std::endl;
  foreach (Processor * p, processors) { p->deactivate(); }

  std::cout << "Destroying Plugins" << std::endl;
  foreach (Processor * p, processors) { delete p; }
  delete compiled;

  std::cout << "Destorying PluginManager (Do more gracefully)" << std::endl;
  PluginManager::cleanupHack();
  */

  std::cout << "Bye!" << std::endl;
  return 0;
}


/** Draws an ascii UNISON logo */
void printLogo()
{
  std::cout <<
      "   __  ___  ______________  _  __\n"
      "  / / / / |/ /  _/ __/ __ \\/ |/ /\n"
      " / /_/ /    // /_\\ \\/ /_/ /    / \n"
      " \\____/_/|_/___/___/\\____/_/|_/  \n\n";
}


/** Draws the GPL mandated disclaimer */
void printDisclaimer()
{
  std::cout <<
      "Unison version 0, Copyright (C) 2010 Paul R Giblock\n"
      "Unison comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n"
      "This is free software, and you are welcome to redistribute it\n"
      "under certain conditions; type `show c' for details.\n\n";
}

// vim: ts=8 sw=2 sts=2 et sta noai
