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

#ifndef EXTENSIONINFO_H
#define EXTENSIONINFO_H

#include "extensionsystem_global.hpp"

#include <QString>
#include <QList>

QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE

namespace ExtensionSystem {

namespace Internal {
    class ExtensionInfoPrivate;
    class ExtensionManagerPrivate;
}

class IExtension;

struct EXTENSIONSYSTEM_EXPORT ExtensionDependency
{
    QString name;
    QString version;
    bool operator==(const ExtensionDependency &other);
};

struct EXTENSIONSYSTEM_EXPORT ExtensionArgumentDescription
{
    QString name;
    QString parameter;
    QString description;
};

class EXTENSIONSYSTEM_EXPORT ExtensionInfo
{
public:
    enum State { Invalid, Read, Resolved, Loaded, Initialized, Running, Stopped, Deleted};

    ~ExtensionInfo();

    // information from the xml file, valid after 'Read' state is reached
    QString name() const;
    QString version() const;
    QString compatVersion() const;
    QString vendor() const;
    QString copyright() const;
    QString license() const;
    QString description() const;
    QString url() const;
    QString category() const;
    bool isExperimental() const;
    bool isEnabled() const;
    // true if loading was not done due to user unselecting this extension or its dependencies
    bool isDisabledByDependency() const;
    QList<ExtensionDependency> dependencies() const;

    typedef QList<ExtensionArgumentDescription> ExtensionArgumentDescriptions;
    ExtensionArgumentDescriptions argumentDescriptions() const;

    // other information, valid after 'Read' state is reached
    QString location() const;
    QString filePath() const;

    void setEnabled(bool value);

    QStringList arguments() const;
    void setArguments(const QStringList &arguments);
    void addArgument(const QString &argument);

    bool provides(const QString &extensionName, const QString &version) const;

    // dependency infos, valid after 'Resolved' state is reached
    QList<ExtensionInfo *> dependencyInfos() const;

    // list of extensions that depend on this - e.g. this extensions provides for them
    QList<ExtensionInfo *> providesInfos() const;

    // add/remove from providesInfos
    void addDependentExtension(ExtensionInfo *dependent);
    void removeDependentExtension(ExtensionInfo *dependent);

    // linked extension instance, valid after 'Loaded' state is reached
    IExtension *extension() const;

    // state
    State state() const;
    bool hasError() const;
    QString errorString() const;

private:
    ExtensionInfo();

    Internal::ExtensionInfoPrivate *d;
    friend class Internal::ExtensionManagerPrivate;
};

} // namespace ExtensionSystem

#endif // EXTENSIONINFO_H

