#include <iostream>

#include <QtGui/QApplication>

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
