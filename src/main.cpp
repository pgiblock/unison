#include <iostream>

#include <QtGui/QApplication>
#include "unison/Lv2Manager.h"

void printLogo();

int main (int argc, char ** argv) {
    bool createGui = false;

    QCoreApplication * app = createGui ?
        new QApplication( argc, argv ) :
        new QCoreApplication( argc, argv );

    app->setApplicationName( "Unison" );
    app->setOrganizationDomain( "unison.sourceforge.net" );
	app->setOrganizationName( "The Unison Team" );

    printLogo();


	// Init
	Lv2Manager::initializeInstance();


	Lv2Manager * man = Lv2Manager::instance();
	man->saveToCacheFile();

	Lv2PluginPtr plugin = man->getDescriptor("http://plugin.org.uk/swh-plugins/xfade")->createPlugin(48000);

    return app->exec();
}



/**
 * Draws an ascii UNISON logo
 */
void printLogo() {
    std::cout <<
        "   __  ___  ______________  _  __\n"
        "  / / / / |/ /  _/ __/ __ \\/ |/ /\n"
        " / /_/ /    // /_\\ \\/ /_/ /    / \n"
        " \\____/_/|_/___/___/\\____/_/|_/  \n\n";
}
