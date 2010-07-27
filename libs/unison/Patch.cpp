/*
 * Patch.cpp
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
#include "unison/Patch.h"
#include "unison/BufferProvider.h"

namespace Unison {

Patch::Patch () :
  Processor()
{
  m_compiled = new QList<CompiledProcessor>();
}

int Patch::portCount () const
{
  return 0;
}


Port* Patch::port (int idx) const
{
  Q_UNUSED(idx);
  // TODO: Implement registered-ports
  return NULL;
}


Port* Patch::port (QString id) const
{
  Q_UNUSED(id);
  // TODO: Implement registered-ports
  return NULL;
}


void Patch::activate (BufferProvider *bp)
{
  foreach (Processor* p, m_processors) {
    p->activate(bp);
  }
}


void Patch::deactivate ()
{
  foreach (Processor* p, m_processors) {
    p->deactivate();
  }
}


void Patch::setBufferLength (PortType type, nframes_t len)
{
  foreach (Processor *p, m_processors) {
    p->setBufferLength(type, len);
  }
}


void Patch::process (const ProcessingContext & context)
{
  foreach (CompiledProcessor cp, *m_compiled) {
    cp.processor->process(context);
  }
}


const QSet<Node* const> Patch::dependencies () const
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


const QSet<Node* const> Patch::dependents () const
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


QString Patch::name () const
{
  return "Patch";
}


void Patch::add (Processor * processor)
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


void Patch::remove (Processor * processor)
{
  Q_ASSERT(processor != NULL);
  Q_ASSERT(processor->parent() == this);
  m_processors.removeOne(processor);
  processor->setParent(NULL);
}


/**
 * Walks @n's parents and returns the parent closest to @_this.  If node @n
 * is not contained in @_this, then NULL is returned.  */
Processor* findOutermostProcessor (Patch * _this, Node * n)
{
  Processor* outer = NULL;
  while (n) {
    if (n == _this) {
      return outer;
    }

    if (Processor* p = dynamic_cast<Processor*>( n )) {
      outer = p;
    }

    n = n->parent();
  }
  return NULL;
}

// 2 if child's dependency is sibling, nothing special - walk and visit
// 3 if child's dependency is nested, then walk into the "nesting" sibling instead
void Patch::compileWalk (Node *n,
    QList<CompiledProcessor> &output)
{
  Processor *p;
  bool pendingAddition = false;

  if ((p = dynamic_cast<Processor*>(n)) && !p->isVisited()) {
    p->visit();
    pendingAddition = true;
  }

  foreach (Node* dep, n->dependencies()) {
    compileWalk( dep, output );
  }

  if (pendingAddition) {
    CompiledProcessor cp;
    cp.processor = p;
    output.append(cp);
  }
}

/*
  Alternate compilation method:
  create set of all nodes - these are "un-traversed".
  remove from traversed, recurse into dependents, add to compiled-list
*/


void Patch::compile (QList<CompiledProcessor>& output)
{
  // Mark everything as unvisited
  QListIterator<Processor*> i( m_processors );
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

    if (isSink) {
      compileWalk( proc, output );
    }
  }

  // Then compile everything else
  QListIterator<Processor*> p( m_processors );
  while (p.hasNext()) {
    compileWalk( p.next(), output );
  }
}


/*
void Patch::compile (BufferProvider & bufferProvider) {
  Q_ASSERT(QAtomicPointer< QList<CompiledProcessor> >
               ::isFetchAndStoreNative());

  QList<CompiledProcessor>* compiledSwap = new QList<CompiledProcessor>();
  qDebug() << name() << "compiling.";
  compile( m_processors, *compiledSwap );

  //qDebug() << "Aquiring 'fixed' buffers";
  qDebug() << name() << "Compiled, sequence is:";
  foreach (CompiledProcessor cp, *compiledSwap) {
    qDebug() << "  " << cp.processor->name();
    for (int i=0; i<cp.processor->portCount(); ++i) {
      Port *port = cp.processor->port(i);
      //qDebug() << "Next port: " << qPrintable(port->name());
      port->connectToBuffer(bufferProvider);
    }
  }


  // FIXME: this is probably not sufficient.
  compiledSwap = m_compiled.fetchAndStoreRelaxed( compiledSwap );
  delete compiledSwap;
}
*/

} // Unison

// vim: ts=8 sw=2 sts=2 et sta noai
