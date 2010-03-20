#include <iostream>

#include <QtGui/QApplication>
#include "unison/PluginManager.h"
#include "unison/ProcessingContext.h"

void printLogo();
void printDisclaimer();

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


	// Init
	using namespace Unison;
	PluginManager::initializeInstance();

	PluginManager * man = PluginManager::instance();

	std::cout << "Creating Plugins" << std::endl;
	PluginPtr fx[2];
	fx[0] = man->descriptor("http://plugin.org.uk/swh-plugins/analogueOsc")->createPlugin(48000);
	fx[1] = man->descriptor("http://plugin.org.uk/swh-plugins/lfoPhaser")->createPlugin(48000);

	std::cout << "Activating Plugins" << std::endl;
	for (int i=0; i<2; ++i)	{ fx[i]->activate(); }

	std::cout << "Deactivating Plugins" << std::endl;
	for (int i=0; i<2; ++i) { fx[i]->deactivate(); }

	std::cout << "Connecting Ports" << std::endl;
	Port* fx0out = fx[0]->port(0);
	Port* fx1in = NULL, * fx1out = NULL;

	for (uint32_t i=0; i<fx[1]->portCount(); ++i) {
		Port* p = fx[1]->port(i);
		if (p->isInput())       { fx1in = p;  }
		else if (p->isOutput()) { fx1out = p; }
	}

	std::cout << "Processing Nodes" << std::endl;
	ProcessingContext context;
	fx[0]->activate();
	fx[0]->process(context);
	fx[0]->deactivate();

    return app->exec();
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
