/*
 * Processor.h
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


#ifndef UNISON_PROCESSOR_H
#define UNISON_PROCESSOR_H

#include "unison/Node.h"
#include "unison/Port.h"

namespace Unison
{

class CompositeProcessor;
class ProcessingContext;

/**
 * Interface for all Nodes that need to have some degree of processing
 * performed on them. */
class Processor : public Node
{
  public:
    Processor ();
    virtual ~Processor ();

    /** @return the total number of ports of all kinds */
    virtual int portCount () const = 0;

    virtual Port* port (int idx) const = 0;
    virtual Port* port (QString name) const = 0;

    virtual void activate () = 0;
    virtual void deactivate () = 0;

    virtual void process (const ProcessingContext & context) = 0;

    //// Connection oriented Stuff ////

    Node* parent () const;
    void setParent(CompositeProcessor* parent);

    /**
     * Has this node been visited (traversed)?  While compiling and perhaps
     * in other cases, it is useful to know if a processor has been visited
     * yet.  This is a leak of concerns, but much faster than most
     * alternatives.
     * @returns true if this node has been visited since the last unvisit */
    bool isVisited()
    {
      return m_visited;
    }

    /** Mark this node as visited */
    void visit()
    {
      m_visited = true;
    }

    /** Unmark this node as visited */
    void unvisit()
    {
      m_visited = false;
    };

  private:
    CompositeProcessor* m_parent;
    bool m_visited;
};

} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
