/*
 * FxLine.h
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

#ifndef UNISON_FXLINE_H
#define UNISON_FXLINE_H

#include "unison/Plugin.h"
#include "extensions/jack/JackPort.h"
#include "extensions/core/Session.h"
#include "unison/CompositeProcessor.h"

namespace Unison
{

class PluginEntry {
public:
    PluginEntry(const PluginDescriptor& descriptor, Processor *proc);
private:
    const PluginDescriptor& descriptor;
    Processor *proc;
};

/** A plugin effects line.
 *
 * An FxLine is a processor that lines up the inputs and outputs of a sequence 
 * of plugins.  An FxLine has exactly 2 JACK input ports and exactly 2 JACK 
 * output ports.
 *
 * This implementation currently assumes that all added plugins will have 
 * exactly 2 audio in ports and exactly 2 audio out ports.  The first plugin 
 * added will have its 2 in ports connected to the FxLine's 2 JACK in ports; 
 * likewise, the plugin's 2 out ports will be connected to the FxLine's 2 JACK 
 * out ports.  If the next plugin to be inserted were to be inserted just 
 * before the first one, then the FxLine's audio in ports will be disconnected 
 * from the first plugin, and they will be reconnected to new plugin's in 
 * ports; in addition, the new plugin's out ports will be connected to the 
 * other plugin's in ports.*/
class FxLine : public CompositeProcessor {
  public:
    /** Constructs an FxLine.
     *
     * When an FxLine object is first created, in1 is connected to out1 and 
     * in2 is connected to out2.*/
    FxLine (Core::Session& session, QString name);
    ~FxLine();

    QString name() const;
    void addEffect();

    /** Insert a plugin into the effects line at the given position.
     *
     * @param descriptor PluginDescriptor of plugin
     * @param pos The index of where plugin will be added
     */
    void addPlugin(const PluginDescriptor& descriptor, int pos = -1);

  private:
    QString m_name;
    Core::Session& m_session;
    /// The 2 JACK audio in ports.
    JackPort* m_inPorts[2];
    /// The 2 JACK audio out ports.
    JackPort* m_outPorts[2];
    QList<PluginEntry> plugins;
};

} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
