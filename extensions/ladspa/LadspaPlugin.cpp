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
//#include "LadspaPort.h"

#include <unison/ProcessingContext.h>

#include <QDebug>
#include <QSet>

using namespace Unison;

namespace Ladspa {
  namespace Internal {

LadspaPlugin::LadspaPlugin (const LadspaDescriptor* descriptor,
                            nframes_t sampleRate) :
  Plugin(),
  m_descriptor(descriptor),
  m_sampleRate(sampleRate)
{
  init();
}


LadspaPlugin::LadspaPlugin (const LadspaPlugin& other) :
  Plugin(other),
  m_descriptor(other.m_descriptor),
  m_sampleRate(other.m_sampleRate)
{
  init();
}


void LadspaPlugin::init ()
{
  m_activated = false;
  m_handle = m_descriptor->instantiate(m_descriptor, m_sampleRate);
  Q_ASSERT(m_handle);

  qDebug() << "Initializing LadspaPlugin" << m_name << "with ports:";

  int count = portCount();
  m_ports.resize( count );
  for (int i = 0; i < count; ++i) {
    m_ports[i] = new LadspaPort( this, i );
    qDebug() << i << m_ports[i]->name();
  }

  qDebug() << "Instantiated LadspaPlugin:" << m_name;
}


LadspaPlugin::~LadspaPlugin () {
  deactivate();
  m_descriptor->cleanup(m_instance)
  for (int i=0; i<m_ports.count(); ++i) {
    delete m_ports[i];
  }

  m_handle = NULL;
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

    m_descriptor->activate(m_handle);
    m_activated = true;
  }
}


void LadspaPlugin::deactivate ()
{
  if (m_activated) {
    m_descriptor->deactivate(m_handle);
    m_activated = false;
  }
}


int LadspaPlugin::audioInputCount () const
{
  return TODO;
}


int LadspaPlugin::audioOutputCount () const
{
  return TODO;
}


QString LadspaPlugin::authorName () const
{
  return m_descriptor->Maker ? m_descriptor->Maker : "Unknown";
}


QString LadspaPlugin::authorEmail () const
{
  return NULL
}


QString LadspaPlugin::authorHomepage () const
{
  return NULL;
}


QString LadspaPlugin::copyright () const
{
  return m_descriptor->Copyright;
}


void LadspaPlugin::process (const ProcessingContext & context)
{
  m_descriptor->run(m_handle, context.bufferSize());
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


LadspaPluginDescriptor::LadspaPluginDescriptor (const QString &path, 
    const LADSPA_Descriptor *desc) :
  m_path(path),
  m_descriptor(desc)
{
  m_uniqueId = QString("%1").arg(desc->UniqueID);
  m_author = desc->Maker;
  m_name = desc->Name;

  for (int i=0; i < desc->PortCount; ++i) {
    LADSPA_PortDescriptor p = desc->PortDescriptors[i];
    if (LADSPA_IS_PORT_AUDIO(p)) {
      if (LADSPA_IS_PORT_INPUT(p)) {
        ++m_audioInputs;
      }
      else if (LADSPA_IS_PORT_OUTPUT(p)) {
        ++m_audioOutputs;
      }
    }
  }
}


LadspaPluginDescriptor::LadspaPluginDescriptor (const LadspaPluginDescriptor& d) :
  PluginDescriptor(d),
  m_path(d.m_path),
  m_descriptor(d.m_descriptor)
{}


PluginPtr LadspaPluginDescriptor::createPlugin (nframes_t sampleRate) const
{
  return PluginPtr(); // new LadspaPlugin( m_world, m_plugin, sampleRate ) );
}

  } // Internal
} // Ladspa

// vim: ts=8 sw=2 sts=2 et sta noai
