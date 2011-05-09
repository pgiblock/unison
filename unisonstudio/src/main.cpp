/*
 * main.cpp
 *
 * Copyright (c) 2010-2011 Paul Giblock <pgib/at/users.sourceforge.net>
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

#include "extensionsystem/ExtensionManager.hpp"
#include "extensionsystem/ExtensionInfo.hpp"
#include "extensionsystem/IExtension.hpp"
#include "raul/Configuration.hpp"
#include "raul/SharedPtr.hpp"

#include "ingen/EngineBase.hpp"
#include "ingen/ServerInterface.hpp"
#include "ingen/Resource.hpp"
#include "qingen/QObjectClientInterface.hpp"
#include "qingen/World.hpp"

#include <QDir>
#include <QDebug>
#include <QSettings>

#include <QtNetwork/QNetworkProxyFactory>

#include <QApplication>

#include "BackgroundStuff.hpp"

//using namespace Unison;
using namespace std;
using namespace Raul;
using namespace Ingen;

QIngen::World* world = NULL;


void initializeQIngen();
void startQIngenDemo();

void ingen_interrupt(int)
{
  cout << "ingen: Interrupted" << endl;
  if (world->engine())
    world->engine()->quit();
  delete world;
  exit(EXIT_FAILURE);
}


void ingen_try(bool cond, const char* msg)
{
  if (!cond) {
    cerr << "ingen: Error: " << msg << endl;
    delete world;
    exit(EXIT_FAILURE);
  }
}


enum { OptionIndent = 2, DescriptionIndent = 24 };

static const char *CORE_EXTENSION_NAME = "Core";
static const char *HELP_OPTION         = "--help";
static const char *VERSION_OPTION      = "--version";


/** Draws an ascii UNISON logo */
void printLogo()
{
  QTextStream str(stdout);  str <<
      "   __  ___  ______________  _  __\n"
      "  / / / / |/ /  _/ __/ __ \\/ |/ /\n"
      " / /_/ /    // /_\\ \\/ /_/ /    / \n"
      " \\____/_/|_/___/___/\\____/_/|_/  \n\n";
}


/** Draws the GPL mandated disclaimer */
void printDisclaimer()
{
  QTextStream str(stdout);
  str <<
      "Unison version 0, Copyright (C) 2010 Paul R Giblock\n"
      "Unison comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n"
      "This is free software, and you are welcome to redistribute it\n"
      "under certain conditions; type `show c' for details.\n\n";
} 

static void printVersion (const ExtensionSystem::ExtensionManager& em)
{
  QTextStream str(stdout);
  str << '\n' << "Unison Studio version 0, using Qt " << qVersion() << "\n\n";
  em.formatExtensionVersions(str);
  str << '\n' << "Unison Studio copyright goes here" << '\n';
}


static void printHelp(const QString& a0, const ExtensionSystem::ExtensionManager& em)
{
  QTextStream str(stdout);
  str << "Usage: " << a0  << " <options...>\n";
  str << "  --help                Display this help\n";
  str << "  --version             Display the Unison Studio and extension versions\n";
  ExtensionSystem::ExtensionManager::formatOptions(str, OptionIndent, DescriptionIndent);
  em.formatExtensionOptions(str,  OptionIndent, DescriptionIndent);
}


static inline QString msgCoreLoadFailure(const QString& why)
{
  return QCoreApplication::translate("Application", "Failed to load core: %1").arg(why);
}


static inline QStringList getExtensionPaths()
{
  // TODO: This needs to be improved to work with other Operating systems
  QStringList rc;
  // Figure out root:  Up one from 'bin'
  QDir rootDir = QApplication::applicationDirPath();
  rootDir.cdUp();
  const QString rootDirPath = rootDir.canonicalPath();

  // 1) system extensions
  QString extensionPath = rootDirPath;
  extensionPath += QLatin1Char('/');
  extensionPath += QLatin1String("lib");
  extensionPath += QLatin1Char('/');
  extensionPath += QLatin1String("unison");
  extensionPath += QLatin1Char('/');
  extensionPath += QLatin1String("extensions");
  rc.push_back(extensionPath);

  // 2) additional search paths??
  // ...
  return rc;
}


int main (int argc, char** argv)
{
  bool createGui = false;
  printLogo();

  QCoreApplication* appPtr;
  if (createGui) {
    appPtr = new QApplication(argc, argv);
  }
  else {
    printDisclaimer();
    appPtr = new QCoreApplication(argc, argv);
  }
  QScopedPointer<QCoreApplication> app(appPtr);
  appPtr = NULL;

  app->setApplicationName("Unison");
  app->setOrganizationDomain("unison.sourceforge.net");
  app->setOrganizationName("Paul Giblock");

  //QTranslator translator;
  //QTranslator qtTranslator;
  QString locale = QLocale::system().name();

  // Must be done before any QSettings class is created
  QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope,
                     QCoreApplication::applicationDirPath() + QLatin1String("/../share/unison"));

  // Work around bug in QSettings which gets triggered on Windows & Mac only
#ifdef Q_OS_MAC
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                     QDir::homePath()+"/.config");
#endif
#ifdef Q_OS_WIN
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                     qgetenv("appdata"));
#endif

  // keep this in sync with the MainWindow ctor in coreplugin/mainwindow.cpp
  const QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                           QLatin1String("Unison"), QLatin1String("UnisonStudio"));

  locale = settings.value("General/OverrideLanguage", locale).toString();

  // Make sure we honor the system's proxy settings
  QNetworkProxyFactory::setUseSystemConfiguration(true);


  // Init extension system
  ExtensionSystem::ExtensionManager extensionManager;
  extensionManager.setFileExtension(QLatin1String("extinfo"));

  const QStringList extensionPaths = getExtensionPaths();
  extensionManager.setExtensionPaths(extensionPaths);

  // Parse arguments
  const QStringList arguments = QCoreApplication::arguments();
  QMap<QString, QString> foundAppOptions;
  if (arguments.size() > 1) {
    QMap<QString, bool> appOptions;
    appOptions.insert(QLatin1String(HELP_OPTION), false);
    appOptions.insert(QLatin1String(VERSION_OPTION), false);
    QString errorMessage;
    bool parseOk = extensionManager.parseOptions(
        arguments, appOptions, &foundAppOptions, &errorMessage);

    if (!parseOk) {
      qWarning() <<errorMessage;
      printHelp(QFileInfo(QApplication::applicationFilePath()).baseName(),
                extensionManager);
      return -1;
    }
  }

  // Handle non-extension-based options
  if (foundAppOptions.contains(QLatin1String(VERSION_OPTION))) {
    printVersion(extensionManager);
    return 0;
  }
  if (foundAppOptions.contains(QLatin1String(HELP_OPTION))) {
    printHelp(QFileInfo(app->applicationFilePath()).baseName(), extensionManager);
    return 0;
  }

  // Loop through discovered extensions, find core extension
  const QList<ExtensionSystem::ExtensionInfo*> extensions = extensionManager.extensions();
  ExtensionSystem::ExtensionInfo* coreextension = 0;
  foreach (ExtensionSystem::ExtensionInfo* info, extensions) {
    if (info->name() == QLatin1String(CORE_EXTENSION_NAME)) {
      coreextension = info;
      break;
    }
  }
  if (!coreextension) {
    QString nativePaths = QDir::toNativeSeparators(extensionPaths.join(QLatin1String(",")));
    const QString reason = QCoreApplication::translate("Application",
                                                       "Could not find 'Core.extinfo' in %1").arg(nativePaths);
    qWarning() << msgCoreLoadFailure(reason);
    return 1;
  }
  if (coreextension->hasError()) {
    qWarning() << msgCoreLoadFailure(coreextension->errorString());
    return 1;
  }

  // Load extensions, displaying any errors that occur
  extensionManager.loadExtensions();
  if (coreextension->hasError()) {
    qWarning() << msgCoreLoadFailure(coreextension->errorString());
    return 1;
  }
  {
    QStringList errors;
    foreach (ExtensionSystem::ExtensionInfo* p, extensionManager.extensions()) {
      if (p->hasError()) {
        errors.append(p->errorString());
      }
    }
    if (!errors.isEmpty()) {
      qWarning() << QCoreApplication::translate("Application",
                      "Unison Studio - Extension loader messages");
      qWarning() << errors.join(QString::fromLatin1("\n\n"));
    }
  }

  initializeQIngen();
  startQIngenDemo();

  // Do this after the event loop has started
  return app->exec();

}


void initializeQIngen()
{
  // FIXME: Don't hardcode path like a noob
  QIngen::World::setBundlePath("/usr/local/bin");
  world = new QIngen::World();

  ingen_try(world->loadModule("server"),
            "Unable to load server module");

  ingen_try(world->engine(),
            "Unable to create engine");

  // Activate the engine, if we have one
  if (world->engine()) {
    ingen_try(world->loadModule("jack"),
              "Unable to load jack module");
  }

  // Activate
  world->engine()->activate();
}


void startQIngenDemo()
{
  using namespace std;

  SharedPtr<ServerInterface> server = world->server();

  // Is this even needed?
  server->get("ingen:plugins"); // TODO: QIngen::Server or QIngen::ServerInterface

  BackgroundStuff* bkgrnd = new BackgroundStuff(qApp, world);

  QIngen::QObjectClientInterface client;
  server->register_client(&client);

  server->bundle_begin();

  Resource::Properties props;
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#AudioPort")));
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#InputPort")));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#name",
                         "Fun Port"));
  server->put("path:/fun_port", props);

  props = Resource::Properties();
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#AudioPort")));
  props.insert(make_pair("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                         Atom(Atom::URI, "http://lv2plug.in/ns/lv2core#OutputPort")));
  props.insert(make_pair("http://lv2plug.in/ns/lv2core#name",
                         "Pain Port"));
  server->put("path:/pain_port", props);

  server->connect("path:/fun_port", "path:/pain_port");

  server->bundle_end();
}


// vim: tw=90 ts=8 sw=2 sts=4 et ci pi cin cino=l1,g0,+2s,\:0,(0,u0,U0,W2s
