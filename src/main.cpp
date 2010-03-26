#include <iostream>

#include <QtGui/QApplication>
#include <QSet>
#include <jack/jack.h>
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

// Node referencing
Port* fxin[2][2];
Port* fxout[2][2];
JackPort* jackPorts[6];
QList<Processor*> processors;

QAtomicPointer< QList<Processor*> > compiled;

void compileRecursive (Node* n, QList<Processor*>& output)
{
  // TODO: Dynamic cast is bad. any better way to do this other
  // than moving process(), visit(), isVisited() etc to Node?
  Processor* p = dynamic_cast<Processor*>( n );
  if (p == NULL || !p->isVisited()) {
    if (p) { p->visit(); }
    foreach (Node* dep, n->dependencies()) {
      compileRecursive( dep, output );
    }
    if (p) { output.append( p ); }
  }
}
/*
  Alternate compilation method:
  create set of all nodes - these are "untraversed".
  remove from traversed, recurse into dependents, add to compiled-list
*/


void compile (QList<Processor*> input, QList<Processor*>& output) {

  // Mark everything as unvisited
  QListIterator<Processor*> i( input );
  while (i.hasNext()) {
    i.next()->unvisit();
  }

  // Process nodes that are pure-sinks first
  i.toFront();
  while (i.hasNext()) {
    Processor* p = i.next();
    if (p->dependents().count() == 0) {
      compileRecursive( p, output );
    }
  }

  // Then process everything else
  i.toFront();
  while (i.hasNext()) {
    Processor* p = i.next();
    compileRecursive( p, output );
  }
}


int main (int argc, char ** argv) {
    bool createGui = false;

    QCoreApplication * app = createGui ?
        new QApplication( argc, argv ) :
        new QCoreApplication( argc, argv );

    app->setApplicationName( "Unison" );
    app->setOrganizationDomain( "unison.sourceforge.net" );
	app->setOrganizationName( "Paul Giblock" );

    printLogo();
	// If running in CLI mode, print a disclaimer
	printDisclaimer();

	// JACK stuff
	jackClient = jack_client_open("Unison Studio", JackNullOption, NULL);
	if (!jackClient) { printf("Failed to connect to JACK."); }
	else { printf("Connected to JACK.\n"); }

        jackEngine = new JackEngine( jackClient );
	jack_set_process_callback(jackClient, &processCb, NULL);

        jackPorts[0] = jackEngine->registerPort("Master/out 1", Port::INPUT);
	jackPorts[1] = jackEngine->registerPort("Master/out 2", Port::INPUT);
	jackPorts[2] = jackEngine->registerPort("Master/in 1", Port::OUTPUT);
	jackPorts[3] = jackEngine->registerPort("Master/in 2", Port::OUTPUT);
	jackPorts[4] = jackEngine->registerPort("Channel/out 1", Port::INPUT);
	jackPorts[5] = jackEngine->registerPort("Channel/out 2", Port::INPUT);

	// Init
	PluginManager::initializeInstance();

	PluginManager * man = PluginManager::instance();

	std::cout << "Creating Plugins" << std::endl;
	processors.append(man->descriptor("http://plugin.org.uk/swh-plugins/vynil")
			->createPlugin(48000));
	processors.append(man->descriptor("http://plugin.org.uk/swh-plugins/lfoPhaser")
			->createPlugin(48000));

	std::cout << "Activating Plugins" << std::endl;
	foreach (Processor* n, processors) { n->activate(); }

	std::cout << "Referencing Ports" << std::endl;
	float controls[] = {0.5f, 0.5f, 0.5f, 0.5f};
	int f=0;
	foreach (Processor* n, processors) {
		int inCnt=0, outCnt =0;
		for (uint32_t i=0; i<n->portCount(); ++i) {
			Port* p = n->port(i);
			if (p->type() == Port::AUDIO) {
                          switch (p->direction()) {
                            case Port::INPUT:
                              fxin[f][inCnt++] = p;
                              break;
                            case Port::OUTPUT:
                              fxout[f][outCnt++] = p;
                              break;
                            default:
                              //TODO: Programming error!
                              break;
                          }
			}
			else if (p->type() == Port::CONTROL) {
				// MEMLEAK
				float * val = new float(0.0f);
				*val = p->maximum(); //p->defaultValue();
				p->connectToBuffer(val);
			}
		}
		f++;
	}

	std::cout << "Connecting Ports" << std::endl;
	fxout[0][0]->connect(fxin[1][0]);
	fxout[1][0]->connect(jackPorts[0]);
	fxout[0][1]->connect(jackPorts[1]);

	//jackPorts[4]->connect(fxout[1][0]); // To channel out
	//fxin[2][0]->connect(jackPorts[2]);  // To master In
	//jackPorts[0]->connect(fxout[2][0]); // To master out


	std::cout << "Compiling" << std::endl;
	QList<Processor*>* compiledSwap = new QList<Processor*>();
	compile( processors, *compiledSwap );
        compiledSwap = compiled.fetchAndStoreRelaxed( compiledSwap );
        delete compiledSwap;

	std::cout << "Processing Nodes" << std::endl;
	jack_activate(jackClient);
	char c;
	std::cin >> &c;

	std::cout << "Disconnecting JACK" << std::endl;
	jack_deactivate(jackClient);
	// TODO: Disconnect jack
	jack_client_close(jackClient);

	std::cout << "Deactivating Plugins" << std::endl;
	foreach (Processor * p, processors) { p->deactivate(); }
	delete compiled;

	std::cout << "Bye!" << std::endl;
	return 0;
}


int processCb (jack_nframes_t nframes, void* data)
{
	ProcessingContext context( nframes );

	float* inBuf = new float[nframes];
	for (nframes_t i=0; i<nframes; ++i) {
		inBuf[i] = 0.0f;
	}

	float* jackBuff0 = (float*)jack_port_get_buffer(jackPorts[0]->jackPort(), nframes);
	float* jackBuff1 = (float*)jack_port_get_buffer(jackPorts[1]->jackPort(), nframes);

        // Vynil input - silent
	fxin[0][0]->connectToBuffer(inBuf);
	fxin[0][1]->connectToBuffer(inBuf);

        // LFO input
        fxout[0][0]->connectToBuffer(jackBuff0);
        fxout[0][1]->connectToBuffer(jackBuff1);
        fxin[1][0]->connectToBuffer(jackBuff0);
        fxout[1][0]->connectToBuffer(jackBuff0);

	// Processing loop
	foreach (Processor * node, *compiled) {
		node->process(context);
	}

        /*
	// Process mixer
	memcpy( jack_port_get_buffer(masterOut[0], nframes),
			jack_port_get_buffer(masterIn[0], nframes),
			nframes * sizeof(float) );
	memcpy( jack_port_get_buffer(masterOut[1], nframes),
			jack_port_get_buffer(masterIn[1], nframes),
			nframes * sizeof(float) );
        */

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
