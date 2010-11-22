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
#include <unison/PluginInfo.h>
#include <unison/types.h>

#include <QVarLengthArray>

#include "ladspa/ladspa.h"

namespace Ladspa {

const char * const UriRoot = "http://ladspa.org/plugins/";

  namespace Internal {

class LadspaPlugin : public Unison::Plugin
{
  public:
    LadspaPlugin (const LADSPA_Descriptor *descriptor,
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
      return m_ports.count();
    }

    Unison::Port* port (int idx) const;
    Unison::Port* port (const QString& name) const;

    // Exposing some innards for LadspaPort

    const LADSPA_Descriptor *ladspaDescriptor ()
    {
      return m_descriptor;
    }

    LADSPA_Handle ladspaHandle ()
    {
      return m_handle;
    }

    inline Unison::nframes_t sampleRate () const
    {
      return m_sampleRate;
    }

    void activate (Unison::BufferProvider& bp);
    void deactivate ();

    void process (const Unison::ProcessingContext &context);

    const QSet<Unison::Node* const> dependencies () const;
    const QSet<Unison::Node* const> dependents () const;

  private:
    const LADSPA_Descriptor *m_descriptor;
    LADSPA_Handle m_handle;

    QString m_uniqueId;
    bool              m_activated;
    Unison::nframes_t m_sampleRate;
    QVarLengthArray<Unison::Port*, 16> m_ports;
    QSet<Unison::Node* const> m_audioInPorts;
    QSet<Unison::Node* const> m_audioOutPorts;

    void init ();
};


/** A description of a LADSPA plugin. */
class LadspaPluginInfo : public Unison::PluginInfo
{
  public:
    LadspaPluginInfo (const QString &path, const LADSPA_Descriptor *descriptor);
    LadspaPluginInfo (const LadspaPluginInfo &descriptor);

    Unison::PluginPtr createPlugin (Unison::nframes_t sampleRate) const;

    const LADSPA_Descriptor *ladspaDescriptor ()
    {
      return m_descriptor;
    }

    QString filePath () const
    {
      return m_path;
    }

  private:
     QString m_path;
     const LADSPA_Descriptor *m_descriptor;
};

  } // Internal
} // Ladspa

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
