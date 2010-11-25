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

#ifndef EXTENSIONSYSTEM_EXTENSIONMANAGER_H
#define EXTENSIONSYSTEM_EXTENSIONMANAGER_H

#include "extensionsystem_global.hpp"
#include <aggregation/aggregate.hpp>

#include <QObject>
#include <QStringList>
#include <QReadWriteLock>

QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

namespace ExtensionSystem {
class ExtensionCollection;
namespace Internal {
    class ExtensionManagerPrivate;
}

class IExtension;
class ExtensionInfo;

class EXTENSIONSYSTEM_EXPORT ExtensionManager : public QObject
{
    Q_DISABLE_COPY(ExtensionManager)
    Q_OBJECT

public:
    static ExtensionManager *instance();

    ExtensionManager();
    virtual ~ExtensionManager();

    // Object pool operations
    void addObject(QObject *obj);
    void removeObject(QObject *obj);
    QList<QObject *> allObjects() const;
    template <typename T> QList<T *> getObjects() const
    {
        QReadLocker lock(&m_lock);
        QList<T *> results;
        QList<QObject *> all = allObjects();
        QList<T *> result;
        foreach (QObject *obj, all) {
            result = Aggregation::query_all<T>(obj);
            if (!result.isEmpty())
                results += result;
        }
        return results;
    }
    template <typename T> T *getObject() const
    {
        QReadLocker lock(&m_lock);
        QList<QObject *> all = allObjects();
        T *result = 0;
        foreach (QObject *obj, all) {
            if ((result = Aggregation::query<T>(obj)) != 0)
                break;
        }
        return result;
    }

    // Extension operations
    void loadExtensions();
    QStringList extensionPaths() const;
    void setExtensionPaths(const QStringList &paths);
    QList<ExtensionInfo *> extensions() const;
    QHash<QString, ExtensionCollection *> extensionCollections() const;
    void setFileExtension(const QString &extension);
    QString fileExtension() const;

    // Settings
    void loadSettings();
    void writeSettings();

    // command line arguments
    QStringList arguments() const;
    bool parseOptions(const QStringList &args,
        const QMap<QString, bool> &appOptions,
        QMap<QString, QString> *foundAppOptions,
        QString *errorString);
    static void formatOptions(QTextStream &str, int optionIndentation, int descriptionIndentation);
    void formatExtensionOptions(QTextStream &str, int optionIndentation, int descriptionIndentation) const;
    void formatExtensionVersions(QTextStream &str) const;

    QString serializedArguments() const;

    bool runningTests() const;
    QString testDataDirectory() const;

signals:
    void objectAdded(QObject *obj);
    void aboutToRemoveObject(QObject *obj);

    void extensionsChanged();

public slots:
    void remoteArguments(const QString &serializedArguments);

private slots:
    void startTests();

private:
    Internal::ExtensionManagerPrivate *d;
    static ExtensionManager *m_instance;
    mutable QReadWriteLock m_lock;

    friend class Internal::ExtensionManagerPrivate;
};

} // namespace ExtensionSystem

#endif // EXTENSIONSYSTEM_EXTENSIONMANAGER_H
