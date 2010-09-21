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


#ifndef UNISON_PROCESSOR_H_
#define UNISON_PROCESSOR_H_

#include "Node.h"

namespace Unison {

  class BufferProvider;
  class Patch;
  class Port;
  class ProcessingContext;

/**
 * Interface for all Nodes that need to have some degree of processing performed on them.
 * The parent @c Patch will call the @f process function when it is time to process.
 * @seealso Plugin
 */
class Processor : public Node
{
  public:
    Processor ();
    virtual ~Processor ();

    /**
     * @returns the total number of ports of all types and directions
     */
    virtual int portCount () const = 0;

    /**
     * @param idx The index of the port to find
     * @returns The port at index @p idx
     */
    virtual Port* port (int idx) const = 0;

    /**
     * @param idx The (short) name of the port to find
     * @returns The port with name @p name, if any
     */
    virtual Port* port (const QString& name) const = 0;

    /**
     * Activates the processor.  Basically, the processor must prepare for processing.
     * The function does not need to be RT-safe.  However, it shouldn't take excessively
     * long to run.  The processor can be thought of as being in a "running" state,
     * although there is no guarantee that process() will be called any time soon.
     * @param bp the @c BufferProvider to use until we are deactivated
     */
    virtual void activate (BufferProvider* bp) = 0;

    /**
     * Deactivates the processor. Allow the processor to free any resources that it no
     * longer needs.  The @f process() function will not be called until the processor is
     * reactivated.  The Processor should no longer use the @c BufferProvider that was
     * previously provided to @f activate().
     */
    virtual void deactivate () = 0;

    /**
     * Process the Processor according to the information in @p context
     * This is where the DSP of the processor will be invoked.  The processor is free to
     * read values from the input ports and write to the output ports.
     * @param context The parameters of the current rendering period
     */
    virtual void process (const ProcessingContext& context) = 0;

    /**
     * Change the bufferLength of the processor.  XXX: The contract of this function is
     * not finalized.  We need to put more thought into samplerate and buffersize changes
     * @param type the type of Port that needs to be resized.  Currently only
     *        @c AudioPort or @c MidiPort makes sense.
     * @param len the length to resize to
     */
    virtual void setBufferLength (PortType type, nframes_t len);

    //// Connection oriented Stuff ////

    Node* parent () const;
    void setParent(Patch* parent);

    /**
     * Has this node been visited (traversed)?  While compiling and perhaps
     * in other cases, it is useful to know if a processor has been visited
     * yet.  This is a leak of concerns, but much faster than most
     * alternatives.
     * @returns true if this node has been visited since the last unvisit
     */
    bool isVisited()
    {
      return m_visited;
    }

    /**
     * Mark this node as visited. Used by @c Patch while compiling.
     * @internal
     */
    void visit()
    {
      m_visited = true;
    }

    /**
     * Unmark this node as visited. Used by @c Patch while compiling.
     * @internal
     */
    void unvisit()
    {
      m_visited = false;
    };

  private:
    Patch* m_parent;
    bool m_visited;
};

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
