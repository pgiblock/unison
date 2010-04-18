/*
 * Session.cpp
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

#include "unison/Node.h"
#include "unison/Processor.h"
#include "unison/Session.h"
#include "unison/BufferProvider.h"
#include "unison/JackEngine.h"

namespace Unison
{

Session::Session (JackEngine& engine) :
  m_bufferProvider(),
  m_engine(engine)
{}


void Session::compileRecursive (Node* n,
    QList<CompiledProcessor>& output)
{
  // TODO: Dynamic cast is bad. any better way to do this other than moving
  // process(), visit(), isVisited() etc to Node?
  Processor* p = dynamic_cast<Processor*>( n );
  if (p == NULL || !p->isVisited()) {
    if (p) {
      p->visit();
    }
    foreach (Node* dep, n->dependencies()) {
      compileRecursive( dep, output );
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


void Session::compile (QList<Processor*> input,
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

    if (isSink) {
      compileRecursive( proc, output );
    }
  }

  // Then compile everything else
  QListIterator<Processor*> p( input );
  while (p.hasNext()) {
    compileRecursive( p.next(), output );
  }
}


void Session::process (const ProcessingContext & context)
{
  // Processing loop
  foreach (CompiledProcessor cp, *compiled) {
    cp.processor->process(context);
  }
}


BufferProvider& Session::bufferProvider() const
{
  return m_bufferProvider;
}

} // Unison

// vim: et ts=8 sw=2 sts=2 noai
