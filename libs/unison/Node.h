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


#ifndef UNISON_NODE_H
#define UNISON_NODE_H

#include <QSet>
#include <QSharedPointer>
#include "unison/types.h"

namespace Unison {

class Patch;

/**
 * Interface for all things that participate in the processing graph. */
class Node
{
  public:
    virtual ~Node () {};

    virtual Node *parent () const = 0;

    Patch *parentPatch () const;

    /**
     * Dependencies are nodes that are directly "connected", that must
     * be processed before this node.  This typically means things that are
     * attached to this node's "input".  There is no requirement that the
     * connection is between ports, or that there is a connection at all.
     * For example, a Processor's dependencies are that Processor's "input"
     * ports, but the dependencies of an input port is the set of connected
     * output ports. */
    virtual const QSet<Node * const> dependencies () const = 0;


    /**
     * Dependents are nodes that directly "connected", that require this
     * node to be processed before themselves.  This typically means things
     * that are attached to this node's "output".  For example, the output
     * Ports of a Processor, or the input Ports connected to an output
     * Port. */
    virtual const QSet<Node * const> dependents () const = 0;

    /**
     * @returns a name for this node, suitable for storing in projects. */
    virtual QString name () const = 0;
};

/** A Safe pointer to a Node. */
typedef QSharedPointer<Node> NodePtr;

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
