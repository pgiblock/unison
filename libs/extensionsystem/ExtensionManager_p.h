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

#ifndef EXTENSIONMANAGER_P_H
#define EXTENSIONMANAGER_P_H

#include "ExtensionInfo.h"

#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE
class QTime;
QT_END_NAMESPACE

namespace ExtensionSystem {

class ExtensionManager;
class ExtensionCollection;

namespace Internal {

class ExtensionInfoPrivate;

class EXTENSIONSYSTEM_EXPORT ExtensionManagerPrivate
{
public:
    ExtensionManagerPrivate(ExtensionManager *extensionManager);
    virtual ~ExtensionManagerPrivate();

    // Object pool operations
    void addObject(QObject *obj);
    void removeObject(QObject *obj);

    // Extension operations
    void loadExtensions();
    void setExtensionPaths(const QStringList &paths);
    QList<ExtensionInfo *> loadQueue();
    void loadExtension(ExtensionInfo *info, ExtensionInfo::State destState);
    void resolveDependencies();
    void initProfiling();
    void profilingReport(const char *what, const ExtensionInfo *info = 0);
    void loadSettings();
    void writeSettings();
    void removeExtensionInfo(ExtensionInfo *info);

    QHash<QString, ExtensionCollection *> extensionCategories;
    QList<ExtensionInfo *> extensionInfos;
    QList<ExtensionInfo *> testInfos;
    QStringList extensionPaths;
    QString extension;
    QList<QObject *> allObjects; // ### make this a QList<QPointer<QObject> > > ?
    QStringList disabledExtensions;
    QStringList forceEnabledExtensions;

    QStringList arguments;
    QScopedPointer<QTime> m_profileTimer;
    int m_profileElapsedMS;

    // Look in argument descriptions of the infos for the option.
    ExtensionInfo *extensionForOption(const QString &option, bool *requiresArgument) const;
    ExtensionInfo *extensionByName(const QString &name) const;

    // used by tests
    static ExtensionInfo *createInfo();
    static ExtensionInfoPrivate *privateInfo(ExtensionInfo *info);
private:
    ExtensionCollection *defaultCollection;
    ExtensionManager *q;

    void readExtensionPaths();
    bool loadQueue(ExtensionInfo *info,
            QList<ExtensionInfo *> &queue,
            QList<ExtensionInfo *> &circularityCheckQueue);
    void stopAll();
};

} // namespace Internal
} // namespace ExtensionSystem

#endif // EXTENSIONMANAGER_P_H
