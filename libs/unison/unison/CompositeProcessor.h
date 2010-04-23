/*
 * CompositeProcessor.h
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


#ifndef COMPOSITE_PROCESSOR_H
#define COMPOSITE_PROCESSOR_H

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
class CompositeProcessor : public Processor
{
  public:
    CompositeProcessor (/*CompositeProcessor* parent*/)
    {};
    virtual ~CompositeProcessor ()
    {};

    /** @return the total number of ports of all kinds */
    virtual int portCount () const;

    virtual Port* port (int idx) const;

    virtual void activate ();
    virtual void deactivate ();

    virtual void process (const ProcessingContext & context);

    const QSet<Node* const> dependencies () const;
    const QSet<Node* const> dependents () const;

  protected:
    void add (Processor * processor);
    virtual void registerPort()
    {};

    /**
     * A hack to let subs force-compile for now.  Should be removed once
     * CompiledProcessor actually manages connect()ing
     */
    void hackCompile (BufferProvider & pool)
    {
      compile(pool);
    }

  private:
    struct CompiledProcessor {
      Processor* processor;
    };

    /**
     * Compiles graph of child processors */
    void compile (BufferProvider& bufferProvider);

    /**
     * Compiles the given list of processors into a proper traversal for
     * rendering.  Not reentrant. */
    static void compile (QList<Processor*> input,
                         QList<CompiledProcessor>& output);

    /**
     * A recursive walk into the dependencies of the node.  Processors
     * are appended to the output.  This uses the visited flag of
     * Processor, therefore this function is not reentrant. */
    static void compileWalk (Node* n, QList<CompiledProcessor>& output);

    QAtomicPointer< QList<CompiledProcessor> > compiled;
    QList<Processor*> processors;
};



class CompositeProcessorProxyPort : public Port
{
  public:
    CompositeProcessorProxyPort (CompositeProcessor * processor,
                                 Port * port) :
      m_processor(processor),
      m_port(port)
    {}

    ~CompositeProcessorProxyPort ()
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

    void connectToBuffer (BufferProvider & provider)
    {
      // TODO: Might need more logic here
      m_port->connectToBuffer(provider);
    }

  private:
    Port * m_port;
    CompositeProcessor * m_processor;
};

} // Unison

#endif

// vim: et ts=8 sw=2 sts=2 noai
