/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "PluginInfo.h"

#include "PluginInfo_p.h"
#include "IPlugin.h"
#include "IPlugin_p.h"
#include "PluginManager.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QRegExp>
#include <QtCore/QCoreApplication>
#include <QtDebug>

#ifdef Q_OS_LINUX
// Using the patched version breaks on Fedora 10, KDE4.2.2/Qt4.5.
#   define USE_UNPATCHED_QPLUGINLOADER 1
#else
#   define USE_UNPATCHED_QPLUGINLOADER 1
#endif

#if USE_UNPATCHED_QPLUGINLOADER

#   include <QtCore/QPluginLoader>
    typedef QT_PREPEND_NAMESPACE(QPluginLoader) PluginLoader;

#else

#   include "patchedpluginloader.cpp"
    typedef PatchedPluginLoader PluginLoader;

#endif

/*!
    \class ExtensionSystem::PluginDependency
    \brief Struct that contains the name and required compatible version number of a plugin's dependency.

    This reflects the data of a dependency tag in the plugin's xml description file.
    The name and version are used to resolve the dependency, i.e. a plugin with the given name and
    plugin \c {compatibility version <= dependency version <= plugin version} is searched for.

    See also ExtensionSystem::IPlugin for more information about plugin dependencies and
    version matching.
*/

/*!
    \variable ExtensionSystem::PluginDependency::name
    String identifier of the plugin.
*/

/*!
    \variable ExtensionSystem::PluginDependency::version
    Version string that a plugin must match to fill this dependency.
*/

/*!
    \class ExtensionSystem::PluginInfo
    \brief Contains the information of the plugins xml description file and
    information about the plugin's current state.

    The plugin info is also filled with more information as the plugin
    goes through its loading process (see PluginInfo::State).
    If an error occurs, the plugin info is the place to look for the
    error details.
*/

/*!
    \enum ExtensionSystem::PluginInfo::State

    The plugin goes through several steps while being loaded.
    The state gives a hint on what went wrong in case of an error.

    \value  Invalid
            Starting point: Even the xml description file was not read.
    \value  Read
            The xml description file has been successfully read, and its
            information is available via the PluginInfo.
    \value  Resolved
            The dependencies given in the description file have been
            successfully found, and are available via the dependencyInfos() method.
    \value  Loaded
            The plugin's library is loaded and the plugin instance created
            (available through plugin()).
    \value  Initialized
            The plugin instance's IPlugin::initialize() method has been called
            and returned a success value.
    \value  Running
            The plugin's dependencies are successfully initialized and
            extensionsInitialized has been called. The loading process is
            complete.
    \value Stopped
            The plugin has been shut down, i.e. the plugin's IPlugin::shutdown() method has been called.
    \value Deleted
            The plugin instance has been deleted.
*/
using namespace ExtensionSystem;
using namespace ExtensionSystem::Internal;

/*!
    \fn bool PluginDependency::operator==(const PluginDependency &other)
    \internal
*/
bool PluginDependency::operator==(const PluginDependency &other)
{
    return name == other.name && version == other.version;
}

/*!
    \fn PluginInfo::PluginInfo()
    \internal
*/
PluginInfo::PluginInfo()
    : d(new PluginInfoPrivate(this))
{
}

/*!
    \fn PluginInfo::~PluginInfo()
    \internal
*/
PluginInfo::~PluginInfo()
{
    delete d;
    d = 0;
}

/*!
    \fn QString PluginInfo::name() const
    The plugin name. This is valid after the PluginInfo::Read state is reached.
*/
QString PluginInfo::name() const
{
    return d->name;
}

/*!
    \fn QString PluginInfo::version() const
    The plugin version. This is valid after the PluginInfo::Read state is reached.
*/
QString PluginInfo::version() const
{
    return d->version;
}

/*!
    \fn QString PluginInfo::compatVersion() const
    The plugin compatibility version. This is valid after the PluginInfo::Read state is reached.
*/
QString PluginInfo::compatVersion() const
{
    return d->compatVersion;
}

/*!
    \fn QString PluginInfo::vendor() const
    The plugin vendor. This is valid after the PluginInfo::Read state is reached.
*/
QString PluginInfo::vendor() const
{
    return d->vendor;
}

/*!
    \fn QString PluginInfo::copyright() const
    The plugin copyright. This is valid after the PluginInfo::Read state is reached.
*/
QString PluginInfo::copyright() const
{
    return d->copyright;
}

/*!
    \fn QString PluginInfo::license() const
    The plugin license. This is valid after the PluginInfo::Read state is reached.
*/
QString PluginInfo::license() const
{
    return d->license;
}

/*!
    \fn QString PluginInfo::description() const
    The plugin description. This is valid after the PluginInfo::Read state is reached.
*/
QString PluginInfo::description() const
{
    return d->description;
}

/*!
    \fn QString PluginInfo::url() const
    The plugin url where you can find more information about the plugin. This is valid after the PluginInfo::Read state is reached.
*/
QString PluginInfo::url() const
{
    return d->url;
}

/*!
    \fn QString PluginInfo::category() const
    The category that the plugin belongs to. Categories are groups of plugins which allow for keeping them together in the UI.
    Returns an empty string if the plugin does not belong to a category.
*/
QString PluginInfo::category() const
{
    return d->category;
}

/*!
    \fn bool PluginInfo::isExperimental() const
    Returns if the plugin has its experimental flag set.
*/
bool PluginInfo::isExperimental() const
{
    return d->experimental;
}

/*!
    \fn bool PluginInfo::isEnabled() const
    Returns if the plugin is loaded at startup. True by default - the user can change it from the Plugin settings.
*/
bool PluginInfo::isEnabled() const
{
    return d->enabled;
}

bool PluginInfo::isDisabledByDependency() const
{
    return d->disabledByDependency;
}

/*!
    \fn QList<PluginDependency> PluginInfo::dependencies() const
    The plugin dependencies. This is valid after the PluginInfo::Read state is reached.
*/
QList<PluginDependency> PluginInfo::dependencies() const
{
    return d->dependencies;
}

/*!
    \fn PluginInfo::PluginArgumentDescriptions PluginInfo::argumentDescriptions() const
    Returns a list of descriptions of command line arguments the plugin processes.
*/

PluginInfo::PluginArgumentDescriptions PluginInfo::argumentDescriptions() const
{
    return d->argumentDescriptions;
}

/*!
    \fn QString PluginInfo::location() const
    The absolute path to the directory containing the plugin xml description file
    this PluginInfo corresponds to.
*/
QString PluginInfo::location() const
{
    return d->location;
}

/*!
    \fn QString PluginInfo::filePath() const
    The absolute path to the plugin xml description file (including the file name)
    this PluginInfo corresponds to.
*/
QString PluginInfo::filePath() const
{
    return d->filePath;
}

/*!
    \fn QStringList PluginInfo::arguments() const
    Command line arguments specific to that plugin. Set at startup
*/

QStringList PluginInfo::arguments() const
{
    return d->arguments;
}

/*!
    \fn void PluginInfo::setArguments(const QStringList &arguments)
    Set the command line arguments specific to that plugin to \a arguments.
*/

void PluginInfo::setArguments(const QStringList &arguments)
{
    d->arguments = arguments;
}

/*!
    \fn PluginInfo::addArgument(const QString &argument)
    Adds \a argument to the command line arguments specific to that plugin.
*/

void PluginInfo::addArgument(const QString &argument)
{
    d->arguments.push_back(argument);
}


/*!
    \fn PluginInfo::State PluginInfo::state() const
    The state in which the plugin currently is.
    See the description of the PluginInfo::State enum for details.
*/
PluginInfo::State PluginInfo::state() const
{
    return d->state;
}

/*!
    \fn bool PluginInfo::hasError() const
    Returns whether an error occurred while reading/starting the plugin.
*/
bool PluginInfo::hasError() const
{
    return d->hasError;
}

/*!
    \fn QString PluginInfo::errorString() const
    Detailed, possibly multi-line, error description in case of an error.
*/
QString PluginInfo::errorString() const
{
    return d->errorString;
}

/*!
    \fn bool PluginInfo::provides(const QString &pluginName, const QString &version) const
    Returns if this plugin can be used to fill in a dependency of the given
    \a pluginName and \a version.

        \sa PluginInfo::dependencies()
*/
bool PluginInfo::provides(const QString &pluginName, const QString &version) const
{
    return d->provides(pluginName, version);
}

/*!
    \fn IPlugin *PluginInfo::plugin() const
    The corresponding IPlugin instance, if the plugin library has already been successfully loaded,
    i.e. the PluginInfo::Loaded state is reached.
*/
IPlugin *PluginInfo::plugin() const
{
    return d->plugin;
}

/*!
    \fn QList<PluginInfo *> PluginInfo::dependencyInfos() const
    Returns the list of dependencies, already resolved to existing plugin infos.
    Valid if PluginInfo::Resolved state is reached.

    \sa PluginInfo::dependencies()
*/
QList<PluginInfo *> PluginInfo::dependencyInfos() const
{
    return d->dependencyInfos;
}

/*!
    \fn QList<PluginInfo *> PluginInfo::providesInfos() const
    Returns the list of plugins that depend on this one.

    \sa PluginInfo::dependencyInfos()
*/
QList<PluginInfo *> PluginInfo::providesInfos() const
{
    return d->providesInfos;
}

/*!
    \fn void PluginInfo::addDependentPlugin(PluginInfo *dependent)
    Adds a dependent the list of plugins that depend on this one.

    \sa PluginInfo::providesInfos()
*/
void PluginInfo::addDependentPlugin(PluginInfo *dependent)
{
    d->providesInfos.append(dependent);
}

void PluginInfo::removeDependentPlugin(PluginInfo *dependent)
{
    d->providesInfos.removeOne(dependent);
}

//==========PluginInfoPrivate==================

namespace {
    const char * const PLUGIN = "plugin";
    const char * const PLUGIN_NAME = "name";
    const char * const PLUGIN_VERSION = "version";
    const char * const PLUGIN_COMPATVERSION = "compatVersion";
    const char * const PLUGIN_EXPERIMENTAL = "experimental";
    const char * const VENDOR = "vendor";
    const char * const COPYRIGHT = "copyright";
    const char * const LICENSE = "license";
    const char * const DESCRIPTION = "description";
    const char * const URL = "url";
    const char * const CATEGORY = "category";
    const char * const DEPENDENCYLIST = "dependencyList";
    const char * const DEPENDENCY = "dependency";
    const char * const DEPENDENCY_NAME = "name";
    const char * const DEPENDENCY_VERSION = "version";
    const char * const ARGUMENTLIST = "argumentList";
    const char * const ARGUMENT = "argument";
    const char * const ARGUMENT_NAME = "name";
    const char * const ARGUMENT_PARAMETER = "parameter";
}
/*!
    \fn PluginInfoPrivate::PluginInfoPrivate(PluginInfo *info)
    \internal
*/
PluginInfoPrivate::PluginInfoPrivate(PluginInfo *info)
    :
    enabled(true),
    disabledByDependency(false),
    plugin(0),
    state(PluginInfo::Invalid),
    hasError(false),
    q(info)
{
}

/*!
    \fn bool PluginInfoPrivate::read(const QString &fileName)
    \internal
*/
bool PluginInfoPrivate::read(const QString &fileName)
{
    name
        = version
        = compatVersion
        = vendor
        = copyright
        = license
        = description
        = url
        = category
        = location
        = "";
    state = PluginInfo::Invalid;
    hasError = false;
    errorString = "";
    dependencies.clear();
    QFile file(fileName);
    if (!file.exists())
        return reportError(tr("File does not exist: %1").arg(file.fileName()));
    if (!file.open(QIODevice::ReadOnly))
        return reportError(tr("Could not open file for read: %1").arg(file.fileName()));
    QFileInfo fileInfo(file);
    location = fileInfo.absolutePath();
    filePath = fileInfo.absoluteFilePath();
    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        reader.readNext();
        switch (reader.tokenType()) {
        case QXmlStreamReader::StartElement:
            readPluginInfo(reader);
            break;
        default:
            break;
        }
    }
    if (reader.hasError())
        return reportError(tr("Error parsing file %1: %2, at line %3, column %4")
                .arg(file.fileName())
                .arg(reader.errorString())
                .arg(reader.lineNumber())
                .arg(reader.columnNumber()));
    state = PluginInfo::Read;
    return true;
}

void PluginInfo::setEnabled(bool value)
{
    d->enabled = value;
}

/*!
    \fn bool PluginInfoPrivate::reportError(const QString &err)
    \internal
*/
bool PluginInfoPrivate::reportError(const QString &err)
{
    errorString = err;
    hasError = true;
    return false;
}

static inline QString msgAttributeMissing(const char *elt, const char *attribute)
{
    return QCoreApplication::translate("PluginInfo", "'%1' misses attribute '%2'").arg(QLatin1String(elt), QLatin1String(attribute));
}

static inline QString msgInvalidFormat(const char *content)
{
    return QCoreApplication::translate("PluginInfo", "'%1' has invalid format").arg(content);
}

static inline QString msgInvalidElement(const QString &name)
{
    return QCoreApplication::translate("PluginInfo", "Invalid element '%1'").arg(name);
}

static inline QString msgUnexpectedClosing(const QString &name)
{
    return QCoreApplication::translate("PluginInfo", "Unexpected closing element '%1'").arg(name);
}

static inline QString msgUnexpectedToken()
{
    return QCoreApplication::translate("PluginInfo", "Unexpected token");
}

/*!
    \fn void PluginInfoPrivate::readPluginInfo(QXmlStreamReader &reader)
    \internal
*/
void PluginInfoPrivate::readPluginInfo(QXmlStreamReader &reader)
{
    QString element = reader.name().toString();
    if (element != QString(PLUGIN)) {
        reader.raiseError(QCoreApplication::translate("PluginInfo", "Expected element '%1' as top level element").arg(PLUGIN));
        return;
    }
    name = reader.attributes().value(PLUGIN_NAME).toString();
    if (name.isEmpty()) {
        reader.raiseError(msgAttributeMissing(PLUGIN, PLUGIN_NAME));
        return;
    }
    version = reader.attributes().value(PLUGIN_VERSION).toString();
    if (version.isEmpty()) {
        reader.raiseError(msgAttributeMissing(PLUGIN, PLUGIN_VERSION));
        return;
    }
    if (!isValidVersion(version)) {
        reader.raiseError(msgInvalidFormat(PLUGIN_VERSION));
        return;
    }
    compatVersion = reader.attributes().value(PLUGIN_COMPATVERSION).toString();
    if (!compatVersion.isEmpty() && !isValidVersion(compatVersion)) {
        reader.raiseError(msgInvalidFormat(PLUGIN_COMPATVERSION));
        return;
    } else if (compatVersion.isEmpty()) {
        compatVersion = version;
    }
    QString experimentalString = reader.attributes().value(PLUGIN_EXPERIMENTAL).toString();
    experimental = (experimentalString.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
    if (!experimentalString.isEmpty() && !experimental
            && experimentalString.compare(QLatin1String("false"), Qt::CaseInsensitive) != 0) {
        reader.raiseError(msgInvalidFormat(PLUGIN_EXPERIMENTAL));
        return;
    }
    enabled = !experimental;
    while (!reader.atEnd()) {
        reader.readNext();
        switch (reader.tokenType()) {
        case QXmlStreamReader::StartElement:
            element = reader.name().toString();
            if (element == VENDOR)
                vendor = reader.readElementText().trimmed();
            else if (element == COPYRIGHT)
                copyright = reader.readElementText().trimmed();
            else if (element == LICENSE)
                license = reader.readElementText().trimmed();
            else if (element == DESCRIPTION)
                description = reader.readElementText().trimmed();
            else if (element == URL)
                url = reader.readElementText().trimmed();
            else if (element == CATEGORY)
                category = reader.readElementText().trimmed();
            else if (element == DEPENDENCYLIST)
                readDependencies(reader);
            else if (element == ARGUMENTLIST)
                readArgumentDescriptions(reader);
            else
                reader.raiseError(msgInvalidElement(name));
            break;
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::EndElement:
        case QXmlStreamReader::Characters:
            break;
        default:
            reader.raiseError(msgUnexpectedToken());
            break;
        }
    }
}

/*!
    \fn void PluginInfoPrivate::readArgumentDescriptions(QXmlStreamReader &reader)
    \internal
*/

void PluginInfoPrivate::readArgumentDescriptions(QXmlStreamReader &reader)
{
    QString element;
    while (!reader.atEnd()) {
        reader.readNext();
        switch (reader.tokenType()) {
        case QXmlStreamReader::StartElement:
            element = reader.name().toString();
            if (element == ARGUMENT) {
                readArgumentDescription(reader);
            } else {
                reader.raiseError(msgInvalidElement(name));
            }
            break;
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::Characters:
            break;
        case QXmlStreamReader::EndElement:
            element = reader.name().toString();
            if (element == ARGUMENTLIST)
                return;
            reader.raiseError(msgUnexpectedClosing(element));
            break;
        default:
            reader.raiseError(msgUnexpectedToken());
            break;
        }
    }
}

/*!
    \fn void PluginInfoPrivate::readArgumentDescription(QXmlStreamReader &reader)
    \internal
*/
void PluginInfoPrivate::readArgumentDescription(QXmlStreamReader &reader)
{
    PluginArgumentDescription arg;
    arg.name = reader.attributes().value(ARGUMENT_NAME).toString();
    if (arg.name.isEmpty()) {
        reader.raiseError(msgAttributeMissing(ARGUMENT, ARGUMENT_NAME));
        return;
    }
    arg.parameter = reader.attributes().value(ARGUMENT_PARAMETER).toString();
    arg.description = reader.readElementText();
    if (reader.tokenType() != QXmlStreamReader::EndElement)
        reader.raiseError(msgUnexpectedToken());
    argumentDescriptions.push_back(arg);
}

/*!
    \fn void PluginInfoPrivate::readDependencies(QXmlStreamReader &reader)
    \internal
*/
void PluginInfoPrivate::readDependencies(QXmlStreamReader &reader)
{
    QString element;
    while (!reader.atEnd()) {
        reader.readNext();
        switch (reader.tokenType()) {
        case QXmlStreamReader::StartElement:
            element = reader.name().toString();
            if (element == DEPENDENCY) {
                readDependencyEntry(reader);
            } else {
                reader.raiseError(msgInvalidElement(name));
            }
            break;
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::Characters:
            break;
        case QXmlStreamReader::EndElement:
            element = reader.name().toString();
            if (element == DEPENDENCYLIST)
                return;
            reader.raiseError(msgUnexpectedClosing(element));
            break;
        default:
            reader.raiseError(msgUnexpectedToken());
            break;
        }
    }
}

/*!
    \fn void PluginInfoPrivate::readDependencyEntry(QXmlStreamReader &reader)
    \internal
*/
void PluginInfoPrivate::readDependencyEntry(QXmlStreamReader &reader)
{
    PluginDependency dep;
    dep.name = reader.attributes().value(DEPENDENCY_NAME).toString();
    if (dep.name.isEmpty()) {
        reader.raiseError(msgAttributeMissing(DEPENDENCY, DEPENDENCY_NAME));
        return;
    }
    dep.version = reader.attributes().value(DEPENDENCY_VERSION).toString();
    if (!dep.version.isEmpty() && !isValidVersion(dep.version)) {
        reader.raiseError(msgInvalidFormat(DEPENDENCY_VERSION));
        return;
    }
    dependencies.append(dep);
    reader.readNext();
    if (reader.tokenType() != QXmlStreamReader::EndElement)
        reader.raiseError(msgUnexpectedToken());
}

/*!
    \fn bool PluginInfoPrivate::provides(const QString &pluginName, const QString &pluginVersion) const
    \internal
*/
bool PluginInfoPrivate::provides(const QString &pluginName, const QString &pluginVersion) const
{
    if (QString::compare(pluginName, name, Qt::CaseInsensitive) != 0)
        return false;
    return (versionCompare(version, pluginVersion) >= 0) && (versionCompare(compatVersion, pluginVersion) <= 0);
}

/*!
    \fn QRegExp &PluginInfoPrivate::versionRegExp()
    \internal
*/
QRegExp &PluginInfoPrivate::versionRegExp()
{
    static QRegExp reg("([0-9]+)(?:[.]([0-9]+))?(?:[.]([0-9]+))?(?:_([0-9]+))?");
    return reg;
}
/*!
    \fn bool PluginInfoPrivate::isValidVersion(const QString &version)
    \internal
*/
bool PluginInfoPrivate::isValidVersion(const QString &version)
{
    return versionRegExp().exactMatch(version);
}

/*!
    \fn int PluginInfoPrivate::versionCompare(const QString &version1, const QString &version2)
    \internal
*/
int PluginInfoPrivate::versionCompare(const QString &version1, const QString &version2)
{
    QRegExp reg1 = versionRegExp();
    QRegExp reg2 = versionRegExp();
    if (!reg1.exactMatch(version1))
        return 0;
    if (!reg2.exactMatch(version2))
        return 0;
    int number1;
    int number2;
    for (int i = 0; i < 4; ++i) {
        number1 = reg1.cap(i+1).toInt();
        number2 = reg2.cap(i+1).toInt();
        if (number1 < number2)
            return -1;
        if (number1 > number2)
            return 1;
    }
    return 0;
}

/*!
    \fn bool PluginInfoPrivate::resolveDependencies(const QList<PluginInfo *> &infos)
    \internal
*/
bool PluginInfoPrivate::resolveDependencies(const QList<PluginInfo *> &infos)
{
    if (hasError)
        return false;
    if (state == PluginInfo::Resolved)
        state = PluginInfo::Read; // Go back, so we just re-resolve the dependencies.
    if (state != PluginInfo::Read) {
        errorString = QCoreApplication::translate("PluginInfo", "Resolving dependencies failed because state != Read");
        hasError = true;
        return false;
    }
    QList<PluginInfo *> resolvedDependencies;
    foreach (const PluginDependency &dependency, dependencies) {
        PluginInfo *found = 0;

        foreach (PluginInfo *info, infos) {
            if (info->provides(dependency.name, dependency.version)) {
                found = info;
                if (!info->isEnabled() || info->isDisabledByDependency())
                    disabledByDependency = true;

                info->addDependentPlugin(q);

                break;
            }
        }
        if (!found) {
            hasError = true;
            if (!errorString.isEmpty())
                errorString.append(QLatin1Char('\n'));
            errorString.append(QCoreApplication::translate("PluginInfo", "Could not resolve dependency '%1(%2)'")
                .arg(dependency.name).arg(dependency.version));
            continue;
        }
        resolvedDependencies.append(found);
    }
    if (hasError)
        return false;

    dependencyInfos = resolvedDependencies;

    if (enabled && !disabledByDependency)
        state = PluginInfo::Resolved;

    return true;
}

/*!
    \fn bool PluginInfoPrivate::loadLibrary()
    \internal
*/
bool PluginInfoPrivate::loadLibrary()
{
    if (hasError)
        return false;
    if (state != PluginInfo::Resolved) {
        if (state == PluginInfo::Loaded)
            return true;
        errorString = QCoreApplication::translate("PluginInfo", "Loading the library failed because state != Resolved");
        hasError = true;
        return false;
    }
#ifdef QT_NO_DEBUG

#ifdef Q_OS_WIN
    QString libName = QString("%1/%2.dll").arg(location).arg(name);
#elif defined(Q_OS_MAC)
    QString libName = QString("%1/lib%2.dylib").arg(location).arg(name);
#else
    QString libName = QString("%1/lib%2.so").arg(location).arg(name);
#endif

#else //Q_NO_DEBUG

#ifdef Q_OS_WIN
    QString libName = QString("%1/%2d.dll").arg(location).arg(name);
#elif defined(Q_OS_MAC)
    QString libName = QString("%1/lib%2_debug.dylib").arg(location).arg(name);
#else
    QString libName = QString("%1/lib%2.so").arg(location).arg(name);
#endif

#endif

    PluginLoader loader(libName);
    if (!loader.load()) {
        hasError = true;
        errorString = QDir::toNativeSeparators(libName)
            + QString::fromLatin1(": ") + loader.errorString();
        return false;
    }
    IPlugin *pluginObject = qobject_cast<IPlugin*>(loader.instance());
    if (!pluginObject) {
        hasError = true;
        errorString = QCoreApplication::translate("PluginInfo", "Plugin is not valid (does not derive from IPlugin)");
        loader.unload();
        return false;
    }
    state = PluginInfo::Loaded;
    plugin = pluginObject;
    plugin->d->pluginInfo = q;
    return true;
}

/*!
    \fn bool PluginInfoPrivate::initializePlugin()
    \internal
*/
bool PluginInfoPrivate::initializePlugin()
{
    if (hasError)
        return false;
    if (state != PluginInfo::Loaded) {
        if (state == PluginInfo::Initialized)
            return true;
        errorString = QCoreApplication::translate("PluginInfo", "Initializing the plugin failed because state != Loaded");
        hasError = true;
        return false;
    }
    if (!plugin) {
        errorString = QCoreApplication::translate("PluginInfo", "Internal error: have no plugin instance to initialize");
        hasError = true;
        return false;
    }
    QString err;
    if (!plugin->initialize(arguments, &err)) {
        errorString = QCoreApplication::translate("PluginInfo", "Plugin initialization failed: %1").arg(err);
        hasError = true;
        return false;
    }
    state = PluginInfo::Initialized;
    return true;
}

/*!
    \fn bool PluginInfoPrivate::initializeExtensions()
    \internal
*/
bool PluginInfoPrivate::initializeExtensions()
{
    if (hasError)
        return false;
    if (state != PluginInfo::Initialized) {
        if (state == PluginInfo::Running)
            return true;
        errorString = QCoreApplication::translate("PluginInfo", "Cannot perform extensionsInitialized because state != Initialized");
        hasError = true;
        return false;
    }
    if (!plugin) {
        errorString = QCoreApplication::translate("PluginInfo", "Internal error: have no plugin instance to perform extensionsInitialized");
        hasError = true;
        return false;
    }
    plugin->extensionsInitialized();
    state = PluginInfo::Running;
    return true;
}

/*!
    \fn bool PluginInfoPrivate::stop()
    \internal
*/
void PluginInfoPrivate::stop()
{
    if (!plugin)
        return;
    plugin->shutdown();
    state = PluginInfo::Stopped;
}

/*!
    \fn bool PluginInfoPrivate::kill()
    \internal
*/
void PluginInfoPrivate::kill()
{
    if (!plugin)
        return;
    delete plugin;
    plugin = 0;
    state = PluginInfo::Deleted;
}

// vim: ts=8 sw=2 sts=2 et sta noai
