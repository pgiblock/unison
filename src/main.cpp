#include <iostream>

#include <QtGui/QApplication>
#include <QSet>
#include <jack/jack.h>
#include "unison/PluginManager.h"
#include "unison/ProcessingContext.h"

using namespace Unison;

void printLogo();
void printDisclaimer();
int processCb(jack_nframes_t nframes, void* data);

// Engine stuff

jack_client_t * jackClient;

class JackPort;
QList<JackPort*> jackPorts;
Port* fxin[2][2];
Port* fxout[2][2];
QList<Node*> nodes;
QAtomicPointer<QList<Node*>> compiled;

class JackPort : public Port
{
public:
	JackPort(jack_port_t * port) :
		Port(),
		m_port(port)
	{
	}

	QString name (size_t maxLength) const {
		return jack_port_short_name( m_port );
	}

	QString fullName () const {
		return jack_port_name( m_port );
	}

	bool isInput () const {
		// TODO Might be swapped
		return jack_port_flags( m_port ) & JackPortIsInput;
	}

	bool isOutput () const {
		// TODO Might be swapped
		return jack_port_flags( m_port ) & JackPortIsOutput;
	}

	bool isSink () const {
		return true; // TODO!
	}

	Type type () const {
		return Port::AUDIO; // TODO!
	}

	void connectToBuffer (float * buf) {
		// TODO
	}
	float value () const {
		return 0.0f;
	}
	void setValue (float value) {
	}
	float defaultValue () const {
		return 0.0f;
	}
	bool isBounded () const {
		return false;
	}
	float minimum () const {
		return 0.0f;
	}
	float maximum () const {
		return 0.0f;
	}
	bool isToggled () const {
		return false;
	}

	const QSet<Node*> providers () const {
		QSet<Node*> providers;

		// TODO: This really sucks. rethink the orientation and duty of JackPort
		if (isOutput()) {
			const char** name = jack_port_get_connections( m_port );
			// Within all connected ports
			while (name != NULL) {
				// Look for (input) ports belonging to us
				foreach (JackPort* jp, jackPorts) {
					if (jp->fullName() == *name) {
						// Then, get connections to the jack ports
						foreach (Port* p, jp->connectedPorts()) {
							// And add providers for those ports
							providers += p->providers();
						}
					}
				}
				++name;
			}
		}
		else if (isInput()) {
		}
		return providers;
	}

private:
	jack_port_t* m_port;
};

/*
void compileRecursive (Node* n, QList<Node*> output) {
	for (uint32_t i = 0; i < n->portCount(); ++i) {
		foreach (Node * provider, n->port(i)->providers()) {
			compileRecursive( provider, output );
		}
	}
	output.append( n );
}*/

void compileRecursive (Node* n, QList<Node*>& output) {
	for (uint32_t i = 0; i < n->portCount(); ++i) {
		Port * port = n->port(i);
		// TODO: These Connections are going the wrong damn way!!!!!!
		// They should be providers, not dependents!
		foreach (Port * connectedPort, port->connectedPorts()) {
			foreach (Node * provider, connectedPort->providers()) {
				compileRecursive( provider, output );
			}
		}
	}
	output.append( n );
}


void compile (Node* n, QList<Node*>& output) {
	compileRecursive( n, output );
	output.append( n );
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
	jack_client_t* jackClient =
		jack_client_open("Unison Studio", JackNullOption, NULL);

	if (!jackClient)
		printf("Failed to connect to JACK.");
	else
		printf("Connected to JACK.\n");

	jack_set_process_callback(jackClient, &processCb, NULL);

	jackPorts.append(new JackPort(jack_port_register(jackClient, "Master/out 1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)));
	jackPorts.append(new JackPort(jack_port_register(jackClient, "Master/out 2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)));
	jackPorts.append(new JackPort(jack_port_register(jackClient, "Master/in 1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0)));
	jackPorts.append(new JackPort(jack_port_register(jackClient, "Master/in 2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0)));
	jackPorts.append(new JackPort(jack_port_register(jackClient, "Channel/out 1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)));
	jackPorts.append(new JackPort(jack_port_register(jackClient, "Channel/out 2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)));

	// Init
	PluginManager::initializeInstance();

	PluginManager * man = PluginManager::instance();

	std::cout << "Creating Plugins" << std::endl;
	nodes.append(man->descriptor("http://plugin.org.uk/swh-plugins/vynil")
			->createPlugin(48000));
	nodes.append(man->descriptor("http://plugin.org.uk/swh-plugins/lfoPhaser")
			->createPlugin(48000));

	std::cout << "Activating Plugins" << std::endl;
	foreach (Node* n, nodes) { n->activate(); }

	std::cout << "Connecting Ports" << std::endl;
	float controls[] = {0.5f, 0.5f, 0.5f, 0.5f};
	int f=0;
	foreach (Node* n, nodes) {
		int inCnt=0, outCnt =0;
		for (uint32_t i=0; i<n->portCount(); ++i) {
			Port* p = n->port(i);
			if (p->type() == Port::AUDIO) {
				if (p->isInput())       { fxin[f][inCnt++] = p;  }
				else if (p->isOutput()) { fxout[f][outCnt++] = p; }
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

	// fx[0] -> fx[1] -> jackPorts[0]

	fxin[1][0]->connectTo(fxout[0][0]);
	jackPorts[0]->connectTo(fxout[1][0]);

	compiled = new QList<Node*>;
	compileRecursive(nodes[1], compiled);

	printf("%d tasks\n", compiled.count());

	return 0;
/*
// todo:
list<Node> nodes;
list<Connection> connections;

node.

	graph.connect(fxOut[0][0], fxIn[1][0]);
	graph.connect(fxOut[1][0], jackPort[0]);
	graph.connect(fxOut[1][1], jackPort[1]);
*/

	std::cout << "Processing Nodes" << std::endl;
	jack_activate(jackClient);
	char c;
	std::cin >> &c;

	std::cout << "Disconnecting JACK" << std::endl;
	jack_deactivate(jackClient);
	// TODO: Disconnect jack
	jack_client_close(jackClient);

	std::cout << "Deactivating Plugins" << std::endl;
	foreach (Node * n, nodes) { n->deactivate(); }

	delete compiled;
	return 0;
}


int processCb (jack_nframes_t nframes, void* data)
{
	ProcessingContext context( nframes );

	/*
	float* inBuf = new float[nframes];
	for (nframes_t i=0; i<nframes; ++i) {
		inBuf[i] = 0.0f;
	}

	// Processing loop
	foreach (Node * node, compiled) {
		node->process(context);
	}

	fxin[0][0]->connectToBuffer(inBuf);
	fxin[0][1]->connectToBuffer(inBuf);
	fxout[0][0]->connectToBuffer((float*)jack_port_get_buffer(chanOut[0], nframes));
	fxout[0][1]->connectToBuffer((float*)jack_port_get_buffer(chanOut[1], nframes));

	printf("%lld  ->  %lld\n",
		jack_port_get_buffer(chanOut[0], nframes),
		jack_port_get_buffer(masterIn[0], nframes));
	// Copies
	//fxin[1][0]->connectToBuffer(jackBuf0);
	//fxout[1][0]->connectToBuffer(jackBuf0);

	//fxin[1][0]->connectToBuffer((float*)jack_port_get_buffer(masterIn[0], nframes));
	//fxout[1][0]->connectToBuffer((float*)jack_port_get_buffer(masterOut[0], nframes));

	// Process channel

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
