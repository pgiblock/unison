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

#include "extensionsystem/ExtensionInfo.hpp"

#include "extensionsystem/IExtension.hpp"
#include "extensionsystem/ExtensionManager.hpp"
#include "ExtensionInfo_p.hpp"
#include "IExtension_p.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QRegExp>
#include <QCoreApplication>
#include <QtDebug>

#ifdef Q_OS_LINUX
// Using the patched version breaks on Fedora 10, KDE4.2.2/Qt4.5.
#   define USE_UNPATCHED_QEXTENSIONLOADER 1
#else
#   define USE_UNPATCHED_QEXTENSIONLOADER 1
#endif

#if USE_UNPATCHED_QEXTENSIONLOADER

#   include <QPluginLoader>
    typedef QT_PREPEND_NAMESPACE(QPluginLoader) ExtensionLoader;

#else

#   include "patchedextensionloader.cpp"
    typedef PatchedExtensionLoader ExtensionLoader;

#endif

/*!
    \class ExtensionSystem::ExtensionDependency
    \brief Struct that contains the name and required compatible version number of a extension's dependency.

    This reflects the data of a dependency tag in the extension's xml description file.
    The name and version are used to resolve the dependency, i.e. a extension with the given name and
    extension \c {compatibility version <= dependency version <= extension version} is searched for.

    See also ExtensionSystem::IExtension for more information about extension dependencies and
    version matching.
*/

/*!
    \variable ExtensionSystem::ExtensionDependency::name
    String identifier of the extension.
*/

/*!
    \variable ExtensionSystem::ExtensionDependency::version
    Version string that a extension must match to fill this dependency.
*/

/*!
    \class ExtensionSystem::ExtensionInfo
    \brief Contains the information of the extensions xml description file and
    information about the extension's current state.

    The extension info is also filled with more information as the extension
    goes through its loading process (see ExtensionInfo::State).
    If an error occurs, the extension info is the place to look for the
    error details.
*/

/*!
    \enum ExtensionSystem::ExtensionInfo::State

    The extension goes through several steps while being loaded.
    The state gives a hint on what went wrong in case of an error.

    \value  Invalid
            Starting point: Even the xml description file was not read.
    \value  Read
            The xml description file has been successfully read, and its
            information is available via the ExtensionInfo.
    \value  Resolved
            The dependencies given in the description file have been
            successfully found, and are available via the dependencyInfos() method.
    \value  Loaded
            The extension's library is loaded and the extension instance created
            (available through extension()).
    \value  Initialized
            The extension instance's IExtension::initialize() method has been called
            and returned a success value.
    \value  Running
            The extension's dependencies are successfully initialized and
            extensionsInitialized has been called. The loading process is
            complete.
    \value Stopped
            The extension has been shut down, i.e. the extension's IExtension::shutdown() method has been called.
    \value Deleted
            The extension instance has been deleted.
*/
using namespace ExtensionSystem;
using namespace ExtensionSystem::Internal;

/*!
    \fn bool ExtensionDependency::operator==(const ExtensionDependency &other)
    \internal
*/
bool ExtensionDependency::operator==(const ExtensionDependency &other)
{
    return name == other.name && version == other.version;
}

/*!
    \fn ExtensionInfo::ExtensionInfo()
    \internal
*/
ExtensionInfo::ExtensionInfo()
    : d(new ExtensionInfoPrivate(this))
{
}

/*!
    \fn ExtensionInfo::~ExtensionInfo()
    \internal
*/
ExtensionInfo::~ExtensionInfo()
{
    delete d;
    d = 0;
}

/*!
    \fn QString ExtensionInfo::name() const
    The extension name. This is valid after the ExtensionInfo::Read state is reached.
*/
QString ExtensionInfo::name() const
{
    return d->name;
}

/*!
    \fn QString ExtensionInfo::version() const
    The extension version. This is valid after the ExtensionInfo::Read state is reached.
*/
QString ExtensionInfo::version() const
{
    return d->version;
}

/*!
    \fn QString ExtensionInfo::compatVersion() const
    The extension compatibility version. This is valid after the ExtensionInfo::Read state is reached.
*/
QString ExtensionInfo::compatVersion() const
{
    return d->compatVersion;
}

/*!
    \fn QString ExtensionInfo::vendor() const
    The extension vendor. This is valid after the ExtensionInfo::Read state is reached.
*/
QString ExtensionInfo::vendor() const
{
    return d->vendor;
}

/*!
    \fn QString ExtensionInfo::copyright() const
    The extension copyright. This is valid after the ExtensionInfo::Read state is reached.
*/
QString ExtensionInfo::copyright() const
{
    return d->copyright;
}

/*!
    \fn QString ExtensionInfo::license() const
    The extension license. This is valid after the ExtensionInfo::Read state is reached.
*/
QString ExtensionInfo::license() const
{
    return d->license;
}

/*!
    \fn QString ExtensionInfo::description() const
    The extension description. This is valid after the ExtensionInfo::Read state is reached.
*/
QString ExtensionInfo::description() const
{
    return d->description;
}

/*!
    \fn QString ExtensionInfo::url() const
    The extension url where you can find more information about the extension. This is valid after the ExtensionInfo::Read state is reached.
*/
QString ExtensionInfo::url() const
{
    return d->url;
}

/*!
    \fn QString ExtensionInfo::category() const
    The category that the extension belongs to. Categories are groups of extensions which allow for keeping them together in the UI.
    Returns an empty string if the extension does not belong to a category.
*/
QString ExtensionInfo::category() const
{
    return d->category;
}

/*!
    \fn bool ExtensionInfo::isExperimental() const
    Returns if the extension has its experimental flag set.
*/
bool ExtensionInfo::isExperimental() const
{
    return d->experimental;
}

/*!
    \fn bool ExtensionInfo::isEnabled() const
    Returns if the extension is loaded at startup. True by default - the user can change it from the Extension settings.
*/
bool ExtensionInfo::isEnabled() const
{
    return d->enabled;
}

bool ExtensionInfo::isDisabledByDependency() const
{
    return d->disabledByDependency;
}

/*!
    \fn QList<ExtensionDependency> ExtensionInfo::dependencies() const
    The extension dependencies. This is valid after the ExtensionInfo::Read state is reached.
*/
QList<ExtensionDependency> ExtensionInfo::dependencies() const
{
    return d->dependencies;
}

/*!
    \fn ExtensionInfo::ExtensionArgumentDescriptions ExtensionInfo::argumentDescriptions() const
    Returns a list of descriptions of command line arguments the extension processes.
*/

ExtensionInfo::ExtensionArgumentDescriptions ExtensionInfo::argumentDescriptions() const
{
    return d->argumentDescriptions;
}

/*!
    \fn QString ExtensionInfo::location() const
    The absolute path to the directory containing the extension xml description file
    this ExtensionInfo corresponds to.
*/
QString ExtensionInfo::location() const
{
    return d->location;
}

/*!
    \fn QString ExtensionInfo::filePath() const
    The absolute path to the extension xml description file (including the file name)
    this ExtensionInfo corresponds to.
*/
QString ExtensionInfo::filePath() const
{
    return d->filePath;
}

/*!
    \fn QStringList ExtensionInfo::arguments() const
    Command line arguments specific to that extension. Set at startup
*/

QStringList ExtensionInfo::arguments() const
{
    return d->arguments;
}

/*!
    \fn void ExtensionInfo::setArguments(const QStringList &arguments)
    Set the command line arguments specific to that extension to \a arguments.
*/

void ExtensionInfo::setArguments(const QStringList &arguments)
{
    d->arguments = arguments;
}

/*!
    \fn ExtensionInfo::addArgument(const QString &argument)
    Adds \a argument to the command line arguments specific to that extension.
*/

void ExtensionInfo::addArgument(const QString &argument)
{
    d->arguments.push_back(argument);
}


/*!
    \fn ExtensionInfo::State ExtensionInfo::state() const
    The state in which the extension currently is.
    See the description of the ExtensionInfo::State enum for details.
*/
ExtensionInfo::State ExtensionInfo::state() const
{
    return d->state;
}

/*!
    \fn bool ExtensionInfo::hasError() const
    Returns whether an error occurred while reading/starting the extension.
*/
bool ExtensionInfo::hasError() const
{
    return d->hasError;
}

/*!
    \fn QString ExtensionInfo::errorString() const
    Detailed, possibly multi-line, error description in case of an error.
*/
QString ExtensionInfo::errorString() const
{
    return d->errorString;
}

/*!
    \fn bool ExtensionInfo::provides(const QString &extensionName, const QString &version) const
    Returns if this extension can be used to fill in a dependency of the given
    \a extensionName and \a version.

        \sa ExtensionInfo::dependencies()
*/
bool ExtensionInfo::provides(const QString &extensionName, const QString &version) const
{
    return d->provides(extensionName, version);
}

/*!
    \fn IExtension *ExtensionInfo::extension() const
    The corresponding IExtension instance, if the extension library has already been successfully loaded,
    i.e. the ExtensionInfo::Loaded state is reached.
*/
IExtension *ExtensionInfo::extension() const
{
    return d->extension;
}

/*!
    \fn QList<ExtensionInfo *> ExtensionInfo::dependencyInfos() const
    Returns the list of dependencies, already resolved to existing extension infos.
    Valid if ExtensionInfo::Resolved state is reached.

    \sa ExtensionInfo::dependencies()
*/
QList<ExtensionInfo *> ExtensionInfo::dependencyInfos() const
{
    return d->dependencyInfos;
}

/*!
    \fn QList<ExtensionInfo *> ExtensionInfo::providesInfos() const
    Returns the list of extensions that depend on this one.

    \sa ExtensionInfo::dependencyInfos()
*/
QList<ExtensionInfo *> ExtensionInfo::providesInfos() const
{
    return d->providesInfos;
}

/*!
    \fn void ExtensionInfo::addDependentExtension(ExtensionInfo *dependent)
    Adds a dependent the list of extensions that depend on this one.

    \sa ExtensionInfo::providesInfos()
*/
void ExtensionInfo::addDependentExtension(ExtensionInfo *dependent)
{
    d->providesInfos.append(dependent);
}

void ExtensionInfo::removeDependentExtension(ExtensionInfo *dependent)
{
    d->providesInfos.removeOne(dependent);
}

//==========ExtensionInfoPrivate==================

namespace {
    const char * const EXTENSION = "extension";
    const char * const EXTENSION_NAME = "name";
    const char * const EXTENSION_VERSION = "version";
    const char * const EXTENSION_COMPATVERSION = "compatVersion";
    const char * const EXTENSION_EXPERIMENTAL = "experimental";
    const char * const VENDOR = "vendor";
    const char * const COPYRIGHT = "copyright";
    const char * const LICENSE = "license";
    const char * const DESCRIPTION = "description";
    const char * const URL = "url";
    const char * const CATEGORY = "category";
    const char * const DEPENDENCYLIST = "dependencies";
    const char * const DEPENDENCY = "dependency";
    const char * const DEPENDENCY_NAME = "name";
    const char * const DEPENDENCY_VERSION = "version";
    const char * const ARGUMENTLIST = "argumentList";
    const char * const ARGUMENT = "argument";
    const char * const ARGUMENT_NAME = "name";
    const char * const ARGUMENT_PARAMETER = "parameter";
}
/*!
    \fn ExtensionInfoPrivate::ExtensionInfoPrivate(ExtensionInfo *info)
    \internal
*/
ExtensionInfoPrivate::ExtensionInfoPrivate(ExtensionInfo *info)
    :
    enabled(true),
    disabledByDependency(false),
    extension(0),
    state(ExtensionInfo::Invalid),
    hasError(false),
    q(info)
{
}

/*!
    \fn bool ExtensionInfoPrivate::read(const QString &fileName)
    \internal
*/
bool ExtensionInfoPrivate::read(const QString &fileName)
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
    state = ExtensionInfo::Invalid;
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
            readExtensionInfo(reader);
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
    state = ExtensionInfo::Read;
    return true;
}

void ExtensionInfo::setEnabled(bool value)
{
    d->enabled = value;
}

/*!
    \fn bool ExtensionInfoPrivate::reportError(const QString &err)
    \internal
*/
bool ExtensionInfoPrivate::reportError(const QString &err)
{
    errorString = err;
    hasError = true;
    return false;
}

static inline QString msgAttributeMissing(const char *elt, const char *attribute)
{
    return QCoreApplication::translate("ExtensionInfo", "'%1' misses attribute '%2'").arg(QLatin1String(elt), QLatin1String(attribute));
}

static inline QString msgInvalidFormat(const char *content)
{
    return QCoreApplication::translate("ExtensionInfo", "'%1' has invalid format").arg(content);
}

static inline QString msgInvalidElement(const QString &name)
{
    return QCoreApplication::translate("ExtensionInfo", "Invalid element '%1'").arg(name);
}

static inline QString msgUnexpectedClosing(const QString &name)
{
    return QCoreApplication::translate("ExtensionInfo", "Unexpected closing element '%1'").arg(name);
}

static inline QString msgUnexpectedToken()
{
    return QCoreApplication::translate("ExtensionInfo", "Unexpected token");
}

/*!
    \fn void ExtensionInfoPrivate::readExtensionInfo(QXmlStreamReader &reader)
    \internal
*/
void ExtensionInfoPrivate::readExtensionInfo(QXmlStreamReader &reader)
{
    QString element = reader.name().toString();
    if (element != QString(EXTENSION)) {
        reader.raiseError(QCoreApplication::translate("ExtensionInfo", "Expected element '%1' as top level element").arg(EXTENSION));
        return;
    }
    name = reader.attributes().value(EXTENSION_NAME).toString();
    if (name.isEmpty()) {
        reader.raiseError(msgAttributeMissing(EXTENSION, EXTENSION_NAME));
        return;
    }
    version = reader.attributes().value(EXTENSION_VERSION).toString();
    if (version.isEmpty()) {
        reader.raiseError(msgAttributeMissing(EXTENSION, EXTENSION_VERSION));
        return;
    }
    if (!isValidVersion(version)) {
        reader.raiseError(msgInvalidFormat(EXTENSION_VERSION));
        return;
    }
    compatVersion = reader.attributes().value(EXTENSION_COMPATVERSION).toString();
    if (!compatVersion.isEmpty() && !isValidVersion(compatVersion)) {
        reader.raiseError(msgInvalidFormat(EXTENSION_COMPATVERSION));
        return;
    } else if (compatVersion.isEmpty()) {
        compatVersion = version;
    }
    QString experimentalString = reader.attributes().value(EXTENSION_EXPERIMENTAL).toString();
    experimental = (experimentalString.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
    if (!experimentalString.isEmpty() && !experimental
            && experimentalString.compare(QLatin1String("false"), Qt::CaseInsensitive) != 0) {
        reader.raiseError(msgInvalidFormat(EXTENSION_EXPERIMENTAL));
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
    \fn void ExtensionInfoPrivate::readArgumentDescriptions(QXmlStreamReader &reader)
    \internal
*/

void ExtensionInfoPrivate::readArgumentDescriptions(QXmlStreamReader &reader)
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
    \fn void ExtensionInfoPrivate::readArgumentDescription(QXmlStreamReader &reader)
    \internal
*/
void ExtensionInfoPrivate::readArgumentDescription(QXmlStreamReader &reader)
{
    ExtensionArgumentDescription arg;
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
    \fn void ExtensionInfoPrivate::readDependencies(QXmlStreamReader &reader)
    \internal
*/
void ExtensionInfoPrivate::readDependencies(QXmlStreamReader &reader)
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
    \fn void ExtensionInfoPrivate::readDependencyEntry(QXmlStreamReader &reader)
    \internal
*/
void ExtensionInfoPrivate::readDependencyEntry(QXmlStreamReader &reader)
{
    ExtensionDependency dep;
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
    \fn bool ExtensionInfoPrivate::provides(const QString &extensionName, const QString &extensionVersion) const
    \internal
*/
bool ExtensionInfoPrivate::provides(const QString &extensionName, const QString &extensionVersion) const
{
    if (QString::compare(extensionName, name, Qt::CaseInsensitive) != 0)
        return false;
    return (versionCompare(version, extensionVersion) >= 0) && (versionCompare(compatVersion, extensionVersion) <= 0);
}

/*!
    \fn QRegExp &ExtensionInfoPrivate::versionRegExp()
    \internal
*/
QRegExp &ExtensionInfoPrivate::versionRegExp()
{
    static QRegExp reg("([0-9]+)(?:[.]([0-9]+))?(?:[.]([0-9]+))?(?:_([0-9]+))?");
    return reg;
}
/*!
    \fn bool ExtensionInfoPrivate::isValidVersion(const QString &version)
    \internal
*/
bool ExtensionInfoPrivate::isValidVersion(const QString &version)
{
    return versionRegExp().exactMatch(version);
}

/*!
    \fn int ExtensionInfoPrivate::versionCompare(const QString &version1, const QString &version2)
    \internal
*/
int ExtensionInfoPrivate::versionCompare(const QString &version1, const QString &version2)
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
    \fn bool ExtensionInfoPrivate::resolveDependencies(const QList<ExtensionInfo *> &infos)
    \internal
*/
bool ExtensionInfoPrivate::resolveDependencies(const QList<ExtensionInfo *> &infos)
{
    if (hasError)
        return false;
    if (state == ExtensionInfo::Resolved)
        state = ExtensionInfo::Read; // Go back, so we just re-resolve the dependencies.
    if (state != ExtensionInfo::Read) {
        errorString = QCoreApplication::translate("ExtensionInfo", "Resolving dependencies failed because state != Read");
        hasError = true;
        return false;
    }
    QList<ExtensionInfo *> resolvedDependencies;
    Q_FOREACH (const ExtensionDependency &dependency, dependencies) {
        ExtensionInfo *found = 0;

        Q_FOREACH (ExtensionInfo *info, infos) {
            if (info->provides(dependency.name, dependency.version)) {
                found = info;
                if (!info->isEnabled() || info->isDisabledByDependency())
                    disabledByDependency = true;

                info->addDependentExtension(q);

                break;
            }
        }
        if (!found) {
            hasError = true;
            if (!errorString.isEmpty())
                errorString.append(QLatin1Char('\n'));
            errorString.append(QCoreApplication::translate("ExtensionInfo", "Could not resolve dependency '%1(%2)'")
                .arg(dependency.name).arg(dependency.version));
            continue;
        }
        resolvedDependencies.append(found);
    }
    if (hasError)
        return false;

    dependencyInfos = resolvedDependencies;

    if (enabled && !disabledByDependency)
        state = ExtensionInfo::Resolved;

    return true;
}

/*!
    \fn bool ExtensionInfoPrivate::loadLibrary()
    \internal
*/
bool ExtensionInfoPrivate::loadLibrary()
{
    if (hasError)
        return false;
    if (state != ExtensionInfo::Resolved) {
        if (state == ExtensionInfo::Loaded)
            return true;
        errorString = QCoreApplication::translate("ExtensionInfo", "Loading the library failed because state != Resolved");
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

    ExtensionLoader loader(libName);
    if (!loader.load()) {
        hasError = true;
        errorString = QDir::toNativeSeparators(libName)
            + QString::fromLatin1(": ") + loader.errorString();
        return false;
    }
    IExtension *extensionObject = qobject_cast<IExtension*>(loader.instance());
    if (!extensionObject) {
        hasError = true;
        errorString = QCoreApplication::translate("ExtensionInfo", "Extension is not valid (does not derive from IExtension)");
        loader.unload();
        return false;
    }
    state = ExtensionInfo::Loaded;
    extension = extensionObject;
    extension->d->extensionInfo = q;
    return true;
}

/*!
    \fn bool ExtensionInfoPrivate::initializeExtension()
    \internal
*/
bool ExtensionInfoPrivate::initializeExtension()
{
    if (hasError)
        return false;
    if (state != ExtensionInfo::Loaded) {
        if (state == ExtensionInfo::Initialized)
            return true;
        errorString = QCoreApplication::translate("ExtensionInfo", "Initializing the extension failed because state != Loaded");
        hasError = true;
        return false;
    }
    if (!extension) {
        errorString = QCoreApplication::translate("ExtensionInfo", "Internal error: have no extension instance to initialize");
        hasError = true;
        return false;
    }
    QString err;
    if (!extension->initialize(arguments, &err)) {
        errorString = QCoreApplication::translate("ExtensionInfo", "Extension initialization failed: %1").arg(err);
        hasError = true;
        return false;
    }
    state = ExtensionInfo::Initialized;
    return true;
}

/*!
    \fn bool ExtensionInfoPrivate::initializeExtensions()
    \internal
*/
bool ExtensionInfoPrivate::initializeExtensions()
{
    if (hasError)
        return false;
    if (state != ExtensionInfo::Initialized) {
        if (state == ExtensionInfo::Running)
            return true;
        errorString = QCoreApplication::translate("ExtensionInfo", "Cannot perform extensionsInitialized because state != Initialized");
        hasError = true;
        return false;
    }
    if (!extension) {
        errorString = QCoreApplication::translate("ExtensionInfo", "Internal error: have no extension instance to perform extensionsInitialized");
        hasError = true;
        return false;
    }
    extension->extensionsInitialized();
    state = ExtensionInfo::Running;
    return true;
}

/*!
    \fn bool ExtensionInfoPrivate::stop()
    \internal
*/
void ExtensionInfoPrivate::stop()
{
    if (!extension)
        return;
    extension->shutdown();
    state = ExtensionInfo::Stopped;
}

/*!
    \fn bool ExtensionInfoPrivate::kill()
    \internal
*/
void ExtensionInfoPrivate::kill()
{
    if (!extension)
        return;
    delete extension;
    extension = 0;
    state = ExtensionInfo::Deleted;
}

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
