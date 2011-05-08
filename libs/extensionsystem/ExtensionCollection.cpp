#include "extensionsystem/ExtensionCollection.hpp"
#include "extensionsystem/ExtensionInfo.hpp"

namespace ExtensionSystem {

ExtensionCollection::ExtensionCollection(const QString& name) :
    m_name(name)
{

}

ExtensionCollection::~ExtensionCollection()
{

}

QString ExtensionCollection::name() const
{
    return m_name;
}

void ExtensionCollection::addExtension(ExtensionInfo *info)
{
    m_extensions.append(info);
}

void ExtensionCollection::removeExtension(ExtensionInfo *info)
{
    m_extensions.removeOne(info);
}

QList<ExtensionInfo *> ExtensionCollection::extensions() const
{
    return m_extensions;
}

}
