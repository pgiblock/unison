/*
 * Port.h
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


#ifndef UNISON_PORT_H
#define UNISON_PORT_H

#include "unison/Command.h"
#include "unison/Node.h"
#include "unison/BufferProvider.h"

namespace Unison {

class ProcessingContext;

/**
 * A Port interface on a plugin.  Encapsulates audio, control, midi, and
 * possibly other port types we may eventually be interested in.  Lv2Port
 * contains a full implementaion and should probably be split so that new Ports
 * like Vst, Ladspa, etc.. can reuse the impl. */
class Port : public Node
{
  public:
    Port ();

    virtual ~Port ()
    {};

    /** @returns the name of port, for example "OSC1 Attack". */
    virtual QString name () const = 0;

    /** @returns an id that can be used for serialization */
    virtual QString id () const = 0;

    /* TODO: Return an set of types instead??? */
    /** @returns the type of port */
    virtual PortType type() const = 0;

    /** @returns true if this port is an input port */
    virtual PortDirection direction() const = 0;

    /** @returns the current value of a port. */
    virtual float value () const = 0;

    /**
     * Instantly set the value of this port, but will only be read by the
     * processing stages once-per-period @param the value, bounded by minimum()
     * and maximum() */
    virtual void setValue (float value) = 0;

    /** @returns The default value as requested by the plugin */
    virtual float defaultValue () const = 0;

    /**
     * @returns true if minimum() and maximum() should be considered by
     * the host. */
    virtual bool isBounded () const = 0;

    /** @returns the minimum value this port should be set to */
    virtual float minimum () const = 0;

    /** @returns the maximum value this port should be set to */
    virtual float maximum () const = 0;

    /** @returns true if this port is toggled between on and off */
    virtual bool isToggled () const = 0;

    virtual void setBufferLength (BufferProvider &bp, nframes_t len);

    /**
     * Called in Process thread to assign the buffer used by this port
     * sub-classes may choose to assign a buffer from the BufferProvider or
     * from some other source. */
    virtual void connectToBuffer (BufferProvider & provider) = 0;

    /** Called in Process thread to retrieve the buffer for this Port */
    SharedBufferPtr buffer ()
    {
      return m_buffer;
    }

    // Connection stuff
    void connect (Port* other);
    void disconnect (Port* other);
    bool isConnected (Port* other) const;
    QSet<Port* const> connectedPorts () const
    {
      return m_connectedPorts;
    }

    /** @returns Either the connected Nodes or the interfaced Nodes */
    const QSet<Node* const> dependencies () const;

    /** @returns Either the connected Nodes or the interfaced Nodes */
    const QSet<Node* const> dependents () const;

    // Private API :: TODO: Move to D-ptr
    QSet<Port* const>* _connectedPorts ()
    {
      return &m_connectedPorts;
    }


  protected:
    // TODO: Move to some Port::Private class or something
    class Disconnect : public Command 
    {
      public:
        void preExecute ();
        void execute (ProcessingContext &context);
        void postExecute ();
    };

  protected:
    /**
     * Used by subclasses to list the nodes directly "behind" this port.  For
     * the most part this means either a single Processor, or some other ports
     * (like in the case of JACK connections).  @returns the set of nodes
     * interfaced by this Port. */
    virtual const QSet<Node* const> interfacedNodes () const = 0;

  protected:
    SharedBufferPtr m_buffer;

  private:
    QSet<Port* const> m_connectedPorts;
};

} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
