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

#include "extensionsystem/IExtension.hpp"
#include "extensionsystem/ExtensionManager.hpp"
#include "extensionsystem/ExtensionInfo.hpp"
#include "IExtension_p.hpp"

/*!
    \class ExtensionSystem::IExtension
    \mainclass
    \brief Base class for all extensions.

    The IExtension class is an abstract class that must be implemented
    once for each extension.
    A extension consists of two parts: A description file, and a library
    that at least contains the IExtension implementation.

    \tableofcontents

    \section1 Extension Information
    The extension information file is an xml file that contains all
    information that are necessary for loading the extension's library,
    plus some textual descriptions. The file must be located in
    (a subdir of) one of the extension manager's extension search paths,
    and must have the \c .xml extension.

    \section2 Main Tag
    The root tag is \c extension. It has mandatory attributes \c name
    and \c version, and an optional \c compatVersion.
    \table
    \header
        \o Tag
        \o Meaning
    \row
        \o extension
        \o Root element in a extension's xml file.
    \endtable
    \table
    \header
        \o Attribute
        \o Meaning
    \row
        \o name
        \o This is used as an identifier for the extension and can e.g.
           be referenced in other extension's dependencies. It is
           also used to construct the name of the extension library
           as \c lib[name].[dll|.so|.dylib].
    \row
        \o version
        \o Version string in the form \c {"x.y.z_n"}, used for identifying
           the extension.
    \row
        \o compatVersion
        \o Compatibility version. Optional. If not given, it is implicitly
           set to the same value as \c version. The compatibility version
           is used to resolve dependencies on this extension. See
           \l {Dependencies}{Dependencies} for details.
    \endtable

    \section2 Extension-describing Tags
    These are direct children of the \c extension tag, and are solely used
    for more detailed (user centric) description of the extension. All of these
    are optional.
    \table
    \header
        \o Tag
        \o Meaning
    \row
        \o vendor
        \o String that describes the extension creator/vendor,
           like \c {MyCompany}.
    \row
        \o copyright
        \o A short copyright notice, like \c {(C) 2007-2008 MyCompany}.
    \row
        \o license
        \o Possibly multi-line license information about the extension.
    \row
        \o description
        \o Possibly multi-line description of what the extension is supposed
           to provide.
    \row
        \o url
        \o Link to further information about the extension, like
           \c {http://www.mycompany-online.com/products/greatextension}.
    \endtable

    \section2 Dependencies
    A extension can have dependencies on other extensions. These are
    specified in the extension's xml file as well, to ensure that
    these other extensions are loaded before this extension.
    Dependency information consists of the name of the required extension
    (lets denote that as \c {dependencyName}),
    and the required version of the extension (\c {dependencyVersion}).
    A extension with given \c name, \c version and \c compatVersion matches
    the dependency if
    \list
        \o its \c name matches \c dependencyName, and
        \o \c {compatVersion <= dependencyVersion <= version}.
    \endlist

    The xml element that describes dependencies is the \c dependency tag,
    with required attributes \c name and \c version. It is an
    optional direct child of the \c extension tag and can appear multiple times.
    \table
    \header
        \o Tag
        \o Meaning
    \row
        \o dependency
        \o Describes a dependency on another extension.
    \endtable
    \table
    \header
        \o Attribute
        \o Meaning
    \row
        \o name
        \o The name of the extension, on which this extension relies.
    \row
        \o version
        \o The version to which the extension must be compatible to
           fill the dependency, in the form \c {"x.y.z_n"}.
    \endtable

    \section2 Example \c extension.xml
    \code
        <extension name="test" version="1.0.1" compatVersion="1.0.0">
            <vendor>MyCompany</vendor>
            <copyright>(C) 2007 MyCompany</copyright>
            <license>
        This is a default license bla
        blubbblubb
        end of terms
            </license>
            <description>
        This extension is just a test.
            it demonstrates the great use of the extension info.
            </description>
            <url>http://www.mycompany-online.com/products/greatextension</url>
            <dependencyList>
                <dependency name="SomeOtherExtension" version="2.3.0_2"/>
                <dependency name="EvenOther" version="1.0.0"/>
            </dependencyList>
        </extension>
    \endcode
    The first dependency could for example be matched by a extension with
    \code
        <extension name="SomeOtherExtension" version="3.1.0" compatVersion="2.2.0">
        </extension>
    \endcode
    since the name matches, and the version \c "2.3.0_2" given in the dependency tag
    lies in the range of \c "2.2.0" and \c "3.1.0".

    \section2 A Note on Extension Versions
    Extension versions are in the form \c "x.y.z_n" where, x, y, z and n are
    non-negative integer numbers. You don't have to specify the version
    in this full form - any left-out part will implicitly be set to zero.
    So, \c "2.10_2" is equal to \c "2.10.0_2", and "1" is the same as "1.0.0_0".

    \section1 Extension Implementation
    Extensions must provide one implementation of the IExtension class, located
    in a library that matches the \c name attribute given in their
    xml description. The IExtension implementation must be exported and
    made known to Qt's extension system via the Q_EXPORT_PLUGIN macro, see the
    Qt documentation for details on that.

    After the extensions' xml files have been read, and dependencies have been
    found, the extension loading is done in three phases:
    \list 1
    \o All extension libraries are loaded in 'root-to-leaf' order of the
       dependency tree.
    \o All extensions' initialize methods are called in 'root-to-leaf' order
       of the dependency tree. This is a good place to put
       objects in the extension manager's object pool.
    \o All extensions' extensionsInitialized methods are called in 'leaf-to-root'
       order of the dependency tree. At this point, extensions can
       be sure that all extensions that depend on this extension have
       been initialized completely (implying that they have put
       objects in the object pool, if they want that during the
       initialization sequence).
    \endlist
    If library loading or initialization of a extension fails, all extensions
    that depend on that extension also fail.

    Extensions have access to the extension manager
    (and its object pool) via the ExtensionManager::instance()
    method.
*/

/*!
    \fn bool IExtension::initialize(const QStringList &arguments, QString *errorString)
    Called after the extension has been loaded and the IExtension instance
    has been created. The initialize methods of extensions that depend
    on this extension are called after the initialize method of this extension
    has been called. Extensions should initialize their internal state in this
    method. Returns if initialization of successful. If it wasn't successful,
    the \a errorString should be set to a user-readable message
    describing the reason.
    \sa extensionsInitialized()
*/

/*!
    \fn void IExtension::extensionsInitialized()
    Called after the IExtension::initialize() method has been called,
    and after both the IExtension::initialize() and IExtension::extensionsInitialized()
    methods of extensions that depend on this extension have been called.
    In this method, the extension can assume that extensions that depend on
    this extension are fully 'up and running'. It is a good place to
    look in the extension manager's object pool for objects that have
    been provided by dependent extensions.
    \sa initialize()
*/

/*!
    \fn void IExtension::shutdown()
    Called during a shutdown sequence in the same order as initialization
    before the extensions get deleted in reverse order.
    This method can be used to optimize the shutdown down, e.g. to
    disconnect from the ExtensionManager::aboutToRemoveObject() signal
    if getting the signal (and probably doing lots of stuff to update
    the internal and visible state) doesn't make sense during shutdown.
*/

using namespace ExtensionSystem;

/*!
    \fn IExtension::IExtension()
    \internal
*/
IExtension::IExtension()
    : d(new Internal::IExtensionPrivate())
{
}

/*!
    \fn IExtension::~IExtension()
    \internal
*/
IExtension::~IExtension()
{
    ExtensionManager *pm = ExtensionManager::instance();
    Q_FOREACH (QObject *obj, d->addedObjectsInReverseOrder)
        pm->removeObject(obj);
    qDeleteAll(d->addedObjectsInReverseOrder);
    d->addedObjectsInReverseOrder.clear();
    delete d;
    d = 0;
}

/*!
    \fn ExtensionInfo *IExtension::extensionInfo() const
    Returns the ExtensionInfo corresponding to this extension.
    This is not available in the constructor.
*/
ExtensionInfo *IExtension::extensionInfo() const
{
    return d->extensionInfo;
}

/*!
    \fn void IExtension::addObject(QObject *obj)
    Convenience method that registers \a obj in the extension manager's
    extension pool by just calling ExtensionManager::addObject().
*/
void IExtension::addObject(QObject *obj)
{
    ExtensionManager::instance()->addObject(obj);
}

/*!
    \fn void IExtension::addAutoReleasedObject(QObject *obj)
    Convenience method for registering \a obj in the extension manager's
    extension pool. Usually, registered objects must be removed from
    the object pool and deleted by hand.
    Objects added to the pool via addAutoReleasedObject are automatically
    removed and deleted in reverse order of registration when
    the IExtension instance is destroyed.
    \sa ExtensionManager::addObject()
*/
void IExtension::addAutoReleasedObject(QObject *obj)
{
    d->addedObjectsInReverseOrder.prepend(obj);
    ExtensionManager::instance()->addObject(obj);
}

/*!
    \fn void IExtension::removeObject(QObject *obj)
    Convenience method that unregisters \a obj from the extension manager's
    extension pool by just calling ExtensionManager::removeObject().
*/
void IExtension::removeObject(QObject *obj)
{
    ExtensionManager::instance()->removeObject(obj);
}

