/*
 * UriMapFeature.cpp
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

#include "UriMapFeature.h"
#include "UriMap.h"

#include <QtDebug>

namespace Lv2 {
  namespace Internal {


UriMapFeature::UriMapFeature (UriMap* uriMap) :
  Feature(LV2_URI_MAP_URI, PLUGIN_FEATURE),
  m_uriMap(uriMap)
{
  m_feature.URI = LV2_URI_MAP_URI;
  m_feature.data = &m_data;

  m_data.callback_data = m_uriMap;
  m_data.uri_to_id = &uriToId;
}


LV2_Feature* UriMapFeature::lv2Feature ()
{
  return &m_feature;
}


uint32_t UriMapFeature::uriToId (LV2_URI_Map_Callback_Data cbData, const char* map, const char* uri)
{
  UriMap* uriMap = static_cast<UriMap*>( cbData );
  if (map != NULL) {
    qWarning() << "LV2::UriMapFeature: Ignoring non-null map" << map;
  }
  return uriMap->uriToId(uri);
}

  } // Internal
} // Lv2

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
