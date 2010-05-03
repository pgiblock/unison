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
#include <math.h>

#include <QDebug>
#include <QtGui/QApplication>
#include <QSet>
#include "unison/JackEngine.h"
#include "unison/PluginManager.h"
#include "unison/ProcessingContext.h"
#include "unison/Session.h"

#include "unison/CompositeProcessor.h"

using namespace Unison;

void printLogo();
void printDisclaimer();

Session * session;

Port * inputL;
Port * inputR;

class FxLine : public CompositeProcessor {
  public:
    FxLine (Session& session, QString name) :
      CompositeProcessor(),
      m_name(name),
      m_session(session)
    {
      m_inPorts[0] = session.engine().registerPort(name + "/in 1", OUTPUT);
      m_inPorts[1] = session.engine().registerPort(name + "/in 2", OUTPUT);
      m_outPorts[0] = session.engine().registerPort(name + "/out 1", INPUT);
      m_outPorts[1] = session.engine().registerPort(name + "/out 2", INPUT);
    }

    ~FxLine ()
    {
      m_session.engine().unregisterPort(m_inPorts[0]);
      m_session.engine().unregisterPort(m_inPorts[1]);
      m_session.engine().unregisterPort(m_outPorts[0]);
      m_session.engine().unregisterPort(m_outPorts[1]);
    }

    QString name() const
    {
      return m_name;
    }

    void addEffect()
    {
      const char * uri = "http://calf.sourceforge.net/plugins/Phaser";
      PluginManager * man = PluginManager::instance();

      Processor * proc = man->descriptor(uri)->createPlugin(48000);
      add(proc);

      // TODO: support for port-groups
      // PortGroup { enum Type {STEREO, QUAD, FIVEPOINTONE}
      // or maybe a processor.portGroups() ?

      int inCnt=0, outCnt =0;
      for (int i=0; i<proc->portCount(); ++i) {
        Port* p = proc->port(i);
        if (p->type() == AUDIO_PORT) {
          switch (p->direction()) {
            case INPUT:
              qDebug() << "Connecting " << m_inPorts[inCnt]->name() << " to " << p->name();
              m_inPorts[inCnt++]->connect(p);
              break;
            case OUTPUT:
              qDebug() << "Connecting " << m_outPorts[outCnt]->name() << " to " << p->name();
              m_outPorts[outCnt++]->connect(p);
              break;
            default:
              //TODO: Programming error!
              break;
          }
        }
      }

      // Now, we are just going to add stuff for testing compiling.
      /*
      Processor * osc1  = man->descriptor("http://plugin.org.uk/swh-plugins/sinCos")->createPlugin(48000);
      Port * osc1freq   = osc1->port(0);      // Base frequency (Hz)"
      Port * osc1pitch  = osc1->port(1);      // Pitch offset"
      Port * osc1outSin = osc1->port(2);      // Sine output"
      Port * osc1outCos = osc1->port(3);      // Cosine output"
      osc1pitch->setValue(16.0f);
      osc1freq->setValue(440.0f / pow(2.0, 16.0) );
      add(osc1);

      Processor * amp1  = man->descriptor("http://plugin.org.uk/swh-plugins/amp")->createPlugin(48000);
      Port * amp1gain   = amp1->port(0);
      Port * amp1in     = amp1->port(1);
      Port * amp1out    = amp1->port(2);
      add(amp1);

      Processor * lfo1 = man->descriptor("http://unisonstudio.org/plugins/LfoController")->createPlugin(48000);
      Port * lfo1out   = lfo1->port(0);
      add(lfo1);

      //osc1pitch->setValue(8.0f);
      //amp1gain->setValue(-50.0f);
      //lfo1out->connect(amp1gain);
      //osc1out->connect(amp1in);

      //inputL->connect(osc1outSin);
      //inputR->connect(amp1out);
      m_outPorts[0]->connect(osc1outSin);
      m_outPorts[1]->connect(osc1outCos);
      //amp1out->connect(inputR);
    */

      hackCompile(m_session.bufferProvider());
    }


  private:
    QString m_name;
    Session& m_session;
    JackPort* m_inPorts[2];
    JackPort* m_outPorts[2];
};


class TestProcessor : public CompositeProcessor {

  public:
    void add (Processor * processor) {
      CompositeProcessor::add(processor);
    }

    void remove (Processor * processor) {
      CompositeProcessor::remove(processor);
    }
};


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

  // Init
  PluginManager::initializeInstance();

  // TODO: Obviously we wouldnt really (mis)manage these this way.
  JackEngine* engine = new JackEngine();
  session = new Session(*engine);
  session->hackCompile();
  engine->activate();


  // Client stuff
  FxLine* fxLine = new FxLine(*session, "Master");
  fxLine->addEffect();
  fxLine->activate();

  session->add(fxLine);
  session->hackCompile();

  char c;
  std::cin >> &c;

  fxLine->deactivate();

  std::cout << "Disconnecting JACK" << std::endl;
  engine->deactivate();

  /*
  //app->exec();

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
