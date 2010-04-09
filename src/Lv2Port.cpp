/*
 * Lv2Port.h
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

#include <QSet>
#include "unison/Lv2Port.h"
#include "unison/BufferProvider.h"

namespace Unison {

Lv2Port::Lv2Port (const Lv2World & world, Lv2Plugin * plugin, uint32_t index) :
  Port(),
  m_world(world),
  m_plugin(plugin),
  m_index(index),
  m_value(0),
  m_defaultValue(0),
  m_min(0),
  m_max(0)
{
  // TODO: Error handling
  m_port = slv2_plugin_get_port_by_index( plugin->slv2Plugin(), index );

  // set range and default
  SLV2Value def, min, max;
  slv2_port_get_range( plugin->slv2Plugin(), m_port, &def , &min, &max );
  if (def) {
    m_defaultValue = slv2_value_as_float( def );
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
}


Lv2Port::~Lv2Port ()
{
  // Is the LVPort just a handle to the Plugin?
}


PortType Lv2Port::type () const
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

PortDirection Lv2Port::direction () const
{
  SLV2Plugin plugin = m_plugin->slv2Plugin();
  if (slv2_port_is_a( plugin, m_port, m_world.inputClass )) {
    return INPUT;
  }
  if (slv2_port_is_a( plugin, m_port, m_world.outputClass )) {
    return OUTPUT;
  }
  // TODO: Fail.
}


QString Lv2Port::name () const
{
  SLV2Value val = slv2_port_get_name( m_plugin->slv2Plugin(), m_port );
  QString ret = QString::fromAscii( slv2_value_as_string(val) );
  slv2_value_free( val );
  return ret;
}



const QSet<Node* const> Lv2Port::interfacedNodes () const {
	QSet<Node* const> p;
	p.insert( m_plugin );
	return p;
}


void Lv2Port::connectToBuffer() {
	slv2_instance_connect_port (m_plugin->slv2Instance(), m_index, buffer()->data());
}


void Lv2Port::acquireBuffer (BufferProvider & provider)
{
  int numConnections;
  switch (direction()) {
    case INPUT:
      numConnections = dependencies().count();
      if (type() == AUDIO_PORT && numConnections == 0) {
        // Use silence
        m_buffer = provider.zeroAudioBuffer();
      }
      else if (numConnections == 1) {
        // Use the other port's buffer
        // TODO: ensure type is the same? or at least make sure on connect
        Port* other = (Port*) *(dependencies().begin());
        m_buffer = other->buffer();
      }

      if (!m_buffer) {
        // Return internal port
        m_buffer = provider.acquire(type(), 1024);
      }

      // TODO: Remove this hack
      if (type() == CONTROL_PORT) {
              float * data = (float*)m_buffer->data();
              data[0] = maximum();
      }
      break;

    case OUTPUT:
      numConnections = dependents().count();
      if (numConnections == 1) {
        // Use the other port's buffer
        // TODO: ensure type is the same? or at least make sure on connect
        Port* other = (Port*) *(dependents().begin());
        m_buffer = other->buffer();
      }

      if (!m_buffer) {
        m_buffer = provider.acquire(type(), 1024);
      }
      break;
  }
}

} // Unison

// vim: et ts=8 sw=2 sts=2 noai
