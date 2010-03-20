/*
 * Route.h
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

#ifndef ROUTE_H
#define ROUTE_H

namespace Unison {

class Port;


/** A direct connection between two ports.  TODO: Consider renaming since
 *  'route' doesn't make it clear if this is a single or multi-hop route. */
class Route {
public:
	Route (Port* insert, Port* output);
	virtual ~Route () {};

	Port* insertPort () const {
		return m_insertPort;
	}

	Port* outputPort () const {
		return m_outputPort;
	}

protected:
	Port* const m_insertPort;
	Port* const m_outputPort;

};

} // Unison

#endif // PORT_H
