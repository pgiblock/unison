/*
 * Lv2Plugin.cpp
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

#include "Lv2Plugin.h"
#include "Lv2Port.h"

#include <unison/ProcessingContext.h>

#include <QDebug>
#include <QSet>

using namespace Unison;

namespace Lv2 {
  namespace Internal {

Lv2World::Lv2World ()
{
  world=slv2_world_new();
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
  inPlaceBroken =
      slv2_value_new_uri( world, SLV2_NAMESPACE_LV2 "inPlaceBroken" );
  gtkGui =
      slv2_value_new_uri( world, "http://lv2plug.in/ns/extensions/ui#GtkUI" );

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
}


Lv2Plugin::Lv2Plugin (Lv2World& world, SLV2Plugin plugin,
                      nframes_t sampleRate) :
  Plugin(),
  m_world(world),
  m_plugin(plugin),
  m_sampleRate(sampleRate)
{
  init();
}


Lv2Plugin::Lv2Plugin (const Lv2Plugin& other) :
  Plugin(other),
  m_world(other.m_world),
  m_plugin(other.m_plugin),
  m_sampleRate(other.m_sampleRate)
{
  init();
}


void Lv2Plugin::init ()
{
  m_activated = false;
  m_instance = slv2_plugin_instantiate( m_plugin, m_sampleRate, NULL );
  Q_ASSERT(m_instance);

  m_name = slv2_plugin_get_name( m_plugin );
  Q_ASSERT(m_name);

  qDebug() << "Initializing Lv2Plugin" << m_name << "with ports:";

  int count = portCount();
  m_ports.resize( count );
  for (int i = 0; i < count; ++i) {
    m_ports[i] = new Lv2Port( m_world, this, i );
    qDebug() << i << m_ports[i]->name();
  }


  m_authorName     = slv2_plugin_get_author_name( m_plugin );
  m_authorEmail    = slv2_plugin_get_author_email( m_plugin );
  m_authorHomepage = slv2_plugin_get_author_homepage( m_plugin );

  qDebug() << "Instantiated Lv2Plugin:" << m_name;
}


Lv2Plugin::~Lv2Plugin () {
  deactivate();
  slv2_instance_free( m_instance );
  slv2_value_free( m_name );
  slv2_value_free( m_authorName );
  slv2_value_free( m_authorEmail );
  slv2_value_free( m_authorHomepage );
  for (int i=0; i<m_ports.count(); ++i) {
    delete m_ports[i];
  }

  m_instance = NULL;
}


Port* Lv2Plugin::port (int idx) const
{
  return m_ports[idx];
}


Port* Lv2Plugin::port (QString id) const
{
  for (int i=0; i<m_ports.count(); ++i) {
    if (m_ports[i]->id() == id) {
      return m_ports[i];
    }
  }

  return NULL;
}


BufferProvider *Lv2Plugin::bufferProvider ()
{
  return m_bufferProvider;
}


void Lv2Plugin::activate (BufferProvider *bp)
{
  if (!m_activated) {
    qDebug() << "Activating plugin" << name();
    m_bufferProvider = bp;

    // Connect all ports first
    for (int i=0; i<m_ports.count(); ++i) {
      m_ports[i]->connectToBuffer();
    }

    slv2_instance_activate( m_instance );
    m_activated = true;
  }
}


void Lv2Plugin::deactivate ()
{
  if (m_activated) {
    slv2_instance_deactivate( m_instance );
    m_activated = false;
  }
}


int Lv2Plugin::audioInputCount () const
{
  return slv2_plugin_get_num_ports_of_class(
      m_plugin, m_world.inputClass, m_world.audioClass, NULL );
}


int Lv2Plugin::audioOutputCount () const
{
  return slv2_plugin_get_num_ports_of_class(
      m_plugin, m_world.outputClass, m_world.audioClass, NULL );
}


QString Lv2Plugin::authorName () const
{
  return m_authorName ? slv2_value_as_string( m_authorName ) : "Unknown";
}


QString Lv2Plugin::authorEmail () const
{
  return m_authorEmail ? slv2_value_as_string( m_authorEmail ) : NULL;
}


QString Lv2Plugin::authorHomepage () const
{
  return m_authorHomepage ? slv2_value_as_string( m_authorHomepage ) : NULL;
}


QString Lv2Plugin::copyright () const
{
  return m_copyright ? slv2_value_as_string( m_copyright ) : NULL;
}


void Lv2Plugin::process (const ProcessingContext & context)
{
  int count = portCount();
  for (int i=0; i<count; ++i) {
    Port* p  = port(i);
    if (p->direction() == INPUT && p->type() == CONTROL_PORT) {
//      qDebug() << "Control Port" << p->name() << "has value" << ((float*)(p->buffer()->data()))[0] ;
    }
  }
  slv2_instance_run(m_instance, context.bufferSize());
}


const QSet<Node* const> Lv2Plugin::dependencies () const
{
  QSet<Node* const> n;
  int count = portCount();
  for (int i=0; i<count; ++i) {
    Port* p  = port(i);
    if (p->direction() == INPUT) {
      n += p;
    }
  }
  return n;
}


const QSet<Node* const> Lv2Plugin::dependents () const {
  QSet<Node* const> n;
  int count = portCount();
  for (int i=0; i<count; ++i) {
    Port* p  = port(i);
    if (p->direction() == OUTPUT) {
      n += p;
    }
  }
  return n;
}



Lv2PluginDescriptor::Lv2PluginDescriptor (Lv2World& world, SLV2Plugin plugin) :
  m_world(world),
  m_plugin(plugin)
{

  SLV2Value data;

  m_uniqueId = QString( slv2_value_as_uri( slv2_plugin_get_uri( plugin ) ) );

  data = slv2_plugin_get_name( plugin );
  m_name = QString( slv2_value_as_string( data ) );
  slv2_value_free( data );

  data = slv2_plugin_get_author_name( plugin );
  if (data) {
    m_author = QString( slv2_value_as_string( data ) );
    slv2_value_free( data );
  }

  m_audioInputs = slv2_plugin_get_num_ports_of_class( plugin,
          world.inputClass, world.audioClass, NULL );

  m_audioOutputs = slv2_plugin_get_num_ports_of_class( plugin,
          world.outputClass, world.audioClass, NULL );

  // TODO: Are 'types' needed? if so, move to non-virtual Plugin function
  if (m_audioInputs > 0) {
    if (m_audioOutputs > 0) {
      m_type = TRANSFER;
    }
    else {
      m_type = SINK;
    }
  }
  else if (m_audioOutputs > 0) {
    m_type = SOURCE;
  }
  else {
    m_type = OTHER;
  }
}


Lv2PluginDescriptor::Lv2PluginDescriptor (const Lv2PluginDescriptor& d) :
  PluginDescriptor(d),
  m_world(d.m_world),
  m_plugin(d.m_plugin)
{}


PluginPtr Lv2PluginDescriptor::createPlugin (nframes_t sampleRate) const
{
  return PluginPtr( new Lv2Plugin( m_world, m_plugin, sampleRate ) );
}

  } // Internal
} // Lv2

// vim: ts=8 sw=2 sts=2 et sta noai
