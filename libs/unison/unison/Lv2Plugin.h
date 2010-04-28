/*
 * Lv2Plugin.h
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

#ifndef UNISON_LV2_PLUGIN_H
#define UNISON_LV2_PLUGIN_H

#include <QVarLengthArray>
#include <slv2/slv2.h>

#include "unison/Plugin.h"
#include "unison/types.h"

namespace Unison
{

/** The SLV2World, and various cached (as symbols, fast) URIs.
 *  This object represents everything Unison 'knows' about LV2
 *  (ie understood extensions/features/etc) */
struct Lv2World
{
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
};



/** Plugin implementation for an Lv2Plugin.  Most values are queried directly
 *  from slv2 on demand.  It will probably be wise to cache some values when
 *  it is safe to do so (like num-ports, port-descriptors, etc..) */
class Lv2Plugin : public Plugin
{
  public:
    Lv2Plugin (Lv2World& world, SLV2Plugin plugin, nframes_t sampleRate);
    Lv2Plugin (const Lv2Plugin &);

    ~Lv2Plugin ();

    QString name () const
    {
       return QString::fromAscii( slv2_value_as_string( m_name ) );
    }

    QString uniqueId () const
    {
      return QString::fromAscii(
          slv2_value_as_uri( slv2_plugin_get_uri( m_plugin ) ) );
    }

    // TODO: PluginType type(); // VST, LADSPA, LV2, DSSI, etc..

    int audioInputCount () const;
    int audioOutputCount () const;
    QString authorName () const;
    QString authorEmail () const;
    QString authorHomepage () const;
    QString copyright () const;

    int portCount () const
    {
      return slv2_plugin_get_num_ports(m_plugin);
    }

    Port* port (int idx) const;

    /** @returns The underlying SLV2Plugin */
    SLV2Plugin slv2Plugin() const
    {
      return m_plugin;
    }

    /** @returns The underlying SLV2Instance */
    SLV2Instance slv2Instance() const
    {
      return m_instance;
    }

    void activate ();
    void deactivate ();

    void process(const ProcessingContext & context);

    const QSet<Node* const> dependencies () const;
    const QSet<Node* const> dependents () const;

    // TODO: loadState and saveState

  private:
    Lv2World&      m_world;
    SLV2Plugin     m_plugin;
    QVarLengthArray<Port*, 16> m_ports;
    nframes_t       m_sampleRate;

    SLV2Instance   m_instance;
    SLV2Value      m_name;
    SLV2Value      m_authorName;
    SLV2Value      m_authorEmail;
    SLV2Value      m_authorHomepage;
    SLV2Value      m_copyright;

    bool           m_activated;

    void init ();
};



/** A description of a LV2 plugin.  This descriptor allows us to query LV2
 *  plugins without actually instantiating them.  This can be abstracted
 *  into a PluginDescriptor if othe plugin types are ever needed. */
class Lv2PluginDescriptor : public PluginDescriptor
{
  public:
    Lv2PluginDescriptor (Lv2World& world, SLV2Plugin plugin);
    Lv2PluginDescriptor (const Lv2PluginDescriptor& descriptor);

    PluginPtr createPlugin (nframes_t sampleRate) const;

  private:
    Lv2World& m_world;
    SLV2Plugin m_plugin;
};

} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
