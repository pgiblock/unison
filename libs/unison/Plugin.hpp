/*
 * Plugin.hpp
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


#ifndef UNISON_PLUGIN_HPP_
#define UNISON_PLUGIN_HPP_

#include "Processor.hpp"
#include "types.hpp"

#include <QtCore/QSharedPointer>

namespace Unison {

  class Patch;

/**
 * The type of plugin, regarding I/O.
 * This is a relic from LMMS and doesn't serve us much purpose.
 * TODO: Pending removal or clarity
 */
enum PluginType
{
  SourcePlugin,     ///< Plugin's audio ports are only for output
  TransferPlugin,   ///< Plugin has both input and output audio ports
  ValidPlugin,      ///< ???
  InvalidPlugin,    ///< ???
  SinkPlugin,       ///< Plugin's audio ports are only for input
  OtherPlugin       ///< ???
};



/**
 * Interface for audio, control, and maybe other "graphed" plugins for Unison.  All
 * accessors are virtual since some plugin types may be able to query the values directly
 * from the underlying resource.  TODO: Consider splitting Plugin from Processor. And
 * allow for Plugin to create a processor "instance". Or better, Move most of the
 * slv2_plugin functions from Lv2Plugin to PluginInfo instead?
 */
class Plugin : public Processor
{
  public:
    Plugin () : Processor()
    {}

    virtual ~Plugin ()
    {};

    /**
     * @returns the name of the plugin. Such as, "Triple Oscillator"
     */
    virtual QString name () const = 0;

    /**
     * @returns a uniqueId to be used when saving references to the plugin
     */
    virtual QString uniqueId () const = 0;

    // TODO: PluginType type(),  loadState(),  and saveState();

    /**
     * @returns the count of audio input channels. 2 for stereo, etc..
     */
    virtual int audioInputCount () const = 0;

    /**
     * @returns the count of audio output channels. 2 for stereo, etc..
     */
    virtual int audioOutputCount () const = 0;

    /**
     * @returns the author, or company's name. Example "Paul Giblock".
     */
    virtual QString authorName () const = 0;

    /**
     * @returns the email of the author or company.
     */
    virtual QString authorEmail () const = 0;

    /**
     * @returns the homepage of the author or company.
     */
    virtual QString authorHomepage() const = 0;

    /**
     * @returns the copyright or license restricting usage of the plugin.
     */
    virtual QString copyright() const = 0;
};

/**
 * A Safe pointer to a plugin.
 */
//typedef QSharedPointer<Plugin> PluginPtr;
typedef Plugin *PluginPtr;


} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
