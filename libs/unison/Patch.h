/*
 * Patch.h
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


#ifndef UNISON_PATCH_H_
#define UNISON_PATCH_H_

#include "Processor.h"

namespace Unison {

  class BufferProvider;
  class ProcessingContext;

/**
 * A processor that contains other processors and their connections. This class allows for
 * a processor which manages other processors.  This is used as a base class for Unison's
 * higher-level constructs such as FxLines and Synths, etc.  Additionally, the root
 * processor of a @c Backend is represented as a Patch.  Patch is optimized by compiling a
 * proper traversal of the children when the graph is changed.  The @c Commander is used
 * to allow us to use the new traversal in the rendering phrase in a RT safe manner.
 */
class Patch : public Processor
{
  public:
    Patch ();

    virtual ~Patch ()
    {};

    virtual QString name () const;

    /**
     * Add @p processor to this Patch and parent the Processor.  Reparenting of processors
     * is not yet supported and will cause the program to abort.
     * @param processor The processor to add, but have no parent
     */
    void add (Processor* processor);

    /**
     * Removes @p processor from this Patch if it is currently a child.  This function
     * will set @p processor's parent to NULL and will disconnect all of the ports
     * @param processor the processor to remove
     */
    void remove (Processor* processor);

    virtual int portCount () const;

    virtual Port* port (int idx) const;
    virtual Port* port (const QString& id) const;

    virtual void activate (BufferProvider* bp);
    virtual void deactivate ();

    virtual void setBufferLength (PortType type, nframes_t len);

    virtual void process (const ProcessingContext& context);

    const QSet<Node* const> dependencies () const;
    const QSet<Node* const> dependents () const;

    // Private API :: TODO: Move to D-ptr

    /**
     * A processor entry in the compiled list of Processors
     * @internal
     */
    struct CompiledProcessor {
      Processor* processor;
    };

    /**
     * Compiles the given list of processors into a proper traversal for
     * rendering.  Not reentrant.
     * @param output The resulting traversal
     * @internal
     */
    void compile (QList<CompiledProcessor>& output);

    /**
     * Swap the list of compiled processors.  Must be called in the Backend's
     * processing thread before or after rendering of the current period.
     * @param processors The new list of processors to use
     */
    void setCompiledProcessors (QList<CompiledProcessor>* processors)
    {
      m_compiled = processors;
    }

    /**
     * @returns the current rendering order used by this Patch
     */
    QList<CompiledProcessor>* compiledProcessors () const
    {
      return m_compiled;
    }


  protected:
    /**
     * Allow subclasses to register ProxyPorts
     */
    virtual void registerPort ()
    {};

  private:

    /**
     * A recursive walk into the dependencies of the node.  Processors are appended to the
     * output.  This uses the visited flag of Processor, therefore this function is not
     * reentrant (and not const either).
     */
    void compileWalk (Node* n, QList<CompiledProcessor>& output);

    QAtomicPointer< QList<CompiledProcessor> > m_compiled; ///< pointer to current order
    QList<Processor*> m_processors; ///< our children
};

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
