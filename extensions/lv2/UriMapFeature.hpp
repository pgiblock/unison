/*
 * UriMapFeature.hpp
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

#ifndef UNISON_LV2_URI_MAP_FEATURE_H
#define UNISON_LV2_URI_MAP_FEATURE_H

#include "Feature.hpp"

#include <lv2/uri-map.lv2/uri-map.h>

namespace Lv2 {
  namespace Internal {

class UriMap;

class UriMapFeature : public Feature
{
  public:
    UriMapFeature (UriMap* uriMap);

    LV2_Feature* lv2Feature ();
    void initialize (LV2_Feature*, const Lv2Plugin&) const {};
    void cleanup (LV2_Feature*) const {};

  private:
    static uint32_t uriToId (LV2_URI_Map_Callback_Data cbData, const char* map, const char* uri);

    LV2_Feature m_feature;
    LV2_URI_Map_Feature m_data;

    UriMap* m_uriMap;
};

  } // Internal
} // Lv2

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
