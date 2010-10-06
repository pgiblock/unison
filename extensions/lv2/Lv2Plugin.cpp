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

Lv2Plugin::Lv2Plugin (Lv2World& world, SLV2Plugin plugin, nframes_t sampleRate) :
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
  m_features = m_world.features.array();
  m_instance = slv2_plugin_instantiate(
      m_plugin, 
      m_sampleRate,
      m_features->get(Feature::PLUGIN_FEATURE) );
  
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

  m_features->initialize( *this );

  qDebug() << "Instantiated Lv2Plugin:" << m_name;
}


Lv2Plugin::~Lv2Plugin () {
  deactivate();
  slv2_instance_free( m_instance );
  slv2_value_free( m_name );
  slv2_value_free( m_authorName );
  slv2_value_free( m_authorEmail );
  slv2_value_free( m_authorHomepage );
  for (int i=0; i < m_ports.count(); ++i) {
    delete m_ports[i];
  }

  m_instance = NULL;
}


Port* Lv2Plugin::port (int idx) const
{
  return m_ports[idx];
}


Port* Lv2Plugin::port (const QString& id) const
{
  for (int i=0; i<m_ports.count(); ++i) {
    if (m_ports[i]->id() == id) {
      return m_ports[i];
    }
  }

  return NULL;
}


BufferProvider* Lv2Plugin::bufferProvider ()
{
  return m_bufferProvider;
}


void Lv2Plugin::activate (BufferProvider* bp)
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


void Lv2Plugin::process (const ProcessingContext& context)
{
  int count = portCount();
  for (int i=0; i<count; ++i) {
    Port* p  = port(i);
    if (p->direction() == Input && p->type() == ControlPort) {
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
    if (p->direction() == Input) {
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
    if (p->direction() == Output) {
      n += p;
    }
  }
  return n;
}


  } // Internal
} // Lv2

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
