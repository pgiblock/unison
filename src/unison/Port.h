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


#ifndef PORT_H
#define PORT_H

#include <QSet>

namespace Unison
{

class Node;

/** A Port interface on a plugin.  Encapsulates audio, control, midi, and
 *  possibly other port types we may eventually be interested in. */
class Port
{
public:
	enum Type { AUDIO, CONTROL, MIDI, UNKNOWN };

	virtual ~Port () {};

	/** @returns the name of port, for example "OSC1 Attack". */
	virtual QString name (size_t maxLength) const = 0;

	/* TODO: Return an std::set of types instead??? */
	/** @returns the type of port */
	virtual Type type() const = 0;

	/* TODO: Replace isInput/Output() with direction() ? */
	/** @returns true if this port is an input port */
	virtual bool isInput() const = 0;

	/** @returns true if this port is an input port */
	virtual bool isOutput() const = 0;

	virtual void connectToBuffer(float * buf) = 0;

	/** @returns the current value of a port. */
	virtual float value () const = 0;

	/** Instantly set the value of this port, but will only be read by the
	 *  processing stages once-per-period
	 *  @param the value, bounded by minimum() and maximum() */
	virtual void setValue (float value) = 0;

	/** @returns The default value as requested by the plugin */
	virtual float defaultValue () const = 0;

	/** @returns true if minimum() and maximum() should be considered by
	 *  the host. */
	virtual bool isBounded () const = 0;

	/** @returns the minimum value this port should be set to */
	virtual float minimum () const = 0;

	/** @returns the maximum value this port should be set to */
	virtual float maximum () const = 0;

	/** @returns true if this port is toggled between on and off */
	virtual bool isToggled () const = 0;

	/** Returns Nodes responsible for providing the port with a value.
	 *  This does not relate to connections.  A port generally only has one
	 *  provider, the parent node.  However, a JACK port may have many nodes
	 *  connected behind it.  This is an easy way to handle that situation.
	 *  This may be handy for composite-nodes (patches) down the road.
	 *
	 *  @returns nodes to process() for this port */
	virtual const QSet<Node*> providers () const = 0;
	//virtual const Node* node () const = 0;

	/** @returns the set of OutputPorts attached to this input port,
	 *  otherwise, an empty set */
	// TODO: Isolate connections better
	virtual const QSet<Port*> connectedPorts() const {
		return m_connectedPorts;
	}

	// This sucks right here
	virtual const bool connectTo(Port* port) {
		if (!m_connectedPorts.contains(port)) {
			m_connectedPorts.insert(port);
			return true;
		}
		return false;
	}

	/** @returns true if this port has no dependents, that is, the port is a
	 *  'pure' sink.  Nodes with all output ports attached to sink'd
	 *  input-ports can be processed before any other node. */
	virtual bool isSink() const = 0;

protected:
	QSet<Port*> m_connectedPorts;
};

} // Unison

#endif // PORT_H
