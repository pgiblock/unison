/*
 * PluginInfo.h
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


#ifndef UNISON_PLUGININFO_H_
#define UNISON_PLUGININFO_H_

#include "Plugin.h"
#include "types.h"

namespace Unison {


/**
 * PluginInfo represents the meta-data of a plugin and provides an
 * interface for instantiating Plugins.
 */
class PluginInfo
{
  public:
    PluginInfo () :
      m_audioInputs(0),
      m_audioOutputs(0)
    {
      // Not much sense initializing anything here since no defaults make
      // sense and this class is abstract anyways.
    }

    PluginInfo (const PluginInfo& info) :
      m_uniqueId(info.m_uniqueId),
      m_author(info.m_author),
      m_name(info.m_name),
      m_audioInputs(info.m_audioInputs),
      m_audioOutputs(info.m_audioOutputs)
    {}

    virtual ~PluginInfo()
    {}

    virtual PluginPtr createPlugin (nframes_t sampleRate) const = 0;

    /**
     * @returns the name of the plugin. Such as, "Triple Oscillator"
     */
    QString name () const
    {
      return m_name;
    }

    /**
     * @returns a uniqueId to be used when saving references to the plugin
     */
    QString uniqueId () const
    {
      return m_uniqueId;
    }

    /**
     * XXX: This is a hold-over from LMMS and should probably be removed.
     */
    PluginType type () const
    {
      if (m_audioInputs > 0) {
        if (m_audioOutputs > 0) {
          return TransferPlugin;
        }
        else {
          return SinkPlugin;
        }
      }
      else if (m_audioOutputs > 0) {
        return SourcePlugin;
      }
      else {
        return OtherPlugin;
      }
    }

    /**
     * @returns the count of audio input channels. 2 for stereo, etc..
     */
    int audioInputCount () const
    {
      return m_audioInputs;
    }

    /**
     * @returns the count of audio output channels. 2 for stereo, etc..
     */
    int audioOutputCount () const
    {
      return m_audioOutputs;
    }

    /**
     * @returns the author, or company's name. Example "Paul Giblock".
     */
    QString authorName () const
    {
      return m_author;
    }

  protected:

    void setName (const QString& name)
    {
      m_name = name;
    }

    void setUniqueId (const QString& uniqueId)
    {
      m_uniqueId = uniqueId;
    }

    void setAudioInputCount (int audioInputCount)
    {
      m_audioInputs = audioInputCount;
    }

    void setAudioOutputCount (int audioOutputCount)
    {
      m_audioOutputs = audioOutputCount;
    }

    void setAuthorName (const QString& authorName)
    {
      m_author = authorName;
    }

  private:
    QString m_uniqueId;
    QString m_author;
    QString m_name;
    int m_audioInputs;
    int m_audioOutputs;
};

/** A Safe pointer to a plugin descriptor. FIXME */
typedef QSharedPointer<PluginInfo> PluginInfoPtr;

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
