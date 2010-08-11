/*
 * LadspaPort.h
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

#include "LadspaPort.h"

#include <unison/BufferProvider.h>

#include <QSet>

using namespace Unison;

namespace Ladspa {
  namespace Internal {

#define UNISON_BUFFER_LENGTH 1024

LadspaPort::LadspaPort (const LadspaWorld& world, LadspaPlugin* plugin, uint32_t index) :
  Port(),
  m_world(world),
  m_plugin(plugin),
  m_index(index),
  m_value(0),
  m_defaultValue(0),
  m_min(0),
  m_max(0),
  m_isSampleRate(false)
{
  // TODO: Error handling
  m_port = slv2_plugin_get_port_by_index( plugin->slv2Plugin(), index );

  // set range and default
  SLV2Value def, min, max;
  slv2_port_get_range( plugin->slv2Plugin(), m_port, &def , &min, &max );
  if (def) {
    m_defaultValue = slv2_value_as_float( def );
    m_value = m_defaultValue;
    slv2_value_free( def );
  }
  if (min) {
    m_min   = slv2_value_as_float( min );
    slv2_value_free( min );
  }
  if (max) {
    m_max   = slv2_value_as_float( max );
    slv2_value_free( max );
  }
  m_isSampleRate = slv2_port_has_property( plugin->slv2Plugin(), m_port, m_world.sampleRate );
}


LadspaPort::~LadspaPort ()
{
  // I don't believe SLV2 requires us to free the port.
}


PortType LadspaPort::type () const
{
  SLV2Plugin slv2Plugin = m_plugin->slv2Plugin();
  if (slv2_port_is_a( slv2Plugin, m_port, m_world.controlClass )) {
    return CONTROL_PORT;
  }
  else if (slv2_port_is_a( slv2Plugin, m_port, m_world.audioClass )) {
    return AUDIO_PORT;
  }
  else if (slv2_port_is_a( slv2Plugin, m_port, m_world.midiClass )) {
    return MIDI_PORT;
  }
  else {
    return UNKNOWN_PORT;
  }
}


PortDirection LadspaPort::direction () const
{
  SLV2Plugin plugin = m_plugin->slv2Plugin();
  if (slv2_port_is_a( plugin, m_port, m_world.inputClass )) {
    return INPUT;
  }
  if (slv2_port_is_a( plugin, m_port, m_world.outputClass )) {
    return OUTPUT;
  }
  // TODO: Maybe have an UNDEFINED direction?
  qFatal("Port `%s' is neither input or output", qPrintable(name()));
}


QString LadspaPort::name () const
{
  SLV2Value val = slv2_port_get_name( m_plugin->slv2Plugin(), m_port );
  QString ret = QString::fromAscii( slv2_value_as_string(val) );
  slv2_value_free( val );
  return ret;
}


QString LadspaPort::id () const
{
  SLV2Value val = slv2_port_get_symbol( m_plugin->slv2Plugin(), m_port );
  QString ret = QString::fromAscii( slv2_value_as_string(val) );
  return ret;
}


float LadspaPort::value () const
{
  return m_value;
}


void LadspaPort::setValue (float value)
{
  m_value = value;
  updateBufferValue();
}


Node* LadspaPort::parent () const
{
  return m_plugin;
}


const QSet<Node* const> LadspaPort::interfacedNodes () const
{
  QSet<Node* const> p;
  p.insert( m_plugin );
  return p;
}


void LadspaPort::connectToBuffer ()
{
  BufferProvider *provider = m_plugin->bufferProvider();
  switch (direction()) {
    case INPUT:
      acquireInputBuffer(*provider, UNISON_BUFFER_LENGTH);
      break;

    case OUTPUT:
      acquireOutputBuffer(*provider, UNISON_BUFFER_LENGTH);
      break;
  }
  slv2_instance_connect_port (m_plugin->slv2Instance(), m_index, buffer()->data());
}


  } // Internal
} // Ladspa

// vim: ts=8 sw=2 sts=2 et sta noai
