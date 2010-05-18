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

#include "PluginManager.h"
#include "PluginManager_p.h"
#include "PluginInfo.h"
#include "PluginInfo_p.h"
#include "OptionsParser.h"
#include "IPlugin.h"
#include "PluginCollection.h"

#include <QtCore/QMetaProperty>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QWriteLocker>
#include <QtCore/QTime>
#include <QtCore/QDateTime>
#include <QtCore/QSettings>
#include <QtDebug>
#ifdef WITH_TESTS
#include <QTest>
#endif

static const char * const C_IGNORED_PLUGINS = "Plugins/Ignored";
static const char * const C_FORCEENABLED_PLUGINS = "Plugins/ForceEnabled";

typedef QList<ExtensionSystem::PluginInfo *> PluginInfoSet;

enum { debugLeaks = 0 };

/*!
    \namespace ExtensionSystem
    \brief The ExtensionSystem namespace provides classes that belong to the core plugin system.

    The basic extension system contains of the plugin manager and its supporting classes,
    and the IPlugin interface that must be implemented by plugin providers.
*/

/*!
    \namespace ExtensionSystem::Internal
    \internal
*/

/*!
    \class ExtensionSystem::PluginManager
    \mainclass

    \brief Core plugin system that manages the plugins, their life cycle and their registered objects.

    The plugin manager is used for the following tasks:
    \list
    \o Manage plugins and their state
    \o Manipulate a 'common object pool'
    \endlist

    \section1 Plugins
    Plugins consist of an xml descriptor file, and of a library that contains a Qt plugin
    (declared via Q_EXPORT_PLUGIN) that must derive from the IPlugin class.
    The plugin manager is used to set a list of file system directories to search for
    plugins, retrieve information about the state of these plugins, and to load them.

    Usually the application creates a PluginManager instance and initiates the loading.
    \code
        ExtensionSystem::PluginManager *manager = new ExtensionSystem::PluginManager();
        manager->setPluginPaths(QStringList() << "plugins"); // 'plugins' and subdirs will be searched for plugins
        manager->loadPlugins(); // try to load all the plugins
    \endcode
    Additionally it is possible to directly access to the plugin informations
    (the information in the descriptor file), and the plugin instances (via PluginInfo),
    and their state.

    \section1 Object Pool
    Plugins (and everybody else) can add objects to a common 'pool' that is located in
    the plugin manager. Objects in the pool must derive from QObject, there are no other
    prerequisites. All objects of a specified type can be retrieved from the object pool
    via the getObjects() and getObject() methods. They are aware of Aggregation::Aggregate, i.e.
    these methods use the Aggregation::query methods instead of a qobject_cast to determine
    the matching objects.

    Whenever the state of the object pool changes a corresponding signal is emitted by the plugin manager.

    A common usecase for the object pool is that a plugin (or the application) provides
    an "extension point" for other plugins, which is a class / interface that can
    be implemented and added to the object pool. The plugin that provides the
    extension point looks for implementations of the class / interface in the object pool.
    \code
        // plugin A provides a "MimeTypeHandler" extension point
        // in plugin B:
        MyMimeTypeHandler *handler = new MyMimeTypeHandler();
        ExtensionSystem::PluginManager::instance()->addObject(handler);
        // in plugin A:
        QList<MimeTypeHandler *> mimeHandlers =
            ExtensionSystem::PluginManager::instance()->getObjects<MimeTypeHandler>();
    \endcode

    \bold Note: The object pool manipulating functions are thread-safe.
*/

/*!
    \fn void PluginManager::objectAdded(QObject *obj)
    Signal that \a obj has been added to the object pool.
*/

/*!
    \fn void PluginManager::aboutToRemoveObject(QObject *obj)
    Signal that \a obj will be removed from the object pool.
*/

/*!
    \fn void PluginManager::pluginsChanged()
    Signal that the list of available plugins has changed.

    \sa plugins()
*/

/*!
    \fn T *PluginManager::getObject() const
    Retrieve the object of a given type from the object pool.
    This method is aware of Aggregation::Aggregate, i.e. it uses
    the Aggregation::query methods instead of qobject_cast to
    determine the type of an object.
    If there are more than one object of the given type in
    the object pool, this method will choose an arbitrary one of them.

    \sa addObject()
*/

/*!
    \fn QList<T *> PluginManager::getObjects() const
    Retrieve all objects of a given type from the object pool.
    This method is aware of Aggregation::Aggregate, i.e. it uses
    the Aggregation::query methods instead of qobject_cast to
    determine the type of an object.

    \sa addObject()
*/

using namespace ExtensionSystem;
using namespace ExtensionSystem::Internal;

static bool lessThanByPluginName(const PluginInfo *one, const PluginInfo *two)
{
    return one->name() < two->name();
}

PluginManager *PluginManager::m_instance = 0;

/*!
    \fn PluginManager *PluginManager::instance()
    Get the unique plugin manager instance.
*/
PluginManager *PluginManager::instance()
{
    return m_instance;
}

/*!
    \fn PluginManager::PluginManager()
    Create a plugin manager. Should be done only once per application.
*/
PluginManager::PluginManager()
    : d(new PluginManagerPrivate(this))
{
    m_instance = this;
}

/*!
    \fn PluginManager::~PluginManager()
    \internal
*/
PluginManager::~PluginManager()
{
    delete d;
    d = 0;
}

/*!
    \fn void PluginManager::addObject(QObject *obj)
    Add the given object \a obj to the object pool, so it can be retrieved again from the pool by type.
    The plugin manager does not do any memory management - added objects
    must be removed from the pool and deleted manually by whoever is responsible for the object.

    Emits the objectAdded() signal.

    \sa PluginManager::removeObject()
    \sa PluginManager::getObject()
    \sa PluginManager::getObjects()
*/
void PluginManager::addObject(QObject *obj)
{
    d->addObject(obj);
}

/*!
    \fn void PluginManager::removeObject(QObject *obj)
    Emits aboutToRemoveObject() and removes the object \a obj from the object pool.
    \sa PluginManager::addObject()
*/
void PluginManager::removeObject(QObject *obj)
{
    d->removeObject(obj);
}

/*!
    \fn QList<QObject *> PluginManager::allObjects() const
    Retrieve the list of all objects in the pool, unfiltered.
    Usually clients do not need to call this.
    \sa PluginManager::getObject()
    \sa PluginManager::getObjects()
*/
QList<QObject *> PluginManager::allObjects() const
{
    return d->allObjects;
}

/*!
    \fn void PluginManager::loadPlugins()
    Tries to load all the plugins that were previously found when
    setting the plugin search paths. The plugin infos of the plugins
    can be used to retrieve error and state information about individual plugins.

    \sa setPluginPaths()
    \sa plugins()
*/
void PluginManager::loadPlugins()
{
    return d->loadPlugins();
}

/*!
    \fn QStringList PluginManager::pluginPaths() const
    The list of paths were the plugin manager searches for plugins.

    \sa setPluginPaths()
*/
QStringList PluginManager::pluginPaths() const
{
    return d->pluginPaths;
}

/*!
    \fn void PluginManager::setPluginPaths(const QStringList &paths)
    Sets the plugin search paths, i.e. the file system paths where the plugin manager
    looks for plugin descriptions. All given \a paths and their sub directory trees
    are searched for plugin xml description files.

    \sa pluginPaths()
    \sa loadPlugins()
*/
void PluginManager::setPluginPaths(const QStringList &paths)
{
    d->setPluginPaths(paths);
}

/*!
    \fn QString PluginManager::fileExtension() const
    The file extension of plugin description files.
    The default is "xml".

    \sa setFileExtension()
*/
QString PluginManager::fileExtension() const
{
    return d->extension;
}

/*!
    \fn void PluginManager::setFileExtension(const QString &extension)
    Sets the file extension of plugin description files.
    The default is "xml".
    At the moment this must be called before setPluginPaths() is called.
    // ### TODO let this + setPluginPaths read the plugin infos lazyly whenever loadPlugins() or plugins() is called.
*/
void PluginManager::setFileExtension(const QString &extension)
{
    d->extension = extension;
}

void PluginManager::loadSettings()
{
    d->loadSettings();
}

void PluginManager::writeSettings()
{
    d->writeSettings();
}

/*!
    \fn QStringList PluginManager::arguments() const
    The arguments left over after parsing (Neither startup nor plugin
    arguments). Typically, this will be the list of files to open.
*/
QStringList PluginManager::arguments() const
{
    return d->arguments;
}

/*!
    \fn QList<PluginInfo *> PluginManager::plugins() const
    List of all plugin informations that have been found in the plugin search paths.
    This list is valid directly after the setPluginPaths() call.
    The plugin informations contain the information from the plugins' xml description files
    and the current state of the plugins. If a plugin's library has been already successfully loaded,
    the plugin information has a reference to the created plugin instance as well.

    \sa setPluginPaths()
*/
QList<PluginInfo *> PluginManager::plugins() const
{
    return d->pluginInfos;
}

QHash<QString, PluginCollection *> PluginManager::pluginCollections() const
{
    return d->pluginCategories;
}

/*!
    \fn QString PluginManager::serializedArguments() const

    Serialize plugin options and arguments for sending in a single string
    via QtSingleApplication:
    ":myplugin|-option1|-option2|:arguments|argument1|argument2",
    as a list of lists started by a keyword with a colon. Arguments are last.

    \sa setPluginPaths()
*/

static const char argumentKeywordC[] = ":arguments";

QString PluginManager::serializedArguments() const
{
    const QChar separator = QLatin1Char('|');
    QString rc;
    foreach (const PluginInfo *ps, plugins()) {
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
    \fn PluginManager::remoteArguments(const QString &argument)

    Parses the options encoded by serializedArguments() const
    and passes them on to the respective plugins along with the arguments.
*/

void PluginManager::remoteArguments(const QString &serializedArgument)
{
    if (serializedArgument.isEmpty())
        return;
    QStringList serializedArguments = serializedArgument.split(QLatin1Char('|'));
    const QStringList arguments = subList(serializedArguments, QLatin1String(argumentKeywordC));
    foreach (const PluginInfo *ps, plugins()) {
        if (ps->state() == PluginInfo::Running) {
            const QStringList pluginOptions = subList(serializedArguments, QLatin1Char(':') + ps->name());
            ps->plugin()->remoteCommand(pluginOptions, arguments);
        }
    }
}

/*!
    \fn bool PluginManager::parseOptions(const QStringList &args, const QMap<QString, bool> &appOptions, QMap<QString, QString> *foundAppOptions, QString *errorString)
    Takes the list of command line options in \a args and parses them.
    The plugin manager itself might process some options itself directly (-noload <plugin>), and
    adds options that are registered by plugins to their plugin infos.
    The caller (the application) may register itself for options via the \a appOptions list, containing pairs
    of "option string" and a bool that indicates if the option requires an argument.
    Application options always override any plugin's options.

    \a foundAppOptions is set to pairs of ("option string", "argument") for any application options that were found.
    The command line options that were not processed can be retrieved via the arguments() method.
    If an error occurred (like missing argument for an option that requires one), \a errorString contains
    a descriptive message of the error.

    Returns if there was an error.
 */
bool PluginManager::parseOptions(const QStringList &args,
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
    \fn static PluginManager::formatOptions(QTextStream &str, int optionIndentation, int descriptionIndentation)

    Format the startup options of the plugin manager for command line help.
*/

void PluginManager::formatOptions(QTextStream &str, int optionIndentation, int descriptionIndentation)
{
    formatOption(str, QLatin1String(OptionsParser::NO_LOAD_OPTION),
                 QLatin1String("plugin"), QLatin1String("Do not load <plugin>"),
                 optionIndentation, descriptionIndentation);
    formatOption(str, QLatin1String(OptionsParser::PROFILE_OPTION),
                 QString(), QLatin1String("Profile plugin loading"),
                 optionIndentation, descriptionIndentation);
}

/*!
    \fn PluginManager::formatPluginOptions(QTextStream &str, int optionIndentation, int descriptionIndentation) const

    Format the plugin  options of the plugin infos for command line help.
*/

void PluginManager::formatPluginOptions(QTextStream &str, int optionIndentation, int descriptionIndentation) const
{
    typedef PluginInfo::PluginArgumentDescriptions PluginArgumentDescriptions;
    // Check plugins for options
    const PluginInfoSet::const_iterator pcend = d->pluginInfos.constEnd();
    for (PluginInfoSet::const_iterator pit = d->pluginInfos.constBegin(); pit != pcend; ++pit) {
        const PluginArgumentDescriptions pargs = (*pit)->argumentDescriptions();
        if (!pargs.empty()) {
            str << "\nPlugin: " <<  (*pit)->name() << '\n';
            const PluginArgumentDescriptions::const_iterator acend = pargs.constEnd();
            for (PluginArgumentDescriptions::const_iterator ait =pargs.constBegin(); ait != acend; ++ait)
                formatOption(str, ait->name, ait->parameter, ait->description, optionIndentation, descriptionIndentation);
        }
    }
}

/*!
    \fn PluginManager::formatPluginVersions(QTextStream &str) const

    Format the version of the plugin infos for command line help.
*/

void PluginManager::formatPluginVersions(QTextStream &str) const
{
    const PluginInfoSet::const_iterator cend = d->pluginInfos.constEnd();
    for (PluginInfoSet::const_iterator it = d->pluginInfos.constBegin(); it != cend; ++it) {
        const PluginInfo *ps = *it;
        str << "  " << ps->name() << ' ' << ps->version() << ' ' << ps->description() <<  '\n';
    }
}

void PluginManager::startTests()
{
#ifdef WITH_TESTS
    foreach (PluginInfo *pluginInfo, d->testInfos) {
        const QMetaObject *mo = pluginInfo->plugin()->metaObject();
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
        QTest::qExec(pluginInfo->plugin(), methods);
    }
#endif
}

/*!
 * \fn bool PluginManager::runningTests() const
 * \internal
 */
bool PluginManager::runningTests() const
{
    return !d->testInfos.isEmpty();
}

/*!
 * \fn QString PluginManager::testDataDirectory() const
 * \internal
 */
QString PluginManager::testDataDirectory() const
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

//============PluginManagerPrivate===========

/*!
    \fn PluginInfo *PluginManagerPrivate::createInfo()
    \internal
*/
PluginInfo *PluginManagerPrivate::createInfo()
{
    return new PluginInfo();
}

/*!
    \fn PluginInfoPrivate *PluginManagerPrivate::privateInfo(PluginInfo *info)
    \internal
*/
PluginInfoPrivate *PluginManagerPrivate::privateInfo(PluginInfo *info)
{
    return info->d;
}

/*!
    \fn PluginManagerPrivate::PluginManagerPrivate(PluginManager *pluginManager)
    \internal
*/
PluginManagerPrivate::PluginManagerPrivate(PluginManager *pluginManager) :
    extension(QLatin1String("xml")),
    m_profileElapsedMS(0),
    q(pluginManager)
{
}

/*!
    \fn PluginManagerPrivate::~PluginManagerPrivate()
    \internal
*/
PluginManagerPrivate::~PluginManagerPrivate()
{
    stopAll();
    qDeleteAll(pluginInfos);
    qDeleteAll(pluginCategories);
    if (!allObjects.isEmpty()) {
        qDebug() << "There are" << allObjects.size() << "objects left in the plugin manager pool: " << allObjects;
    }
}

void PluginManagerPrivate::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                                 QLatin1String("Unison"), QLatin1String("UnisonStudio"));

    QStringList tempDisabledPlugins;
    QStringList tempForceEnabledPlugins;
    foreach(PluginInfo *info, pluginInfos) {
        if (!info->isExperimental() && !info->isEnabled())
            tempDisabledPlugins.append(info->name());
        if (info->isExperimental() && info->isEnabled())
            tempForceEnabledPlugins.append(info->name());
    }

    settings.setValue(QLatin1String(C_IGNORED_PLUGINS), tempDisabledPlugins);
    settings.setValue(QLatin1String(C_FORCEENABLED_PLUGINS), tempForceEnabledPlugins);
}

void PluginManagerPrivate::loadSettings()
{
    const QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                                 QLatin1String("Unison"), QLatin1String("UnisonStudio"));

    disabledPlugins = settings.value(QLatin1String(C_IGNORED_PLUGINS)).toStringList();
    forceEnabledPlugins = settings.value(QLatin1String(C_FORCEENABLED_PLUGINS)).toStringList();
}

void PluginManagerPrivate::stopAll()
{
    QList<PluginInfo *> queue = loadQueue();
    foreach (PluginInfo *info, queue) {
        loadPlugin(info, PluginInfo::Stopped);
    }
    QListIterator<PluginInfo *> it(queue);
    it.toBack();
    while (it.hasPrevious()) {
        loadPlugin(it.previous(), PluginInfo::Deleted);
    }
}

/*!
    \fn void PluginManagerPrivate::addObject(QObject *obj)
    \internal
*/
void PluginManagerPrivate::addObject(QObject *obj)
{
    {
        QWriteLocker lock(&(q->m_lock));
        if (obj == 0) {
            qWarning() << "PluginManagerPrivate::addObject(): trying to add null object";
            return;
        }
        if (allObjects.contains(obj)) {
            qWarning() << "PluginManagerPrivate::addObject(): trying to add duplicate object";
            return;
        }

        if (debugLeaks)
            qDebug() << "PluginManagerPrivate::addObject" << obj << obj->objectName();

        allObjects.append(obj);
    }
    emit q->objectAdded(obj);
}

/*!
    \fn void PluginManagerPrivate::removeObject(QObject *obj)
    \internal
*/
void PluginManagerPrivate::removeObject(QObject *obj)
{
    if (obj == 0) {
        qWarning() << "PluginManagerPrivate::removeObject(): trying to remove null object";
        return;
    }

    if (!allObjects.contains(obj)) {
        qWarning() << "PluginManagerPrivate::removeObject(): object not in list:"
            << obj << obj->objectName();
        return;
    }
    if (debugLeaks)
        qDebug() << "PluginManagerPrivate::removeObject" << obj << obj->objectName();

    emit q->aboutToRemoveObject(obj);
    QWriteLocker lock(&(q->m_lock));
    allObjects.removeAll(obj);
}

/*!
    \fn void PluginManagerPrivate::loadPlugins()
    \internal
*/
void PluginManagerPrivate::loadPlugins()
{
    QList<PluginInfo *> queue = loadQueue();
    foreach (PluginInfo *info, queue) {
        loadPlugin(info, PluginInfo::Loaded);
    }
    foreach (PluginInfo *info, queue) {
        loadPlugin(info, PluginInfo::Initialized);
    }
    QListIterator<PluginInfo *> it(queue);
    it.toBack();
    while (it.hasPrevious()) {
        loadPlugin(it.previous(), PluginInfo::Running);
    }
    emit q->pluginsChanged();
}

/*!
    \fn void PluginManagerPrivate::loadQueue()
    \internal
*/
QList<PluginInfo *> PluginManagerPrivate::loadQueue()
{
    QList<PluginInfo *> queue;
    foreach (PluginInfo *info, pluginInfos) {
        QList<PluginInfo *> circularityCheckQueue;
        loadQueue(info, queue, circularityCheckQueue);
    }
    return queue;
}

/*!
    \fn bool PluginManagerPrivate::loadQueue(PluginInfo *info, QList<PluginInfo *> &queue, QList<PluginInfo *> &circularityCheckQueue)
    \internal
*/
bool PluginManagerPrivate::loadQueue(PluginInfo *info, QList<PluginInfo *> &queue,
        QList<PluginInfo *> &circularityCheckQueue)
{
    if (queue.contains(info))
        return true;
    // check for circular dependencies
    if (circularityCheckQueue.contains(info)) {
        info->d->hasError = true;
        info->d->errorString = PluginManager::tr("Circular dependency detected:\n");
        int index = circularityCheckQueue.indexOf(info);
        for (int i = index; i < circularityCheckQueue.size(); ++i) {
            info->d->errorString.append(PluginManager::tr("%1(%2) depends on\n")
                .arg(circularityCheckQueue.at(i)->name()).arg(circularityCheckQueue.at(i)->version()));
        }
        info->d->errorString.append(PluginManager::tr("%1(%2)").arg(info->name()).arg(info->version()));
        return false;
    }
    circularityCheckQueue.append(info);
    // check if we have the dependencies
    if (info->state() == PluginInfo::Invalid || info->state() == PluginInfo::Read) {
        if (!info->isDisabledByDependency() && info->isEnabled()) {
            info->d->hasError = true;
            info->d->errorString += "\n";
            info->d->errorString += PluginManager::tr("Cannot load plugin because dependencies are not resolved");
        }
        return false;
    }
    // add dependencies
    foreach (PluginInfo *depInfo, info->dependencyInfos()) {
        if (!loadQueue(depInfo, queue, circularityCheckQueue)) {
            info->d->hasError = true;
            info->d->errorString =
                PluginManager::tr("Cannot load plugin because dependency failed to load: %1(%2)\nReason: %3")
                    .arg(depInfo->name()).arg(depInfo->version()).arg(depInfo->errorString());
            return false;
        }
    }
    // add self
    queue.append(info);
    return true;
}

/*!
    \fn void PluginManagerPrivate::loadPlugin(PluginInfo *info, PluginInfo::State destState)
    \internal
*/
void PluginManagerPrivate::loadPlugin(PluginInfo *info, PluginInfo::State destState)
{
    if (info->hasError() || info->isDisabledByDependency())
        return;

    switch (destState) {
    case PluginInfo::Running:
        profilingReport(">initializeExtensions", info);
        info->d->initializeExtensions();
        profilingReport("<initializeExtensions", info);
        return;
    case PluginInfo::Deleted:
        info->d->kill();
        return;
    default:
        break;
    }
    foreach (const PluginInfo *depInfo, info->dependencyInfos()) {
        if (depInfo->state() != destState) {
            info->d->hasError = true;
            info->d->errorString =
                PluginManager::tr("Cannot load plugin because dependency failed to load: %1(%2)\nReason: %3")
                    .arg(depInfo->name()).arg(depInfo->version()).arg(depInfo->errorString());
            return;
        }
    }
    switch (destState) {
    case PluginInfo::Loaded:
        profilingReport(">loadLibrary", info);
        info->d->loadLibrary();
        profilingReport("<loadLibrary", info);
        break;
    case PluginInfo::Initialized:
        profilingReport(">initializePlugin", info);
        info->d->initializePlugin();
        profilingReport("<initializePlugin", info);
        break;
    case PluginInfo::Stopped:
        profilingReport(">stop", info);
        info->d->stop();
        profilingReport("<stop", info);
        break;
    default:
        break;
    }
}

/*!
    \fn void PluginManagerPrivate::setPluginPaths(const QStringList &paths)
    \internal
*/
void PluginManagerPrivate::setPluginPaths(const QStringList &paths)
{
    pluginPaths = paths;
    loadSettings();
    readPluginPaths();
}

/*!
    \fn void PluginManagerPrivate::readPluginPaths()
    \internal
*/
void PluginManagerPrivate::readPluginPaths()
{
    qDeleteAll(pluginCategories);
    qDeleteAll(pluginInfos);
    pluginInfos.clear();
    pluginCategories.clear();

    QStringList infoFiles;
    QStringList searchPaths = pluginPaths;
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
    defaultCollection = new PluginCollection(QString());
    pluginCategories.insert("", defaultCollection);

    foreach (const QString &infoFile, infoFiles) {
        PluginInfo *info = new PluginInfo;
        info->d->read(infoFile);

        PluginCollection *collection = 0;
        // find correct plugin collection or create a new one
        if (pluginCategories.contains(info->category()))
            collection = pluginCategories.value(info->category());
        else {
            collection = new PluginCollection(info->category());
            pluginCategories.insert(info->category(), collection);
        }
        if (info->isExperimental() && forceEnabledPlugins.contains(info->name()))
            info->setEnabled(true);
        if (!info->isExperimental() && disabledPlugins.contains(info->name()))
            info->setEnabled(false);

        collection->addPlugin(info);
        pluginInfos.append(info);
    }
    resolveDependencies();
    // ensure deterministic plugin load order by sorting
    qSort(pluginInfos.begin(), pluginInfos.end(), lessThanByPluginName);
    emit q->pluginsChanged();
}

void PluginManagerPrivate::resolveDependencies()
{
    foreach (PluginInfo *info, pluginInfos) {
        info->d->resolveDependencies(pluginInfos);
    }
}

 // Look in argument descriptions of the infos for the option.
PluginInfo *PluginManagerPrivate::pluginForOption(const QString &option, bool *requiresArgument) const
{
    // Look in the plugins for an option
    typedef PluginInfo::PluginArgumentDescriptions PluginArgumentDescriptions;

    *requiresArgument = false;
    const PluginInfoSet::const_iterator pcend = pluginInfos.constEnd();
    for (PluginInfoSet::const_iterator pit = pluginInfos.constBegin(); pit != pcend; ++pit) {
        PluginInfo *ps = *pit;
        const PluginArgumentDescriptions pargs = ps->argumentDescriptions();
        if (!pargs.empty()) {
            const PluginArgumentDescriptions::const_iterator acend = pargs.constEnd();
            for (PluginArgumentDescriptions::const_iterator ait = pargs.constBegin(); ait != acend; ++ait) {
                if (ait->name == option) {
                    *requiresArgument = !ait->parameter.isEmpty();
                    return ps;
                }
            }
        }
    }
    return 0;
}

void PluginManagerPrivate::removePluginInfo(PluginInfo *info)
{
    pluginInfos.removeAll(info);

    if (pluginCategories.contains(info->category()))
        pluginCategories.value(info->category())->removePlugin(info);

    foreach(PluginInfo *dep, info->dependencyInfos()) {
        dep->removeDependentPlugin(info);
    }

    delete info;
    info = 0;
}

PluginInfo *PluginManagerPrivate::pluginByName(const QString &name) const
{
    foreach (PluginInfo *info, pluginInfos)
        if (info->name() == name)
            return info;
    return 0;
}

void PluginManagerPrivate::initProfiling()
{
    if (m_profileTimer.isNull()) {
        m_profileTimer.reset(new QTime);
        m_profileTimer->start();
        m_profileElapsedMS = 0;
        qDebug("Profiling started");
    }
}

void PluginManagerPrivate::profilingReport(const char *what, const PluginInfo *info /* = 0 */)
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
