/*
 * InstanceAccessFeature.h
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

#ifndef UNISON_LV2_INSTANCE_ACCESS_FEATURE_H
#define UNISON_LV2_INSTANCE_ACCESS_FEATURE_H

#include "Feature.h"
#include "Lv2Plugin.h"

#include <lv2/instance-access.lv2/instance-access.h>

namespace Lv2 {
  namespace Internal {

class InstanceAccessFeature : public Feature
{
  public:
    InstanceAccessFeature () :
      Feature(LV2_INSTANCE_ACCESS_URI)
    {};

    LV2_Feature* lv2Feature ()
    {
      LV2_Feature* feat = new LV2_Feature();
      feat->URI = LV2_INSTANCE_ACCESS_URI;
      feat->data = NULL;
      return feat;
    };
      
    void initialize (LV2_Feature* feat, const Lv2Plugin& plugin) const
    {
      Q_ASSERT(uri() == feat->URI);
      feat->data = slv2_instance_get_handle( plugin.slv2Instance() );
    };

    void cleanup (LV2_Feature* feat) const
    {
      delete feat;
    };
};

  } // Internal
} // Lv2

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
