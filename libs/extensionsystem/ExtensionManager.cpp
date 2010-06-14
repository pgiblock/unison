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

#include "ExtensionManager.h"
#include "ExtensionManager_p.h"
#include "ExtensionInfo.h"
#include "ExtensionInfo_p.h"
#include "OptionsParser.h"
#include "IExtension.h"
#include "ExtensionCollection.h"

#include <QMetaProperty>
#include <QDir>
#include <QTextStream>
#include <QWriteLocker>
#include <QTime>
#include <QDateTime>
#include <QSettings>
#include <QtDebug>
#ifdef WITH_TESTS
#include <QTest>
#endif

static const char * const C_IGNORED_EXTENSIONS = "Extensions/Ignored";
static const char * const C_FORCEENABLED_EXTENSIONS = "Extensions/ForceEnabled";

typedef QList<ExtensionSystem::ExtensionInfo *> ExtensionInfoSet;

enum { debugLeaks = 0 };

/*!
    \namespace ExtensionSystem
    \brief The ExtensionSystem namespace provides classes that belong to the core extension system.

    The basic extension system contains of the extension manager and its supporting classes,
    and the IExtension interface that must be implemented by extension providers.
*/

/*!
    \namespace ExtensionSystem::Internal
    \internal
*/

/*!
    \class ExtensionSystem::ExtensionManager
    \mainclass

    \brief Core extension system that manages the extensions, their life cycle and their registered objects.

    The extension manager is used for the following tasks:
    \list
    \o Manage extensions and their state
    \o Manipulate a 'common object pool'
    \endlist

    \section1 Extensions
    Extensions consist of an xml descriptor file, and of a library that contains a Qt extension
    (declared via Q_EXPORT_PLUGIN) that must derive from the IExtension class.
    The extension manager is used to set a list of file system directories to search for
    extensions, retrieve information about the state of these extensions, and to load them.

    Usually the application creates a ExtensionManager instance and initiates the loading.
    \code
        ExtensionSystem::ExtensionManager *manager = new ExtensionSystem::ExtensionManager();
        manager->setExtensionPaths(QStringList() << "extensions"); // 'extensions' and subdirs will be searched for extensions
        manager->loadExtensions(); // try to load all the extensions
    \endcode
    Additionally it is possible to directly access to the extension informations
    (the information in the descriptor file), and the extension instances (via ExtensionInfo),
    and their state.

    \section1 Object Pool
    Extensions (and everybody else) can add objects to a common 'pool' that is located in
    the extension manager. Objects in the pool must derive from QObject, there are no other
    prerequisites. All objects of a specified type can be retrieved from the object pool
    via the getObjects() and getObject() methods. They are aware of Aggregation::Aggregate, i.e.
    these methods use the Aggregation::query methods instead of a qobject_cast to determine
    the matching objects.

    Whenever the state of the object pool changes a corresponding signal is emitted by the extension manager.

    A common usecase for the object pool is that a extension (or the application) provides
    an "extension point" for other extensions, which is a class / interface that can
    be implemented and added to the object pool. The extension that provides the
    extension point looks for implementations of the class / interface in the object pool.
    \code
        // extension A provides a "MimeTypeHandler" extension point
        // in extension B:
        MyMimeTypeHandler *handler = new MyMimeTypeHandler();
        ExtensionSystem::ExtensionManager::instance()->addObject(handler);
        // in extension A:
        QList<MimeTypeHandler *> mimeHandlers =
            ExtensionSystem::ExtensionManager::instance()->getObjects<MimeTypeHandler>();
    \endcode

    \bold Note: The object pool manipulating functions are thread-safe.
*/

/*!
    \fn void ExtensionManager::objectAdded(QObject *obj)
    Signal that \a obj has been added to the object pool.
*/

/*!
    \fn void ExtensionManager::aboutToRemoveObject(QObject *obj)
    Signal that \a obj will be removed from the object pool.
*/

/*!
    \fn void ExtensionManager::extensionsChanged()
    Signal that the list of available extensions has changed.

    \sa extensions()
*/

/*!
    \fn T *ExtensionManager::getObject() const
    Retrieve the object of a given type from the object pool.
    This method is aware of Aggregation::Aggregate, i.e. it uses
    the Aggregation::query methods instead of qobject_cast to
    determine the type of an object.
    If there are more than one object of the given type in
    the object pool, this method will choose an arbitrary one of them.

    \sa addObject()
*/

/*!
    \fn QList<T *> ExtensionManager::getObjects() const
    Retrieve all objects of a given type from the object pool.
    This method is aware of Aggregation::Aggregate, i.e. it uses
    the Aggregation::query methods instead of qobject_cast to
    determine the type of an object.

    \sa addObject()
*/

using namespace ExtensionSystem;
using namespace ExtensionSystem::Internal;

static bool lessThanByExtensionName(const ExtensionInfo *one, const ExtensionInfo *two)
{
    return one->name() < two->name();
}

ExtensionManager *ExtensionManager::m_instance = 0;

/*!
    \fn ExtensionManager *ExtensionManager::instance()
    Get the unique extension manager instance.
*/
ExtensionManager *ExtensionManager::instance()
{
    return m_instance;
}

/*!
    \fn ExtensionManager::ExtensionManager()
    Create a extension manager. Should be done only once per application.
*/
ExtensionManager::ExtensionManager()
    : d(new ExtensionManagerPrivate(this))
{
    m_instance = this;
}

/*!
    \fn ExtensionManager::~ExtensionManager()
    \internal
*/
ExtensionManager::~ExtensionManager()
{
    delete d;
    d = 0;
}

/*!
    \fn void ExtensionManager::addObject(QObject *obj)
    Add the given object \a obj to the object pool, so it can be retrieved again from the pool by type.
    The extension manager does not do any memory management - added objects
    must be removed from the pool and deleted manually by whoever is responsible for the object.

    Emits the objectAdded() signal.

    \sa ExtensionManager::removeObject()
    \sa ExtensionManager::getObject()
    \sa ExtensionManager::getObjects()
*/
void ExtensionManager::addObject(QObject *obj)
{
    d->addObject(obj);
}

/*!
    \fn void ExtensionManager::removeObject(QObject *obj)
    Emits aboutToRemoveObject() and removes the object \a obj from the object pool.
    \sa ExtensionManager::addObject()
*/
void ExtensionManager::removeObject(QObject *obj)
{
    d->removeObject(obj);
}

/*!
    \fn QList<QObject *> ExtensionManager::allObjects() const
    Retrieve the list of all objects in the pool, unfiltered.
    Usually clients do not need to call this.
    \sa ExtensionManager::getObject()
    \sa ExtensionManager::getObjects()
*/
QList<QObject *> ExtensionManager::allObjects() const
{
    return d->allObjects;
}

/*!
    \fn void ExtensionManager::loadExtensions()
    Tries to load all the extensions that were previously found when
    setting the extension search paths. The extension infos of the extensions
    can be used to retrieve error and state information about individual extensions.

    \sa setExtensionPaths()
    \sa extensions()
*/
void ExtensionManager::loadExtensions()
{
    return d->loadExtensions();
}

/*!
    \fn QStringList ExtensionManager::extensionPaths() const
    The list of paths were the extension manager searches for extensions.

    \sa setExtensionPaths()
*/
QStringList ExtensionManager::extensionPaths() const
{
    return d->extensionPaths;
}

/*!
    \fn void ExtensionManager::setExtensionPaths(const QStringList &paths)
    Sets the extension search paths, i.e. the file system paths where the extension manager
    looks for extension descriptions. All given \a paths and their sub directory trees
    are searched for extension xml description files.

    \sa extensionPaths()
    \sa loadExtensions()
*/
void ExtensionManager::setExtensionPaths(const QStringList &paths)
{
    d->setExtensionPaths(paths);
}

/*!
    \fn QString ExtensionManager::fileExtension() const
    The file extension of extension description files.
    The default is "xml".

    \sa setFileExtension()
*/
QString ExtensionManager::fileExtension() const
{
    return d->extension;
}

/*!
    \fn void ExtensionManager::setFileExtension(const QString &extension)
    Sets the file extension of extension description files.
    The default is "xml".
    At the moment this must be called before setExtensionPaths() is called.
    // ### TODO let this + setExtensionPaths read the extension infos lazyly whenever loadExtensions() or extensions() is called.
*/
void ExtensionManager::setFileExtension(const QString &extension)
{
    d->extension = extension;
}

void ExtensionManager::loadSettings()
{
    d->loadSettings();
}

void ExtensionManager::writeSettings()
{
    d->writeSettings();
}

/*!
    \fn QStringList ExtensionManager::arguments() const
    The arguments left over after parsing (Neither startup nor extension
    arguments). Typically, this will be the list of files to open.
*/
QStringList ExtensionManager::arguments() const
{
    return d->arguments;
}

/*!
    \fn QList<ExtensionInfo *> ExtensionManager::extensions() const
    List of all extension informations that have been found in the extension search paths.
    This list is valid directly after the setExtensionPaths() call.
    The extension informations contain the information from the extensions' xml description files
    and the current state of the extensions. If a extension's library has been already successfully loaded,
    the extension information has a reference to the created extension instance as well.

    \sa setExtensionPaths()
*/
QList<ExtensionInfo *> ExtensionManager::extensions() const
{
    return d->extensionInfos;
}

QHash<QString, ExtensionCollection *> ExtensionManager::extensionCollections() const
{
    return d->extensionCategories;
}

/*!
    \fn QString ExtensionManager::serializedArguments() const

    Serialize extension options and arguments for sending in a single string
    via QtSingleApplication:
    ":myextension|-option1|-option2|:arguments|argument1|argument2",
    as a list of lists started by a keyword with a colon. Arguments are last.

    \sa setExtensionPaths()
*/

static const char argumentKeywordC[] = ":arguments";

QString ExtensionManager::serializedArguments() const
{
    const QChar separator = QLatin1Char('|');
    QString rc;
    foreach (const ExtensionInfo *ps, extensions()) {
        if (!ps->arguments().isEmpty()) {
            if (!rc.isEmpty())
                rc += separator;
            rc += QLatin1Char(':');
            rc += ps->name();
            rc += separator;
            rc +=  ps->arguments().join(QString(separator));
        }
    }
    if (!d->arguments.isEmpty()) {
        if (!rc.isEmpty())
            rc += separator;
        rc += QLatin1String(argumentKeywordC);
        // If the argument appears to be a file, make it absolute
        // when sending to another instance.
        foreach(const QString &argument, d->arguments) {
            rc += separator;
            const QFileInfo fi(argument);
            if (fi.exists() && fi.isRelative()) {
                rc += fi.absoluteFilePath();
            } else {
                rc += argument;
            }
        }
    }
    return rc;
}

/* Extract a sublist from the serialized arguments
 * indicated by a keyword starting with a colon indicator:
 * ":a,i1,i2,:b:i3,i4" with ":a" -> "i1,i2"
 */
static QStringList subList(const QStringList &in, const QString &key)
{
    QStringList rc;
    // Find keyword and copy arguments until end or next keyword
    const QStringList::const_iterator inEnd = in.constEnd();
    QStringList::const_iterator it = qFind(in.constBegin(), inEnd, key);
    if (it != inEnd) {
        const QChar nextIndicator = QLatin1Char(':');
        for (++it; it != inEnd && !it->startsWith(nextIndicator); ++it)
            rc.append(*it);
    }
    return rc;
}

/*!
    \fn ExtensionManager::remoteArguments(const QString &argument)

    Parses the options encoded by serializedArguments() const
    and passes them on to the respective extensions along with the arguments.
*/

void ExtensionManager::remoteArguments(const QString &serializedArgument)
{
    if (serializedArgument.isEmpty())
        return;
    QStringList serializedArguments = serializedArgument.split(QLatin1Char('|'));
    const QStringList arguments = subList(serializedArguments, QLatin1String(argumentKeywordC));
    foreach (const ExtensionInfo *ps, extensions()) {
        if (ps->state() == ExtensionInfo::Running) {
            const QStringList extensionOptions = subList(serializedArguments, QLatin1Char(':') + ps->name());
            ps->extension()->remoteCommand(extensionOptions, arguments);
        }
    }
}

/*!
    \fn bool ExtensionManager::parseOptions(const QStringList &args, const QMap<QString, bool> &appOptions, QMap<QString, QString> *foundAppOptions, QString *errorString)
    Takes the list of command line options in \a args and parses them.
    The extension manager itself might process some options itself directly (-noload <extension>), and
    adds options that are registered by extensions to their extension infos.
    The caller (the application) may register itself for options via the \a appOptions list, containing pairs
    of "option string" and a bool that indicates if the option requires an argument.
    Application options always override any extension's options.

    \a foundAppOptions is set to pairs of ("option string", "argument") for any application options that were found.
    The command line options that were not processed can be retrieved via the arguments() method.
    If an error occurred (like missing argument for an option that requires one), \a errorString contains
    a descriptive message of the error.

    Returns if there was an error.
 */
bool ExtensionManager::parseOptions(const QStringList &args,
    const QMap<QString, bool> &appOptions,
    QMap<QString, QString> *foundAppOptions,
    QString *errorString)
{
    OptionsParser options(args, appOptions, foundAppOptions, errorString, d);
    return options.parse();
}



static inline void indent(QTextStream &str, int indent)
{
    const QChar blank = QLatin1Char(' ');
    for (int i = 0 ; i < indent; i++)
        str << blank;
}

static inline void formatOption(QTextStream &str,
                                const QString &opt, const QString &parm, const QString &description,
                                int optionIndentation, int descriptionIndentation)
{
    int remainingIndent = descriptionIndentation - optionIndentation - opt.size();
    indent(str, optionIndentation);
    str << opt;
    if (!parm.isEmpty()) {
        str << " <" << parm << '>';
        remainingIndent -= 3 + parm.size();
    }
    indent(str, qMax(1, remainingIndent));
    str << description << '\n';
}

/*!
    \fn static ExtensionManager::formatOptions(QTextStream &str, int optionIndentation, int descriptionIndentation)

    Format the startup options of the extension manager for command line help.
*/

void ExtensionManager::formatOptions(QTextStream &str, int optionIndentation, int descriptionIndentation)
{
    formatOption(str, QLatin1String(OptionsParser::NO_LOAD_OPTION),
                 QLatin1String("extension"), QLatin1String("Do not load <extension>"),
                 optionIndentation, descriptionIndentation);
    formatOption(str, QLatin1String(OptionsParser::PROFILE_OPTION),
                 QString(), QLatin1String("Profile extension loading"),
                 optionIndentation, descriptionIndentation);
}

/*!
    \fn ExtensionManager::formatExtensionOptions(QTextStream &str, int optionIndentation, int descriptionIndentation) const

    Format the extension  options of the extension infos for command line help.
*/

void ExtensionManager::formatExtensionOptions(QTextStream &str, int optionIndentation, int descriptionIndentation) const
{
    typedef ExtensionInfo::ExtensionArgumentDescriptions ExtensionArgumentDescriptions;
    // Check extensions for options
    const ExtensionInfoSet::const_iterator pcend = d->extensionInfos.constEnd();
    for (ExtensionInfoSet::const_iterator pit = d->extensionInfos.constBegin(); pit != pcend; ++pit) {
        const ExtensionArgumentDescriptions pargs = (*pit)->argumentDescriptions();
        if (!pargs.empty()) {
            str << "\nExtension: " <<  (*pit)->name() << '\n';
            const ExtensionArgumentDescriptions::const_iterator acend = pargs.constEnd();
            for (ExtensionArgumentDescriptions::const_iterator ait =pargs.constBegin(); ait != acend; ++ait)
                formatOption(str, ait->name, ait->parameter, ait->description, optionIndentation, descriptionIndentation);
        }
    }
}

/*!
    \fn ExtensionManager::formatExtensionVersions(QTextStream &str) const

    Format the version of the extension infos for command line help.
*/

void ExtensionManager::formatExtensionVersions(QTextStream &str) const
{
    const ExtensionInfoSet::const_iterator cend = d->extensionInfos.constEnd();
    for (ExtensionInfoSet::const_iterator it = d->extensionInfos.constBegin(); it != cend; ++it) {
        const ExtensionInfo *ps = *it;
        str << "  " << ps->name() << ' ' << ps->version() << ' ' << ps->description() <<  '\n';
    }
}

void ExtensionManager::startTests()
{
#ifdef WITH_TESTS
    foreach (ExtensionInfo *extensionInfo, d->testInfos) {
        const QMetaObject *mo = extensionInfo->extension()->metaObject();
        QStringList methods;
        methods.append("arg0");
        // We only want slots starting with "test"
        for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
            if (QByteArray(mo->method(i).signature()).startsWith("test") &&
                !QByteArray(mo->method(i).signature()).endsWith("_data()")) {
                QString method = QString::fromLatin1(mo->method(i).signature());
                methods.append(method.left(method.size()-2));
            }
        }
        QTest::qExec(extensionInfo->extension(), methods);
    }
#endif
}

/*!
 * \fn bool ExtensionManager::runningTests() const
 * \internal
 */
bool ExtensionManager::runningTests() const
{
    return !d->testInfos.isEmpty();
}

/*!
 * \fn QString ExtensionManager::testDataDirectory() const
 * \internal
 */
QString ExtensionManager::testDataDirectory() const
{
    /*
    QByteArray ba = qgetenv("QTCREATOR_TEST_DIR");
    QString s = QString::fromLocal8Bit(ba.constData(), ba.size());
    if (s.isEmpty()) {
        s = IDE_TEST_DIR;
        s.append("/tests");
    }
    s = QDir::cleanPath(s);
    return s;*/
    return "testDataDirectory-NOT-IMPLEMENTED";
}

//============ExtensionManagerPrivate===========

/*!
    \fn ExtensionInfo *ExtensionManagerPrivate::createInfo()
    \internal
*/
ExtensionInfo *ExtensionManagerPrivate::createInfo()
{
    return new ExtensionInfo();
}

/*!
    \fn ExtensionInfoPrivate *ExtensionManagerPrivate::privateInfo(ExtensionInfo *info)
    \internal
*/
ExtensionInfoPrivate *ExtensionManagerPrivate::privateInfo(ExtensionInfo *info)
{
    return info->d;
}

/*!
    \fn ExtensionManagerPrivate::ExtensionManagerPrivate(ExtensionManager *extensionManager)
    \internal
*/
ExtensionManagerPrivate::ExtensionManagerPrivate(ExtensionManager *extensionManager) :
    extension(QLatin1String("xml")),
    m_profileElapsedMS(0),
    q(extensionManager)
{
}

/*!
    \fn ExtensionManagerPrivate::~ExtensionManagerPrivate()
    \internal
*/
ExtensionManagerPrivate::~ExtensionManagerPrivate()
{
    stopAll();
    qDeleteAll(extensionInfos);
    qDeleteAll(extensionCategories);
    if (!allObjects.isEmpty()) {
        qDebug() << "There are" << allObjects.size() << "objects left in the extension manager pool: " << allObjects;
    }
}

void ExtensionManagerPrivate::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                                 QLatin1String("Unison"), QLatin1String("UnisonStudio"));

    QStringList tempDisabledExtensions;
    QStringList tempForceEnabledExtensions;
    foreach(ExtensionInfo *info, extensionInfos) {
        if (!info->isExperimental() && !info->isEnabled())
            tempDisabledExtensions.append(info->name());
        if (info->isExperimental() && info->isEnabled())
            tempForceEnabledExtensions.append(info->name());
    }

    settings.setValue(QLatin1String(C_IGNORED_EXTENSIONS), tempDisabledExtensions);
    settings.setValue(QLatin1String(C_FORCEENABLED_EXTENSIONS), tempForceEnabledExtensions);
}

void ExtensionManagerPrivate::loadSettings()
{
    const QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                                 QLatin1String("Unison"), QLatin1String("UnisonStudio"));

    disabledExtensions = settings.value(QLatin1String(C_IGNORED_EXTENSIONS)).toStringList();
    forceEnabledExtensions = settings.value(QLatin1String(C_FORCEENABLED_EXTENSIONS)).toStringList();
}

void ExtensionManagerPrivate::stopAll()
{
    QList<ExtensionInfo *> queue = loadQueue();
    foreach (ExtensionInfo *info, queue) {
        loadExtension(info, ExtensionInfo::Stopped);
    }
    QListIterator<ExtensionInfo *> it(queue);
    it.toBack();
    while (it.hasPrevious()) {
        loadExtension(it.previous(), ExtensionInfo::Deleted);
    }
}

/*!
    \fn void ExtensionManagerPrivate::addObject(QObject *obj)
    \internal
*/
void ExtensionManagerPrivate::addObject(QObject *obj)
{
    {
        QWriteLocker lock(&(q->m_lock));
        if (obj == 0) {
            qWarning() << "ExtensionManagerPrivate::addObject(): trying to add null object";
            return;
        }
        if (allObjects.contains(obj)) {
            qWarning() << "ExtensionManagerPrivate::addObject(): trying to add duplicate object";
            return;
        }

        if (debugLeaks)
            qDebug() << "ExtensionManagerPrivate::addObject" << obj << obj->objectName();

        allObjects.append(obj);
    }
    emit q->objectAdded(obj);
}

/*!
    \fn void ExtensionManagerPrivate::removeObject(QObject *obj)
    \internal
*/
void ExtensionManagerPrivate::removeObject(QObject *obj)
{
    if (obj == 0) {
        qWarning() << "ExtensionManagerPrivate::removeObject(): trying to remove null object";
        return;
    }

    if (!allObjects.contains(obj)) {
        qWarning() << "ExtensionManagerPrivate::removeObject(): object not in list:"
            << obj << obj->objectName();
        return;
    }
    if (debugLeaks)
        qDebug() << "ExtensionManagerPrivate::removeObject" << obj << obj->objectName();

    emit q->aboutToRemoveObject(obj);
    QWriteLocker lock(&(q->m_lock));
    allObjects.removeAll(obj);
}

/*!
    \fn void ExtensionManagerPrivate::loadExtensions()
    \internal
*/
void ExtensionManagerPrivate::loadExtensions()
{
    QList<ExtensionInfo *> queue = loadQueue();
    foreach (ExtensionInfo *info, queue) {
        loadExtension(info, ExtensionInfo::Loaded);
    }
    foreach (ExtensionInfo *info, queue) {
        loadExtension(info, ExtensionInfo::Initialized);
    }
    QListIterator<ExtensionInfo *> it(queue);
    it.toBack();
    while (it.hasPrevious()) {
        loadExtension(it.previous(), ExtensionInfo::Running);
    }
    emit q->extensionsChanged();
}

/*!
    \fn void ExtensionManagerPrivate::loadQueue()
    \internal
*/
QList<ExtensionInfo *> ExtensionManagerPrivate::loadQueue()
{
    QList<ExtensionInfo *> queue;
    foreach (ExtensionInfo *info, extensionInfos) {
        QList<ExtensionInfo *> circularityCheckQueue;
        loadQueue(info, queue, circularityCheckQueue);
    }
    return queue;
}

/*!
    \fn bool ExtensionManagerPrivate::loadQueue(ExtensionInfo *info, QList<ExtensionInfo *> &queue, QList<ExtensionInfo *> &circularityCheckQueue)
    \internal
*/
bool ExtensionManagerPrivate::loadQueue(ExtensionInfo *info, QList<ExtensionInfo *> &queue,
        QList<ExtensionInfo *> &circularityCheckQueue)
{
    if (queue.contains(info))
        return true;
    // check for circular dependencies
    if (circularityCheckQueue.contains(info)) {
        info->d->hasError = true;
        info->d->errorString = ExtensionManager::tr("Circular dependency detected:\n");
        int index = circularityCheckQueue.indexOf(info);
        for (int i = index; i < circularityCheckQueue.size(); ++i) {
            info->d->errorString.append(ExtensionManager::tr("%1(%2) depends on\n")
                .arg(circularityCheckQueue.at(i)->name()).arg(circularityCheckQueue.at(i)->version()));
        }
        info->d->errorString.append(ExtensionManager::tr("%1(%2)").arg(info->name()).arg(info->version()));
        return false;
    }
    circularityCheckQueue.append(info);
    // check if we have the dependencies
    if (info->state() == ExtensionInfo::Invalid || info->state() == ExtensionInfo::Read) {
        if (!info->isDisabledByDependency() && info->isEnabled()) {
            info->d->hasError = true;
            info->d->errorString += "\n";
            info->d->errorString += ExtensionManager::tr("Cannot load extension because dependencies are not resolved");
        }
        return false;
    }
    // add dependencies
    foreach (ExtensionInfo *depInfo, info->dependencyInfos()) {
        if (!loadQueue(depInfo, queue, circularityCheckQueue)) {
            info->d->hasError = true;
            info->d->errorString =
                ExtensionManager::tr("Cannot load extension because dependency failed to load: %1(%2)\nReason: %3")
                    .arg(depInfo->name()).arg(depInfo->version()).arg(depInfo->errorString());
            return false;
        }
    }
    // add self
    queue.append(info);
    return true;
}

/*!
    \fn void ExtensionManagerPrivate::loadExtension(ExtensionInfo *info, ExtensionInfo::State destState)
    \internal
*/
void ExtensionManagerPrivate::loadExtension(ExtensionInfo *info, ExtensionInfo::State destState)
{
    if (info->hasError() || info->isDisabledByDependency())
        return;

    switch (destState) {
    case ExtensionInfo::Running:
        profilingReport(">initializeExtensions", info);
        info->d->initializeExtensions();
        profilingReport("<initializeExtensions", info);
        return;
    case ExtensionInfo::Deleted:
        info->d->kill();
        return;
    default:
        break;
    }
    foreach (const ExtensionInfo *depInfo, info->dependencyInfos()) {
        if (depInfo->state() != destState) {
            info->d->hasError = true;
            info->d->errorString =
                ExtensionManager::tr("Cannot load extension because dependency failed to load: %1(%2)\nReason: %3")
                    .arg(depInfo->name()).arg(depInfo->version()).arg(depInfo->errorString());
            return;
        }
    }
    switch (destState) {
    case ExtensionInfo::Loaded:
        profilingReport(">loadLibrary", info);
        info->d->loadLibrary();
        profilingReport("<loadLibrary", info);
        break;
    case ExtensionInfo::Initialized:
        profilingReport(">initializeExtension", info);
        info->d->initializeExtension();
        profilingReport("<initializeExtension", info);
        break;
    case ExtensionInfo::Stopped:
        profilingReport(">stop", info);
        info->d->stop();
        profilingReport("<stop", info);
        break;
    default:
        break;
    }
}

/*!
    \fn void ExtensionManagerPrivate::setExtensionPaths(const QStringList &paths)
    \internal
*/
void ExtensionManagerPrivate::setExtensionPaths(const QStringList &paths)
{
    extensionPaths = paths;
    loadSettings();
    readExtensionPaths();
}

/*!
    \fn void ExtensionManagerPrivate::readExtensionPaths()
    \internal
*/
void ExtensionManagerPrivate::readExtensionPaths()
{
    qDeleteAll(extensionCategories);
    qDeleteAll(extensionInfos);
    extensionInfos.clear();
    extensionCategories.clear();

    QStringList infoFiles;
    QStringList searchPaths = extensionPaths;
    while (!searchPaths.isEmpty()) {
        const QDir dir(searchPaths.takeFirst());
        const QString pattern = QLatin1String("*.") + extension;
        const QFileInfoList files = dir.entryInfoList(QStringList(pattern), QDir::Files);
        foreach (const QFileInfo &file, files)
            infoFiles << file.absoluteFilePath();
        const QFileInfoList dirs = dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
        foreach (const QFileInfo &subdir, dirs)
            searchPaths << subdir.absoluteFilePath();
    }
    defaultCollection = new ExtensionCollection(QString());
    extensionCategories.insert("", defaultCollection);

    foreach (const QString &infoFile, infoFiles) {
        ExtensionInfo *info = new ExtensionInfo;
        info->d->read(infoFile);

        ExtensionCollection *collection = 0;
        // find correct extension collection or create a new one
        if (extensionCategories.contains(info->category()))
            collection = extensionCategories.value(info->category());
        else {
            collection = new ExtensionCollection(info->category());
            extensionCategories.insert(info->category(), collection);
        }
        if (info->isExperimental() && forceEnabledExtensions.contains(info->name()))
            info->setEnabled(true);
        if (!info->isExperimental() && disabledExtensions.contains(info->name()))
            info->setEnabled(false);

        collection->addExtension(info);
        extensionInfos.append(info);
    }
    resolveDependencies();
    // ensure deterministic extension load order by sorting
    qSort(extensionInfos.begin(), extensionInfos.end(), lessThanByExtensionName);
    emit q->extensionsChanged();
}

void ExtensionManagerPrivate::resolveDependencies()
{
    foreach (ExtensionInfo *info, extensionInfos) {
        info->d->resolveDependencies(extensionInfos);
    }
}

 // Look in argument descriptions of the infos for the option.
ExtensionInfo *ExtensionManagerPrivate::extensionForOption(const QString &option, bool *requiresArgument) const
{
    // Look in the extensions for an option
    typedef ExtensionInfo::ExtensionArgumentDescriptions ExtensionArgumentDescriptions;

    *requiresArgument = false;
    const ExtensionInfoSet::const_iterator pcend = extensionInfos.constEnd();
    for (ExtensionInfoSet::const_iterator pit = extensionInfos.constBegin(); pit != pcend; ++pit) {
        ExtensionInfo *ps = *pit;
        const ExtensionArgumentDescriptions pargs = ps->argumentDescriptions();
        if (!pargs.empty()) {
            const ExtensionArgumentDescriptions::const_iterator acend = pargs.constEnd();
            for (ExtensionArgumentDescriptions::const_iterator ait = pargs.constBegin(); ait != acend; ++ait) {
                if (ait->name == option) {
                    *requiresArgument = !ait->parameter.isEmpty();
                    return ps;
                }
            }
        }
    }
    return 0;
}

void ExtensionManagerPrivate::removeExtensionInfo(ExtensionInfo *info)
{
    extensionInfos.removeAll(info);

    if (extensionCategories.contains(info->category()))
        extensionCategories.value(info->category())->removeExtension(info);

    foreach(ExtensionInfo *dep, info->dependencyInfos()) {
        dep->removeDependentExtension(info);
    }

    delete info;
    info = 0;
}

ExtensionInfo *ExtensionManagerPrivate::extensionByName(const QString &name) const
{
    foreach (ExtensionInfo *info, extensionInfos)
        if (info->name() == name)
            return info;
    return 0;
}

void ExtensionManagerPrivate::initProfiling()
{
    if (m_profileTimer.isNull()) {
        m_profileTimer.reset(new QTime);
        m_profileTimer->start();
        m_profileElapsedMS = 0;
        qDebug("Profiling started");
    }
}

void ExtensionManagerPrivate::profilingReport(const char *what, const ExtensionInfo *info /* = 0 */)
{
    if (!m_profileTimer.isNull()) {
        const int absoluteElapsedMS = m_profileTimer->elapsed();
        const int elapsedMS = absoluteElapsedMS - m_profileElapsedMS;
        m_profileElapsedMS = absoluteElapsedMS;
        if (info) {
            qDebug("%-22s %-22s %8dms (%8dms)", what, qPrintable(info->name()), absoluteElapsedMS, elapsedMS);
        } else {
            qDebug("%-22s %8dms (%8dms)", what, absoluteElapsedMS, elapsedMS);
        }
    }
}
