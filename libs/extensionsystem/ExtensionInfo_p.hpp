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

#ifndef EXTENSIONINFO_P_H
#define EXTENSIONINFO_P_H

#include "extensionsystem/ExtensionInfo.hpp"

#include <QObject>
#include <QStringList>
#include <QXmlStreamReader>

namespace ExtensionSystem {

class IExtension;
class ExtensionManager;

namespace Internal {

class EXTENSIONSYSTEM_EXPORT ExtensionInfoPrivate : public QObject
{
    Q_OBJECT

public:
    ExtensionInfoPrivate(ExtensionInfo *info);

    bool read(const QString &fileName);
    bool provides(const QString &extensionName, const QString &version) const;
    bool resolveDependencies(const QList<ExtensionInfo *> &infos);
    bool loadLibrary();
    bool initializeExtension();
    bool initializeExtensions();
    void stop();
    void kill();

    QString name;
    QString version;
    QString compatVersion;
    bool experimental;
    QString vendor;
    QString copyright;
    QString license;
    QString description;
    QString url;
    QString category;
    QList<ExtensionDependency> dependencies;
    bool enabled;
    bool disabledByDependency;

    QString location;
    QString filePath;
    QStringList arguments;

    QList<ExtensionInfo *> providesInfos;
    QList<ExtensionInfo *> dependencyInfos;
    ExtensionInfo::ExtensionArgumentDescriptions argumentDescriptions;
    IExtension *extension;

    ExtensionInfo::State state;
    bool hasError;
    QString errorString;

    static bool isValidVersion(const QString &version);
    static int versionCompare(const QString &version1, const QString &version2);

private:
    ExtensionInfo *q;

    bool reportError(const QString &err);
    void readExtensionInfo(QXmlStreamReader &reader);
    void readDependencies(QXmlStreamReader &reader);
    void readDependencyEntry(QXmlStreamReader &reader);
    void readArgumentDescriptions(QXmlStreamReader &reader);
    void readArgumentDescription(QXmlStreamReader &reader);

    static QRegExp &versionRegExp();
};

} // namespace Internal
} // namespace ExtensionSystem

#endif // EXTENSIONINFO_P_H
