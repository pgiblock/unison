#ifndef QINGEN_STORE_HPP
#define QINGEN_STORE_HPP

#include "raul/Path.hpp"

namespace Raul { class Atom; }

namespace Ingen { class GraphObject; }

namespace QIngen {

class Node;
class Object;
class Patch;
class Plugin;
class Port;
class QObjectClientInterface;


/** Automatically manages models of objects in the engine.
 *
 * \ingroup IngenClient
 */
class Store : public QObject
{
public:
  Store(
//        SharedPtr<Shared::LV2URIMap>  uris,
//        SharedPtr<ServerInterface>    engine=SharedPtr<ServerInterface>(),
  QSharedPointer<QObjectClientInterface> emitter);

//  SharedPtr<PluginModel>  plugin(const Raul::URI& uri);
//  SharedPtr<ObjectModel>  object(const Raul::Path& path);
//  SharedPtr<Resource>     resource(const Raul::URI& uri);

  void clear();

//  typedef Raul::Table<Raul::URI, SharedPtr<PluginModel> > Plugins;
//  SharedPtr<const Plugins> plugins() const                   { return _plugins; }
//  SharedPtr<Plugins>       plugins()                         { return _plugins; }
//  void                     setPlugins(SharedPtr<Plugins> p) { _plugins = p; }

//  Shared::LV2URIMap& uris() { return *_uris.get(); }

signals:
  void newObject(QSharedPointer<GraphObject>);
//      void newPlugin(SharedPtr<PluginModel>);

protected slots:
  void propertyPut(Raul::URI                        subject,
                   Ingen::Resource::Properties      properties,
                   Ingen::Resource::Graph           context);

  void propertyDelta(Raul::URI                      subject,
                     Ingen::Resource::Properties    remove,
                     Ingen::Resource::Properties    add);

  void objectMoved(Raul::Path oldPath,
                   Raul::Path newPath);

  void objectDeleted(Raul::URI uri);

  void connected(Raul::Path srcPortPath,
                 Raul::Path dstPortPath);

  void disconnected(Raul::URI srcPort,
                    Raul::URI dstPort);

  void disconnectedAll(Raul::Path parentPatchPath,
                       Raul::Path path);

  void propertyChanged(Raul::URI  subject,
                       Raul::URI  key,
                       Raul::Atom value);

  void portActivity(Raul::Path portPath);

private:
  void add(GraphObject* o) { throw; }

  void add_object(SharedPtr<ObjectModel> object);
  SharedPtr<ObjectModel> remove_object(const Raul::Path& path);

  void add_plugin(SharedPtr<PluginModel> plugin);

  SharedPtr<PatchModel> connection_patch(const Raul::Path& src_port_path,
                                         const Raul::Path& dst_port_path);

  bool attempt_connection(const Raul::Path& src_port_path,
                          const Raul::Path& dst_port_path);

//  SharedPtr<Shared::LV2URIMap>  _uris;
//  SharedPtr<ServerInterface>    _engine;
  QSharedPointer<SigClientInterface> m_emitter;

//  SharedPtr<Plugins> _plugins; ///< Map, keyed by plugin URI
};

} // namespace QIngen

#endif // QINGEN_STORE_HPP

// vim: tw=90 ts=8 sw=2 sts=4 et ci pi cin cino=l1,g0,+2s,\:0,(0,u0,U0,W2s
