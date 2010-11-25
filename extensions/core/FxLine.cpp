/*
 * Engine.cpp
 *
 * Copyright (c) 2010 Paul Giblock <pgib/at/users.sourceforge.net>
 *                    kizzobot <kizzobot/at/gmail.com>
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

#include "Engine.hpp"
#include "FxLine.hpp"
#include "PluginManager.hpp"

#include <unison/Backend.hpp>
#include <unison/BackendPort.hpp>
#include <unison/Patch.hpp>
#include <unison/Plugin.hpp>

#include <QDebug>

using namespace Unison;

namespace Core {

FxLine::FxLine (Patch& parent, const QString& name) :
  m_name(name),
  m_parent(parent),
  m_entries()
{
  m_inPorts[0]  = Engine::backend()->registerPort(name + "/in 1", Output);
  m_inPorts[1]  = Engine::backend()->registerPort(name + "/in 2", Output);
  Entry entry = Entry();
  entry.outputPorts << m_inPorts[0] << m_inPorts[1];
  m_entries.append(entry);
  
  m_outPorts[0] = Engine::backend()->registerPort(name + "/out 1", Input);
  m_outPorts[1] = Engine::backend()->registerPort(name + "/out 2", Input);
  entry = Entry();
  entry.inputPorts << m_outPorts[0] << m_outPorts[1];
  m_entries.append(entry);

  // TODO: Bring back once backend ports have patch?
  m_inPorts[0]->connect(m_outPorts[0], *Engine::bufferProvider());
  m_inPorts[1]->connect(m_outPorts[1], *Engine::bufferProvider());
}


FxLine::~FxLine ()
{
  Engine::backend()->unregisterPort(m_inPorts[0]);
  Engine::backend()->unregisterPort(m_inPorts[1]);
  Engine::backend()->unregisterPort(m_outPorts[0]);
  Engine::backend()->unregisterPort(m_outPorts[1]);
}


QString FxLine::name() const
{
  return m_name;
}


void FxLine::collectPorts (Plugin* plugin, QList<Port*>* audioIn, QList<Port*>* audioOut) const
{
  if (audioIn) {
    audioIn->empty();
  }
  if (audioOut) {
    audioOut->empty();
  }
  for (int i = 0; i < plugin->portCount(); i++)
  {
    Port* p = plugin->port(i);

    if (p->type() == AudioPort)
    {
      switch (p->direction()) {
        case Input:
          if (audioIn) {
            audioIn->append(p);
          }
          break;
        case Output:
          if (audioOut) {
            audioOut->append(p);
          }
          break;
        default:
          // Error.
          break;
      }
    }
  }
}


/// TODO: This code can be simplified.  Make a PortContainer class.
/// Plugin and Patch can inherit from PortContainer.  We can have a 
/// pseudo-PortContainer here to represent the input and output ports
/// Then, all adds/removals will be symetric
/// ^^^: Or, just wrap Plugin and BackendPorts with a class that acts this way
void FxLine::addPlugin(const PluginInfoPtr info, int pos)
{
  int idx = pos + 1;
  int pluginCnt = m_entries.length() - 2;
  
  // Check for proper position value. TODO: Report error, not fatal
  Q_ASSERT(pos <= pluginCnt);

  // Create the plugin. TODO: Report error, not fatal
  Plugin* plugin = info->createPlugin(48000);
  Q_ASSERT(plugin);

  plugin->activate(*Engine::bufferProvider());
  m_parent.add(plugin);

  // Verify number of ports. TODO: Report error, not fatal
  Q_ASSERT(plugin->audioInputCount() == 2);
  Q_ASSERT(plugin->audioOutputCount() == 2);

  // Collect ports.
  Entry entry;
  entry.plugin = plugin;
  collectPorts(plugin, &entry.inputPorts, &entry.outputPorts);
  Q_ASSERT(entry.inputPorts.length() == 2);
  Q_ASSERT(entry.outputPorts.length() == 2);

  /*
  if (m_plugins.length() == 0) {
    // If there are no plugins, we disconnect the backend ports from each
    // other. Then, we connect the plugin in-between the backend ports

    // TODO: Bring back once backend ports have patch?
    // m_inPorts[0]->disconnect(m_outPorts[0]);
    // m_inPorts[1]->disconnect(m_outPorts[1]);

    m_inPorts[0]->connect(pluginIn.at(0));
    m_inPorts[1]->connect(pluginIn.at(1));

    m_outPorts[0]->connect(pluginOut.at(0));
    m_outPorts[1]->connect(pluginOut.at(1));
  }
  else if (pos < m_plugins.length()) {
  */
    // At this point, there is at least one plugin already in the line, and we 
    // are trying to insert the new plugin before another one.
    Entry producer = m_entries.value(idx-1);
    Entry consumer = m_entries.value(idx);

    for (int i=0; i<producer.outputPorts.count(); ++i) {
      Port* producerPort = producer.outputPorts.at(i);
      Port* consumerPort = consumer.inputPorts.at(i);
      
      // Work around:
      if (producerPort->parentPatch() == NULL && consumerPort->parentPatch() == NULL) {
        qWarning("Probably disconnecting two backend-ports. Ignoring");
      }
      else {
        producerPort->disconnect(consumerPort, *Engine::bufferProvider());
      }
      producerPort->connect(entry.inputPorts.at(i), *Engine::bufferProvider());
      consumerPort->connect(entry.outputPorts.at(i), *Engine::bufferProvider());
    }

  m_entries.insert(idx, entry);
}


} // Core

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
