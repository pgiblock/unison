#ifndef EXTENSIONCOLLECTION_H
#define EXTENSIONCOLLECTION_H

#include <QList>
#include <QString>
#include "extensionsystem/extensionsystem_global.hpp"


namespace ExtensionSystem {
class ExtensionInfo;

class EXTENSIONSYSTEM_EXPORT ExtensionCollection
{

public:
    explicit ExtensionCollection(const QString& name);
    ~ExtensionCollection();

    QString name() const;
    void addExtension(ExtensionInfo *info);
    void removeExtension(ExtensionInfo *info);
    QList<ExtensionInfo *> extensions() const;
private:
    QString m_name;
    QList<ExtensionInfo *> m_extensions;

};

}

#endif // EXTENSIONCOLLECTION_H
