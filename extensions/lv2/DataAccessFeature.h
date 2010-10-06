/*
 * DataAccessFeature.h
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

#ifndef UNISON_LV2_DATA_ACCESS_FEATURE_H
#define UNISON_LV2_DATA_ACCESS_FEATURE_H

#include "Feature.h"
#include "Lv2Plugin.h"

#include <lv2/data-access.lv2/data-access.h>

namespace Lv2 {
  namespace Internal {

class DataAccessFeature : public Feature
{
  public:
    DataAccessFeature () :
      Feature(LV2_DATA_ACCESS_URI, UI_FEATURE)
    {};

    LV2_Feature* lv2Feature ()
    {
      LV2_Extension_Data_Feature* data = new LV2_Extension_Data_Feature();
      data->data_access = NULL;

      LV2_Feature* feat = new LV2_Feature();
      feat->URI = LV2_DATA_ACCESS_URI;
      feat->data = data;
      return feat;
    };
      
    void initialize (LV2_Feature* feat, const Lv2Plugin& plugin) const
    {
      Q_ASSERT(uri() == feat->URI);
      const LV2_Descriptor* desc =
          slv2_instance_get_descriptor( plugin.slv2Instance() );
      LV2_Extension_Data_Feature* data =
          static_cast<LV2_Extension_Data_Feature*>( feat->data );

      data->data_access = desc->extension_data;
    };

    void cleanup (LV2_Feature* feat) const
    {
      if (feat->data) {
        delete static_cast<LV2_Extension_Data_Feature*>( feat->data );
      }
      delete feat;
    };
};

  } // Internal
} // Lv2

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
