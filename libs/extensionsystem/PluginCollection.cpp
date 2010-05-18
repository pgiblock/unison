#include "PluginCollection.h"
#include "PluginInfo.h"

namespace ExtensionSystem {

PluginCollection::PluginCollection(const QString& name) :
    m_name(name)
{

}

PluginCollection::~PluginCollection()
{

}

QString PluginCollection::name() const
{
    return m_name;
}

void PluginCollection::addPlugin(PluginInfo *info)
{
    m_plugins.append(info);
}

void PluginCollection::removePlugin(PluginInfo *info)
{
    m_plugins.removeOne(info);
}

QList<PluginInfo *> PluginCollection::plugins() const
{
    return m_plugins;
}

}
