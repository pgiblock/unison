/*
 * LadspaPlugin.cpp
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

#include "LadspaPlugin.h"
#include "LadspaPort.h"

#include <unison/ProcessingContext.h>

#include <QDebug>
#include <QSet>

using namespace Unison;

namespace Ladspa {
  namespace Internal {

LadspaPlugin::LadspaPlugin (LadspaWorld& world, SLV2Plugin plugin,
                      nframes_t sampleRate) :
  Plugin(),
  m_world(world),
  m_plugin(plugin),
  m_sampleRate(sampleRate)
{
  init();
}


LadspaPlugin::LadspaPlugin (const LadspaPlugin& other) :
  Plugin(other),
  m_world(other.m_world),
  m_plugin(other.m_plugin),
  m_sampleRate(other.m_sampleRate)
{
  init();
}


void LadspaPlugin::init ()
{
  m_activated = false;
  m_instance = slv2_plugin_instantiate( m_plugin, m_sampleRate, NULL );
  Q_ASSERT(m_instance);

  m_name = slv2_plugin_get_name( m_plugin );
  Q_ASSERT(m_name);

  qDebug() << "Initializing LadspaPlugin" << m_name << "with ports:";

  int count = portCount();
  m_ports.resize( count );
  for (int i = 0; i < count; ++i) {
    m_ports[i] = new LadspaPort( m_world, this, i );
    qDebug() << i << m_ports[i]->name();
  }


  m_authorName     = slv2_plugin_get_author_name( m_plugin );
  m_authorEmail    = slv2_plugin_get_author_email( m_plugin );
  m_authorHomepage = slv2_plugin_get_author_homepage( m_plugin );

  qDebug() << "Instantiated LadspaPlugin:" << m_name;
}


LadspaPlugin::~LadspaPlugin () {
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


Port* LadspaPlugin::port (int idx) const
{
  return m_ports[idx];
}


Port* LadspaPlugin::port (QString id) const
{
  for (int i=0; i<m_ports.count(); ++i) {
    if (m_ports[i]->id() == id) {
      return m_ports[i];
    }
  }

  return NULL;
}


BufferProvider *LadspaPlugin::bufferProvider ()
{
  return m_bufferProvider;
}


void LadspaPlugin::activate (BufferProvider *bp)
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


void LadspaPlugin::deactivate ()
{
  if (m_activated) {
    slv2_instance_deactivate( m_instance );
    m_activated = false;
  }
}


int LadspaPlugin::audioInputCount () const
{
  return slv2_plugin_get_num_ports_of_class(
      m_plugin, m_world.inputClass, m_world.audioClass, NULL );
}


int LadspaPlugin::audioOutputCount () const
{
  return slv2_plugin_get_num_ports_of_class(
      m_plugin, m_world.outputClass, m_world.audioClass, NULL );
}


QString LadspaPlugin::authorName () const
{
  return m_authorName ? slv2_value_as_string( m_authorName ) : "Unknown";
}


QString LadspaPlugin::authorEmail () const
{
  return m_authorEmail ? slv2_value_as_string( m_authorEmail ) : NULL;
}


QString LadspaPlugin::authorHomepage () const
{
  return m_authorHomepage ? slv2_value_as_string( m_authorHomepage ) : NULL;
}


QString LadspaPlugin::copyright () const
{
  return m_copyright ? slv2_value_as_string( m_copyright ) : NULL;
}


void LadspaPlugin::process (const ProcessingContext & context)
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


const QSet<Node* const> LadspaPlugin::dependencies () const
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


const QSet<Node* const> LadspaPlugin::dependents () const {
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



LadspaPluginDescriptor::LadspaPluginDescriptor (LadspaWorld& world, SLV2Plugin plugin) :
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


LadspaPluginDescriptor::LadspaPluginDescriptor (const LadspaPluginDescriptor& d) :
  PluginDescriptor(d),
  m_world(d.m_world),
  m_plugin(d.m_plugin)
{}


PluginPtr LadspaPluginDescriptor::createPlugin (nframes_t sampleRate) const
{
  return PluginPtr( new LadspaPlugin( m_world, m_plugin, sampleRate ) );
}

  } // Internal
} // Ladspa

// vim: ts=8 sw=2 sts=2 et sta noai
