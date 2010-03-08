#include <iostream>

#include <QtGui/QApplication>
#include "unison/PluginManager.h"

void printLogo();
void printDisclaimer();

int main (int argc, char ** argv) {
    bool createGui = false;

    QCoreApplication * app = createGui ?
        new QApplication( argc, argv ) :
        new QCoreApplication( argc, argv );

    app->setApplicationName( "Unison" );
    app->setOrganizationDomain( "unison.sourceforge.net" );
	app->setOrganizationName( "The Unison Team" );

    printLogo();
	// If running in CLI mode, print a disclaimer
	printDisclaimer();


	// Init
	PluginManager::initializeInstance();

	PluginManager * man = PluginManager::instance();
	PluginPtr plugin = man->descriptor("http://plugin.org.uk/swh-plugins/xfade")->createPlugin(48000);

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
