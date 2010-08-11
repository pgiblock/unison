/*
 * LadspaPlugin.h
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

#ifndef UNISON_LADSPA_PLUGIN_H
#define UNISON_LADSPA_PLUGIN_H

#include <unison/Plugin.h>
#include <unison/types.h>

#include <QVarLengthArray>

namespace Ladspa {
  namespace Internal {

/** Plugin implementation for an LadspaPlugin. 
 *  Put docs here */
class LadspaPlugin : public Unison::Plugin
{
  public:
    LadspaPlugin (/* stuff you need for a Ladspa Instance*/,
               Unison::nframes_t sampleRate);
    LadspaPlugin (const LadspaPlugin &);

    ~LadspaPlugin ();

    QString name () const;
    QString uniqueId () const;

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

    /* TODO: Functions that expose the C Ladspa structures (for LadspaPort) */

    void activate (Unison::BufferProvider *bp);
    void deactivate ();

    void process(const Unison::ProcessingContext &context);

    const QSet<Unison::Node* const> dependencies () const;
    const QSet<Unison::Node* const> dependents () const;

    Unison::BufferProvider *bufferProvider ();

  private:
    bool              m_activated;
    Unison::nframes_t m_sampleRate;
    Unison::BufferProvider *m_bufferProvider;

    void init ();
};



/** A description of a LADSPA plugin. */
class LadspaPluginDescriptor : public Unison::PluginDescriptor
{
  public:
    LadspaPluginDescriptor (/* Stuff you need for a description*/);
    LadspaPluginDescriptor (const LadspaPluginDescriptor& descriptor);

    Unison::PluginPtr createPlugin (Unison::nframes_t sampleRate) const;
};

  } // Internal
} // Ladspa

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
