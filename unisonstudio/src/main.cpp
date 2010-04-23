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
      const char * uri = "http://plugin.org.uk/swh-plugins/vynil";
      PluginManager * man = PluginManager::instance();
      Processor * proc = man->descriptor(uri)->createPlugin(48000);
      add(proc);

      // TODO: Some fashion to connect the ports easily
      // Should consider groups and stuff:
      // processor.portGroup(AUDIO, INPUT)
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
      hackCompile(m_session.bufferProvider());
    }


  private:
    QString m_name;
    Session& m_session;
    JackPort* m_inPorts[2];
    JackPort* m_outPorts[2];
};


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

  engine->activate();

  // Client stuff
  FxLine* fxLine = new FxLine(*session, "Master");
  fxLine->addEffect();

  fxLine->activate();

  session->setRootNodeHack(fxLine);

  char c;
  std::cin >> &c;

  fxLine->deactivate();

  std::cout << "Disconnecting JACK" << std::endl;
  engine->deactivate();

  /*
   TODO-NOW: Control Port stuff

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
