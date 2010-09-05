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

#include <unison/Plugin.h>
#include <unison/PluginInfo.h>
#include <unison/types.h>

#include <QVarLengthArray>
#include <slv2/slv2.h>

namespace Lv2 {
  namespace Internal {

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
class Lv2Plugin : public Unison::Plugin
{
  public:
    Lv2Plugin (Lv2World& world, SLV2Plugin plugin,
               Unison::nframes_t sampleRate);
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

    Unison::Port* port (int idx) const;
    Unison::Port* port (QString name) const;

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

    void activate (Unison::BufferProvider *bp);
    void deactivate ();

    void process(const Unison::ProcessingContext &context);

    const QSet<Unison::Node* const> dependencies () const;
    const QSet<Unison::Node* const> dependents () const;

    // TODO: loadState and saveState

    Unison::BufferProvider *bufferProvider ();

  private:
    Lv2World&         m_world;
    SLV2Plugin        m_plugin;

    SLV2Instance      m_instance;
    SLV2Value         m_name;
    SLV2Value         m_authorName;
    SLV2Value         m_authorEmail;
    SLV2Value         m_authorHomepage;
    SLV2Value         m_copyright;

    bool              m_activated;
    Unison::nframes_t m_sampleRate;
    QVarLengthArray<Unison::Port*, 16> m_ports;
    Unison::BufferProvider *m_bufferProvider;

    void init ();
};



/** A description of a LV2 plugin.  This descriptor allows us to query LV2
 *  plugins without actually instantiating them.  This can be abstracted
 *  into a PluginInfo if othe plugin types are ever needed. */
class Lv2PluginInfo : public Unison::PluginInfo
{
  public:
    Lv2PluginInfo (Lv2World& world, SLV2Plugin plugin);
    Lv2PluginInfo (const Lv2PluginInfo& descriptor);

    Unison::PluginPtr createPlugin (Unison::nframes_t sampleRate) const;

  private:
    Lv2World& m_world;
    SLV2Plugin m_plugin;
};

  } // Internal
} // Lv2

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
