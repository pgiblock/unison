/*
 * Node.h
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


#ifndef NODE_H
#define NODE_H

#include "unison/types.h"
#include "unison/Port.h"

namespace Unison {

class ProcessingContext;

/** Interface for all things that participate in the processing graph.
 *  TODO: We probably want to add a StandardNode abstract class that handles
 *  most features that don't vary across different Node classes. */
class Node {
public:
	virtual ~Node () {};

	/** @return the total number of ports of all kinds */
	virtual uint32_t portCount () const = 0;

	/* TODO: Return Port* or shared pointer? */
	virtual Port* port (uint32_t idx) const = 0;

	virtual void activate () = 0;
	virtual void deactivate () = 0;

	virtual void process (const ProcessingContext & context) = 0;
};

/** A Safe pointer to a plugin. */
typedef QSharedPointer<Node> NodePtr;

} // Unison

#endif // NODE_H
