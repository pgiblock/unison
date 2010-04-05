#include <iostream>

#include <QtGui/QApplication>
#include <QSet>
#include <jack/jack.h>
#include "unison/BufferProvider.h"
#include "unison/PluginManager.h"
#include "unison/ProcessingContext.h"
#include "unison/JackEngine.h"

using namespace Unison;

void printLogo();
void printDisclaimer();
int processCb(jack_nframes_t nframes, void* data);

// Engine stuff
jack_client_t * jackClient;
JackEngine * jackEngine;
PooledBufferProvider * pool;

// Node referencing
Port* fxin[2][2];
Port* fxout[2][2];
JackPort* jackPorts[6];
QList<Processor*> processors;

struct CompiledProcessor {
  Processor * processor;
};


QAtomicPointer< QList<CompiledProcessor> > compiled;

int numBuffers = 0;
int numBuffersNeeded = 0;

void compileRecursive (Node* n, QList<CompiledProcessor>& output)
{
  // TODO: Dynamic cast is bad. any better way to do this other
  // than moving process(), visit(), isVisited() etc to Node?
  Processor* p = dynamic_cast<Processor*>( n );
  if (p == NULL || !p->isVisited()) {
    std::cout << "  " << qPrintable(n->name()) << std::endl;
    if (p) {
      p->visit();
    }
    foreach (Node* dep, n->dependencies()) {
      compileRecursive( dep, output );
    }
    if (p) {
      CompiledProcessor cp;
      cp.processor = p;
      output.append(cp);
    }
  }
}
/*
  Alternate compilation method:
  create set of all nodes - these are "untraversed".
  remove from traversed, recurse into dependents, add to compiled-list
*/


void compile (QList<Processor*> input, QList<CompiledProcessor>& output) {

  // Mark everything as unvisited
  QListIterator<Processor*> i( input );
  while (i.hasNext()) {
    i.next()->unvisit();
  }

  // Process nodes that are pure-sinks first
  i.toFront();
  while (i.hasNext()) {
    Processor* proc = i.next();

    bool isSink = true;

    // For each output port
    for (int j = 0; j < proc->portCount(); ++j) {
      Port* port = proc->port(j);
      if (port->direction() == OUTPUT) {
        // For all connected Ports.
        QSetIterator<Node* const> k( port->dependents() );
        while (k.hasNext()) {
          Port* otherPort = (Port*)k.next();
          // Not a sink if a connected port has any dependents.
          if (otherPort->dependents().count() != 0) {
            isSink = false;
            goto notSink;
          }
        }
      }
    }
    notSink:
    if (isSink) {
      compileRecursive( proc, output );
    }
  }

  // Then compile everything else
  QListIterator<Processor*> p( input );
  while (p.hasNext()) {
    compileRecursive( p.next(), output );
  }
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

	// JACK stuff
	jackClient = jack_client_open("Unison Studio", JackNullOption, NULL);
	if (!jackClient) { printf("Failed to connect to JACK."); }
	else { printf("Connected to JACK.\n"); }

        jackEngine = new JackEngine( jackClient );
	jack_set_process_callback(jackClient, &processCb, NULL);

        jackPorts[0] = jackEngine->registerPort("Master/out 1", INPUT);
	jackPorts[1] = jackEngine->registerPort("Master/out 2", INPUT);
	jackPorts[2] = jackEngine->registerPort("Master/in 1", OUTPUT);
	jackPorts[3] = jackEngine->registerPort("Master/in 2", OUTPUT);
	jackPorts[4] = jackEngine->registerPort("Channel/out 1", INPUT);
	jackPorts[5] = jackEngine->registerPort("Channel/out 2", INPUT);

	// Init
	PluginManager::initializeInstance();

        pool = new PooledBufferProvider();
        pool->setBufferLength(jack_get_buffer_size(jackClient));

        PluginManager * man = PluginManager::instance();

	std::cout << "Creating Plugins" << std::endl;
	processors.append(man->descriptor("http://plugin.org.uk/swh-plugins/lfoPhaser")
			->createPlugin(48000));
	processors.append(man->descriptor("http://plugin.org.uk/swh-plugins/vynil")
			->createPlugin(48000));

	std::cout << "Activating Plugins" << std::endl;
	foreach (Processor* n, processors) { n->activate(); }

	std::cout << "Referencing Ports" << std::endl;
	float controls[] = {0.5f, 0.5f, 0.5f, 0.5f};
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

	std::cout << "Connecting Ports" << std::endl;
	fxout[1][0]->connect(fxin[0][0]);  // Vinyl to LFO
	fxout[0][0]->connect(jackPorts[4]); // LFO to Channel-L
	jackPorts[5]->connect(fxout[1][1]); // Vinyl to Channel-R

        //fxout[1][1]->connect(jackPorts[1]);
	//jackPorts[4]->connect(fxout[1][0]); // To channel out
	//fxin[2][0]->connect(jackPorts[2]);  // To master In
	//jackPorts[0]->connect(fxout[2][0]); // To master out


	std::cout << "Compiling" << std::endl;
	QList<CompiledProcessor>* compiledSwap = new QList<CompiledProcessor>();
	compile( processors, *compiledSwap );
        compiledSwap = compiled.fetchAndStoreRelaxed( compiledSwap );
        delete compiledSwap;

	for (QList<CompiledProcessor>::iterator i = compiled->begin(); i != compiled->end(); ++i) {
		std::cout << qPrintable(i->processor->name()) << " -- ";
	}
	std::cout << std::endl;


        std::cout << "Aquiring 'fixed' buffers" << std::endl;
	foreach (CompiledProcessor cp, *compiled) {
          for (int i=0; i<cp.processor->portCount(); ++i) {
            Port *port = cp.processor->port(i);
            std::cout << "Next port: " << qPrintable(port->name()) << std::endl;
            port->aquireBuffer(*pool);
            port->connectToBuffer();
          }
	}


	std::cout << "Processing Nodes" << std::endl;
	jack_activate(jackClient);

        //app->exec();
        char c;
	std::cin >> &c;

	std::cout << "Disconnecting JACK" << std::endl;
	jack_deactivate(jackClient);
	// TODO: Disconnect jack
	jack_client_close(jackClient);

	std::cout << "Deactivating Plugins" << std::endl;
	foreach (Processor * p, processors) { p->deactivate(); }

	std::cout << "Destroying Plugins" << std::endl;
	foreach (Processor * p, processors) { delete p; }
	delete compiled;

        std::cout << "Destorying PluginManager (Do more gracefully)" << std::endl;
        PluginManager::cleanupHack();

	std::cout << "Bye!" << std::endl;
	return 0;
}



int processCb (jack_nframes_t nframes, void* data)
{
	ProcessingContext context( nframes );

        // Aquire JACK buffers
        for (int i=0; i<sizeof jackPorts / sizeof jackPorts[0]; ++i) {
          Port *port = jackPorts[i];
          port->aquireBuffer(*pool);
          port->connectToBuffer();

          // Re-aquire buffers on ports connected to JACK
	  foreach (Port *other, port->connectedPorts()) {
            other->aquireBuffer(*pool);
            other->connectToBuffer();
          }
        }

	// Processing loop
	foreach (CompiledProcessor cp, *compiled) {
	  cp.processor->process(context);
	}

	return 0;
}



/** Draws an ascii UNISON logo */
void printLogo() {
    std::cout <<
        "   __  ___  ______________  _  __\n"
        "  / / / / |/ /  _/ __/ __ \\/ |/ /\n"
        " / /_/ /    // /_\\ \\/ /_/ /    / \n"
        " \\____/_/|_/___/___/\\____/_/|_/  \n\n";
}


/** Draws the GPL mandated disclaimer */
void printDisclaimer() {
	std::cout <<
		"Unison version 0, Copyright (C) 2010 Paul R Giblock\n"
		"Unison comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n"
		"This is free software, and you are welcome to redistribute it\n"
		"under certain conditions; type `show c' for details.\n\n";
}

// vim: et ts=8 sw=2 sts=2 noai
