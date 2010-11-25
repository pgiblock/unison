/*
 * Node.hpp
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


#ifndef UNISON_NODE_HPP_
#define UNISON_NODE_HPP_

#include "types.hpp"

#include <QtCore/QSet>
#include <QtCore/QSharedPointer>

namespace Unison {
  
  class Patch;

/**
 * Interface for all things that participate in the processing graph. Unison
 * organizes all processing components into a hierarchial graph structure.
 * Not only are the objects in a directed graph, but there is also a
 * parent-child relationship.  The directed graph structure is to determine a
 * proper rendering traversal of dependencies.  The hierarchial stucture is to
 * manage object ownership as well as possible optimizations down the road
 * such as culling and delegating to slaves.
 */
class Node
{
  public:
    virtual ~Node () {};

    /**
     * Return the Parent node of this Node.  The actual subclass of the parent
     * depends on the subclass of this Node.  For example, the parent of a
     * Port is a Processor (or even more specifically, a Plugin).  The parent
     * of a Plugin is a Patch.  The parent of a Patch is another Patch, or
     * perhaps NULL, meaning this is the "root" Patch.
     *
     * There is a chance that Node should provide the implementation of
     * @f parent and subclasses should just static_cast to the expected type
     * @returns This Node's parent Node
     */
    virtual Node* parent () const = 0;

    /**
     * Walk the parents and find the first Patch.  This is useful primarily
     * for finding the right Patch to use when connecting two Ports.  However,
     * if we move the list of connections into Patch (instead of on Port) then
     * we may not require this function anymore
     * @returns The closest parent, @c NULL if the node is not owned
     */
    Patch* parentPatch () const;

    /**
     * Dependencies are nodes that are directly "connected", that must
     * be processed before this node.  This typically means things that are
     * attached to this node's "input".  There is no requirement that the
     * connection is between ports, or that there is a connection at all.
     * For example, a Processor's dependencies are that Processor's "input"
     * ports, but the dependencies of an input port is the set of connected
     * output ports.
     * This function is not required to be RT safe, and as such, must not be
     * called while processing.
     * @returns the set of Nodes required by this Node
     */
    virtual const QSet<Node* const> dependencies () const = 0;


    /**
     * Dependents are nodes that directly "connected", that require this
     * node to be processed before themselves.  This typically means things
     * that are attached to this node's "output".  For example, the output
     * Ports of a Processor, or the input Ports connected to an output
     * Port.
     * This function is not required to be RT safe, and as such, must not be
     * called while processing.
     * @returns the set of Nodes directly requiring this Node
     */
    virtual const QSet<Node* const> dependents () const = 0;

    /**
     * @returns a name for this node, suitable for storing in projects. The
     * name will also be used for generating paths.
     */
    virtual QString name () const = 0;
};

/** A Safe pointer to a Node. */
typedef QSharedPointer<Node> NodePtr;

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
