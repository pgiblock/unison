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

namespace Unison {

class Node;

/** A Port interface on a plugin.  Encapsulates audio, control, midi, and
 *  possibly other port types we may eventually be interested in. */
class Port {
public:
	virtual ~Port () {};

	virtual const Node* node () const = 0;

	/** @returns the name of port, for example "OSC1 Attack". */
	virtual QString name (size_t maxLength) const = 0;

	/* TODO: virtual Type types() const
		and  virtual boolean isType(Type) const
		for audio, control, midi, etc */

	/** @returns true if this port is an input port */
	virtual bool isInput() const = 0;

	/** @returns true if this port is an input port */
	virtual bool isOutput() const = 0;

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
};

} // Unison

#endif // PORT_H
