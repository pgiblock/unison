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


#ifndef UNISON_PATCH_H
#define UNISON_PATCH_H

#include "unison/Processor.h"

namespace Unison
{

class BufferProvider;
class ProcessingContext;


/**
 * A processor that contains other processors and their connections. This
 * class allows for a processor which manages other processors.  This is used
 * as a base class for Unison's higher-level constructs such as FxLines and
 * Synths, etc. */
class Patch : public Processor
{
  public:
    Patch ();

    virtual ~Patch ()
    {};

    virtual QString name () const;

    void add (Processor* processor);
    void remove (Processor* processor);

    /** @return the total number of ports of all kinds */
    virtual int portCount () const;

    virtual Port* port (int idx) const;
    virtual Port* port (QString id) const;

    virtual void activate (BufferProvider *bp);
    virtual void deactivate ();

    virtual void setBufferLength (PortType type, nframes_t len);

    virtual void process (const ProcessingContext& context);

    const QSet<Node* const> dependencies () const;
    const QSet<Node* const> dependents () const;

    // Private API :: TODO: Move to D-ptr

    struct CompiledProcessor {
      Processor* processor;
    };

    /**
     * Compiles the given list of processors into a proper traversal for
     * rendering.  Not reentrant. */
    void compile (QList<CompiledProcessor>& output);

    void setCompiledProcessors(QList<CompiledProcessor>* processors)
    {
      m_compiled = processors;
    }

    QList<CompiledProcessor>* compiledProcessors() const
    {
      return m_compiled;
    }


  protected:
    virtual void registerPort()
    {};

  private:

    /**
     * A recursive walk into the dependencies of the node.  Processors
     * are appended to the output.  This uses the visited flag of
     * Processor, therefore this function is not reentrant. */
    void compileWalk (Node* n, QList<CompiledProcessor>& output);

    QAtomicPointer< QList<CompiledProcessor> > m_compiled;
    QList<Processor*> m_processors;
};



/**
 * Provides a proxy to a port internal to a Patch. The
 * Patch allows for implementations to registerPorts.  This
 * exposes an internal port as if it is a port of the Patch. */
class PatchProxyPort : public Port
{
  protected:
    /**
     * Constructs a Port for a Patch.  Called by
     * Patch itself.
     * @param processor The parent processor
     * @param port      The port to proxy */
    PatchProxyPort (Patch* processor,
                                 Port* port) :
      m_port(port),
      m_processor(processor)
    {}

    ~PatchProxyPort ()
    {}

    QString name () const
    {
      return m_port->name();
    }

    PortType type () const
    {
      return m_port->type();
    }

    PortDirection direction () const
    {
      return m_port->direction();
    }

    float value () const
    {
      return m_port->value();
    }

    void setValue (float value)
    {
      m_port->setValue(value);
    }

    float defaultValue () const
    {
      return m_port->defaultValue();
    }

    bool isBounded () const
    {
      return m_port->isBounded();
    }

    float minimum () const
    {
      return m_port->minimum();
    }

    float maximum () const
    {
      return m_port->maximum();
    }

    bool isToggled () const
    {
      return m_port->isToggled();
    }

    const QSet<Node* const> interfacedNodes () const
    {
      QSet<Node* const> p;
      p.insert( m_processor );
      return p;
    }

    void connectToBuffer ()
    {
      // TODO: Might need more logic here
      m_port->connectToBuffer();
    }

  private:
    Port* m_port;                    ///< The proxied port
    Patch* m_processor; ///< The processor owning this port

    friend class Patch;
};

} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
