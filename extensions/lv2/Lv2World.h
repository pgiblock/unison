/*
 * Lv2World.h
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

#ifndef UNISON_LV2_WORLD_H
#define UNISON_LV2_WORLD_H

#include "Feature.h"
#include "UriMap.h"

#include <slv2/slv2.h>

namespace Lv2 {
  namespace Internal {

/** The SLV2World, and various cached (as symbols, fast) URIs.
 *  This object represents everything Unison 'knows' about LV2
 *  (ie understood extensions/features/etc) */
struct Lv2World
{
  public:
  Lv2World ();
  ~Lv2World ();

  SLV2World world;         ///< The SLV2World itself

  SLV2Value inputClass;    ///< Input port
  SLV2Value outputClass;   ///< Output port

  SLV2Value audioClass;    ///< Audio port
  SLV2Value controlClass;  ///< Control port
  SLV2Value eventClass;    ///< Event port
  SLV2Value midiClass;     ///< MIDI event

  SLV2Value inPlaceBroken; ///< Plugin requires seperate buffers
  SLV2Value integer;       ///< Integer restrictions for control ports
  SLV2Value toggled;       ///< Boolean restriction for control ports
  SLV2Value sampleRate;    ///< Port values are multiplied by sampling rate
  SLV2Value gtkGui;        ///< GTK-based gui is available

  UriMap    uriMap;        ///< UriMap used by host and plugins
  FeatureSet features;     ///< Feature storage and array generation
};

  } // Internal
} // Lv2

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
