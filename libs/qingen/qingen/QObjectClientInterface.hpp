/*
 * QObjectClientInterface.hpp
 *
 * Copyright (c) 2011 Paul Giblock <pgib/at/users.sourceforge.net>
 *
 * This file is part of Unison - http://unison.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#ifndef QINGEN_QOBJECTCLIENTINTERFACE_HPP
#define QINGEN_QOBJECTCLIENTINTERFACE_HPP

#include <QObject>
#include <inttypes.h>
#include "raul/Path.hpp"
#include "ingen/ClientInterface.hpp"

namespace QIngen {

/**
 * A QObject-based interface for Qt clients to use.
 *
 * The signals here correlated to class on Ingen::ClientInterface, but have
 * been renamed slightly to match Qt naming suggestions.
 */
class QObjectClientInterface : public QObject, public Ingen::ClientInterface
{
  Q_OBJECT

public:
  QObjectClientInterface() {}

  Raul::URI uri() const { return "http://TODO.com/ns/qingen#client"; }

signals:
  void responseOk(int32_t id);
  void responseError(int32_t id, std::string msg);
  void engineError(const std::string msg);

  void bundleBegin();
  void bundleEnd();

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

protected:

    // ClientInterface hooks that fire the above signals
    void bundle_begin()
    { emit bundleBegin(); }

    void bundle_end()
    { emit bundleEnd(); }

    void transfer_begin() {}
    void transfer_end()   {}

    void response_ok(int32_t id)
    { emit responseOk(id); }

    void response_error(int32_t id, const std::string& msg)
    { emit responseError(id, msg); }

    void error(const std::string& msg)
    { emit engineError(msg); }

    void put(const Raul::URI& uri,
             const Ingen::Resource::Properties& properties,
             Ingen::Resource::Graph ctx=Ingen::Resource::DEFAULT)
    { emit propertyPut(uri, properties, ctx); }

    void delta(const Raul::URI& uri,
               const Ingen::Resource::Properties& remove,
               const Ingen::Resource::Properties& add)
    { emit propertyDelta(uri, remove, add); }

    void connect(const Raul::Path& src_port_path, const Raul::Path& dst_port_path)
    { emit connected(src_port_path, dst_port_path); }

    void del(const Raul::URI& uri)
    { emit objectDeleted(uri); }

    void move(const Raul::Path& old_path, const Raul::Path& new_path)
    { emit objectMoved(old_path, new_path); }

    void disconnect(const Raul::URI& src, const Raul::URI& dst)
    { emit disconnected(src, dst); }

    void disconnect_all(const Raul::Path& parent_patch_path, const Raul::Path& path)
    { emit disconnectedAll(parent_patch_path, path); }

    void set_property(const Raul::URI& subject, const Raul::URI& key, const Raul::Atom& value)
    { emit propertyChanged(subject, key, value); }

    void activity(const Raul::Path& port_path)
    { emit portActivity(port_path); }
  };

} // namespace QIngen

#endif // QINGEN_QOBJECTCLIENTINTERFACE_HPP

// vim: tw=90 ts=8 sw=2 sts=4 et ci pi cin cino=l1,g0,+2s,\:0,(0,u0,U0,W2s
