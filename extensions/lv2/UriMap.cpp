/*
 * UriMap.h
 *
 * Copyright (c) 2010 Paul Giblock <pgib/at/users.sourceforge.net>
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

#include "UriMap.hpp"

#include <QMutexLocker>

namespace Lv2 {
  namespace Internal {

uint32_t UriMap::uriToId (const QString& uri)
{
    QMutexLocker locker(&m_lock);

    uint32_t id = m_mappedIds.value(uri, 0);
    if (id) {
        return id;
    }
  
    // Skip 0, since we use it to test, and it sucks as an Id anyways
    id = m_mappedIds.count() + 1;
    m_mappedIds.insert(uri, id);

    return id;
}


QString UriMap::idToUri (uint32_t id)
{
    QMutexLocker locker(&m_lock);
    return m_mappedIds.key(id);
}


bool UriMap::hasUri (const QString& uri) const
{
    QMutexLocker locker(&m_lock);
    return m_mappedIds.contains(uri);
}

  } // Internal
} // Lv2

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
