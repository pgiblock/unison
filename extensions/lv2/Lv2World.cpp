/*
 * Lv2World.cpp
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

#include "Lv2World.h"

// Features
#include "DataAccessFeature.h"
#include "InstanceAccessFeature.h"
#include "UriMapFeature.h"

#include <QDebug>

namespace Lv2 {
  namespace Internal {


Lv2World::Lv2World () :
  uriMap(),
  features()
{
  world = slv2_world_new();
  Q_ASSERT(world);
  slv2_world_load_all( world );

  // Hold on to these classes for performance
  inputClass =   slv2_value_new_uri( world, SLV2_PORT_CLASS_INPUT );
  outputClass =  slv2_value_new_uri( world, SLV2_PORT_CLASS_OUTPUT );
  controlClass = slv2_value_new_uri( world, SLV2_PORT_CLASS_CONTROL );
  audioClass =   slv2_value_new_uri( world, SLV2_PORT_CLASS_AUDIO );
  eventClass =   slv2_value_new_uri( world, SLV2_PORT_CLASS_EVENT );
  midiClass =    slv2_value_new_uri( world, SLV2_EVENT_CLASS_MIDI );
  integer =      slv2_value_new_uri( world, SLV2_NAMESPACE_LV2 "integer" );
  toggled =      slv2_value_new_uri( world, SLV2_NAMESPACE_LV2 "toggled" );
  sampleRate =   slv2_value_new_uri( world, SLV2_NAMESPACE_LV2 "sampleRate" );
  inPlaceBroken =slv2_value_new_uri( world, SLV2_NAMESPACE_LV2 "inPlaceBroken" );
  gtkGui =       slv2_value_new_uri( world, "http://lv2plug.in/ns/extensions/ui#GtkUI" );

  // Add the features
  features.insert( new DataAccessFeature() );
  features.insert( new InstanceAccessFeature() );
  features.insert( new UriMapFeature(&uriMap) );

  qDebug() << "Created Lv2World.";
}


Lv2World::~Lv2World ()
{
  slv2_value_free( inputClass );
  slv2_value_free( outputClass );
  slv2_value_free( controlClass );
  slv2_value_free( audioClass );
  slv2_value_free( eventClass );
  slv2_value_free( midiClass );
  slv2_value_free( inPlaceBroken );
  slv2_value_free( integer );
  slv2_value_free( toggled );
  slv2_value_free( sampleRate );
  slv2_value_free( gtkGui );

  slv2_world_free( world );

  foreach (Feature* feature, features.list()) {
    delete feature;
  }
}

  } // Internal
} // Lv2

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
