#ifndef PLUGINCOLLECTION_H
#define PLUGINCOLLECTION_H

#include <QList>
#include <QString>
#include "extensionsystem_global.h"


namespace ExtensionSystem {
class PluginInfo;

class EXTENSIONSYSTEM_EXPORT PluginCollection
{

public:
    explicit PluginCollection(const QString& name);
    ~PluginCollection();

    QString name() const;
    void addPlugin(PluginInfo *info);
    void removePlugin(PluginInfo *info);
    QList<PluginInfo *> plugins() const;
private:
    QString m_name;
    QList<PluginInfo *> m_plugins;

};

}

#endif // PLUGINCOLLECTION_H
