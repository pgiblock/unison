/*
 * CompositeProcessor.cpp
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

#include <QDebug>

#include "unison/Node.h"
#include "unison/CompositeProcessor.h"
#include "unison/Session.h"
#include "unison/BufferProvider.h"

namespace Unison
{

int CompositeProcessor::portCount () const
{
  return 0;
}


Port* CompositeProcessor::port (int idx) const
{
  return NULL;
}


void CompositeProcessor::activate ()
{
  foreach (Processor* p, m_processors) {
    p->activate();
  }
}


void CompositeProcessor::deactivate ()
{
  foreach (Processor* p, m_processors) {
    p->deactivate();
  }
}


void CompositeProcessor::process (const ProcessingContext & context)
{
  foreach (CompiledProcessor cp, *m_compiled) {
    //qDebug() << "CompositeProcessor" << name() << " processing " << cp.processor->name();
    cp.processor->process(context);
  }
}


const QSet<Node* const> CompositeProcessor::dependencies () const
{
  QSet<Node* const> n;
  int count = portCount();
  for (int i=0; i<count; ++i) {
    Port* p  = port(i);
    if (p->direction() == INPUT) {
      n += p;
    }
  }
  return n;
}


const QSet<Node* const> CompositeProcessor::dependents () const
{
  QSet<Node* const> n;
  int count = portCount();
  for (int i=0; i<count; ++i) {
    Port* p  = port(i);
    if (p->direction() == OUTPUT) {
      n += p;
    }
  }
  return n;
}


QString CompositeProcessor::name () const
{
  return "CompositeProcessor";
}


void CompositeProcessor::add (Processor * processor)
{
  Q_ASSERT(processor != NULL);
  if (processor->parent() == this ) {
    return;
  }
  if (processor->parent()) {
    qFatal("Reparenting processors is not yet supported");
  }
  if (!m_processors.contains(processor)) {
    m_processors.append(processor);
    processor->setParent(this);
  }
}


void CompositeProcessor::remove (Processor * processor)
{
  Q_ASSERT(processor != NULL);
  Q_ASSERT(processor->parent() == this);
  m_processors.removeOne(processor);
  processor->setParent(NULL);
}


void CompositeProcessor::compileWalk (Node* n,
    QList<CompiledProcessor>& output)
{
  // TODO: Dynamic cast is bad. any better way to do this other than moving
  // process(), visit(), isVisited() etc to Node?
  Processor* p = dynamic_cast<Processor*>( n );

  qDebug() << "Walking node: " << n->name();

  if (p == NULL || !p->isVisited()) {
    if (p) {
      p->visit();
    }
    foreach (Node* dep, n->dependencies()) {
      compileWalk( dep, output );
    }
    if (p) {
      CompiledProcessor cp;
      cp.processor = p;
      output.append(cp);
    }
  }
}

/*
  Alternate compilation method:
  create set of all nodes - these are "untraversed".
  remove from traversed, recurse into dependents, add to compiled-list
*/


void CompositeProcessor::compile (QList<Processor*> input,
    QList<CompiledProcessor>& output)
{
  // Mark everything as unvisited
  QListIterator<Processor*> i( input );
  while (i.hasNext()) {
    i.next()->unvisit();
  }

  // Process nodes that are pure-sinks first
  i.toFront();
  while (i.hasNext()) {
    Processor* proc = i.next();

    bool isSink = true;
    bool done = false;

    // A rather naster traversal to find sinks:

    // For each output port
    for (int j = 0; j < proc->portCount() && !done; ++j) {
      Port* port = proc->port(j);
      if (port->direction() == OUTPUT) {
        // For all connected Ports.
        QSetIterator<Node* const> k( port->dependents() );
        while (k.hasNext()) {
          Port* otherPort = (Port*)k.next();
          // Not a sink if a connected port has any dependents.
          if (otherPort->dependents().count() != 0) {
            // Not sink, and break out of outer loop:
            isSink = false;
            done = true;
            continue;
          }
        }
      }
    }

    qDebug() << "Node " << proc->name() << " is sink: " << isSink;

    if (isSink) {
      compileWalk( proc, output );
    }
  }

  // Then compile everything else
  QListIterator<Processor*> p( input );
  while (p.hasNext()) {
    qDebug() << "Compiling next remaining node";
    compileWalk( p.next(), output );
  }
}


void CompositeProcessor::compile (BufferProvider & bufferProvider) {
  QList<CompiledProcessor>* compiledSwap = new QList<CompiledProcessor>();
  qDebug() << name() << "compiling.";
  compile( m_processors, *compiledSwap );

  qDebug() << "Aquiring 'fixed' buffers";
  foreach (CompiledProcessor cp, *compiledSwap) {
    for (int i=0; i<cp.processor->portCount(); ++i) {
      Port *port = cp.processor->port(i);
      qDebug() << "Next port: " << qPrintable(port->name());
      port->connectToBuffer(bufferProvider);
    }
  }

  // FIXME: this is probably not sufficient.
  compiledSwap = m_compiled.fetchAndStoreRelaxed( compiledSwap );
  delete compiledSwap;
}

} // Unison

// vim: ts=8 sw=2 sts=2 et sta noai
