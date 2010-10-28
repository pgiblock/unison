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

#include "Patch.h"

#include "Node.h"
#include "Port.h"
#include "Scheduler.h"

#include <QtCore/QDebug>

namespace Unison {

Patch::Patch () :
  Processor()
{
  m_schedule = new Internal::Schedule();
  m_schedule->work = NULL;
  m_schedule->workCount = 0;
}

int Patch::portCount () const
{
  return 0;
}


Port* Patch::port (int idx) const
{
  Q_UNUSED(idx);
  // TODO: Implement registered-ports
  qFatal("Patch::port not implemented");
  return NULL;
}


Port* Patch::port (const QString& id) const
{
  Q_UNUSED(id);
  // TODO: Implement registered-ports
  qFatal("Patch::port not implemented");
  return NULL;
}


void Patch::activate (BufferProvider* bp)
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
  foreach (Processor* p, m_processors) {
    p->setBufferLength(type, len);
  }
}


void Patch::process (const ProcessingContext& context)
{
  Q_UNUSED(context);

  // We don't actually need to do anything, scheduler will
  // pull in our children as dependents
  for (int i=0; i<m_schedule->workCount; ++i) {
    Unison::Internal::WorkUnit& w = m_schedule->work[i];
    w.wait = w.initialWait;
  } 
}


const QSet<Node* const> Patch::dependencies () const
{
  QSet<Node* const> n;
  int count = portCount();
  for (int i=0; i<count; ++i) {
    Port* p  = port(i);
    if (p->direction() == Input) {
      n += p;
    }
  }
  return n;
}


const QSet<Node*  const> Patch::dependents () const
{
  QSet<Node*  const> n;
  int count = portCount();
  for (int i=0; i<count; ++i) {
    Port* p  = port(i);
    if (p->direction() == Output) {
      n += p;
    }
  }
  return n;
}


QString Patch::name () const
{
  // Don't translate me. If we want translations, then we must store the
  // name in the patch, so the name won't magically change if the project
  // is opened in a different locale
  return "Patch";
}


void Patch::add (Processor* processor)
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


void Patch::remove (Processor* processor)
{
  Q_ASSERT(processor != NULL);
  Q_ASSERT(processor->parent() == this);
  m_processors.removeOne(processor);
  processor->setParent(NULL);
  // XXX: Need to fix connections!
}


void Patch::compileSchedule (Internal::Schedule& output)
{
  qDebug() << "Compiling schedule for" << name()
           << "with" << m_processors.count() << "children.";
  // FIXME: this is not threadsafe, lock m_processors.
  output.workCount = m_processors.count();
  output.work = new Internal::WorkUnit[output.workCount];

  output.readyWorkCount = 1;
  output.readyWork = new Internal::WorkUnit[output.readyWorkCount];
  
  QListIterator<Processor*> i( m_processors );
  for (int wc=0; i.hasNext(); ++wc) {
    Processor* proc = i.next();

    // A rather naster traversal to find processors
    QSet<Node* const> dependencies;
    QSet<Node* const> dependents;

    // For each output port
    for (int pc = 0; pc < proc->portCount(); ++pc) {
      Port* port = proc->port(pc);

      // For all connected Ports.
      QSetIterator<Port* const> pi( port->connectedPorts() );
      while (pi.hasNext()) {
        Port* otherPort = pi.next();

        // We can count on getting processors from this relationship, but it sucks to.
        if (port->direction() == Input) {
          dependencies.unite(otherPort->dependencies());
        }
        else {
          dependents.unite(otherPort->dependents());
        }
      }
    }

    output.work[wc].processor = proc;
    output.work[wc].initialWait = dependencies.count() + 1; // dependents + patch;

    output.work[wc].dependents = new Internal::WorkUnit*[dependents.count()+1];
    int dc=0;
    for (QSetIterator<Node* const> ni(dependents); ni.hasNext(); ++dc) {
      int dwc = m_processors.indexOf((Processor*)ni.next());
      output.work[wc].dependents[dc] = &output.work[dwc];
    }
    output.work[wc].dependents[dc] = NULL; // NULL termination
  }

  // Now prepare the Patch gwork
  output.readyWork[0].processor = this;
  output.readyWork[0].initialWait = 0;
  output.readyWork[0].dependents = new Internal::WorkUnit*[output.workCount+1];
  int dc=0;
  for (; dc < output.workCount; ++dc) {
    output.readyWork[0].dependents[dc] = &output.work[dc];
  }
  output.readyWork[0].dependents[dc] = NULL; // NULL termination

  // Print it out
  /*
  printf("Compiled!!! \n");
  int wc;
  for (wc=0; wc< output.workCount; ++wc) {
    Unison::Internal::WorkUnit& w = output.work[wc];
    printf(" Work: `%s` (%x) wait: %d\n",
           qPrintable(w.processor->name()), w.processor, w.initialWait);
    for (Unison::Internal::WorkUnit** wpp = w.dependents; *wpp; ++wpp) {
      printf("  Dep: `%s` (%x) wait: %d\n",
             qPrintable((*wpp)->processor->name()), (*wpp)->processor, (*wpp)->initialWait);
    }
  }
  for (wc=0; wc< output.readyWorkCount; ++wc) {
    Unison::Internal::WorkUnit &w = output.readyWork[wc];
    printf(" Rdy Work: `%s` (%x) wait: %d\n", qPrintable(w.processor->name()), w.processor, w.initialWait);
    for (Unison::Internal::WorkUnit** wpp = w.dependents; *wpp; ++wpp) {
      printf("  Dep: `%s` (%x) wait: %d\n",
             qPrintable((*wpp)->processor->name()), (*wpp)->processor, (*wpp)->initialWait);
    }
  }
  printf("\n");
*/

}


/*
void Patch::compile (BufferProvider&  bufferProvider) {
  Q_ASSERT(QAtomicPointer< QList<CompiledProcessor> >
               ::isFetchAndStoreNative());

  QList<CompiledProcessor>* compiledSwap = new QList<CompiledProcessor>();
  qDebug() << name() << "compiling.";
  compile( m_processors,* compiledSwap );

  //qDebug() << "Aquiring 'fixed' buffers";
  qDebug() << name() << "Compiled, sequence is:";
  foreach (CompiledProcessor cp, *compiledSwap) {
    qDebug() << "  " << cp.processor->name();
    for (int i=0; i<cp.processor->portCount(); ++i) {
      Port* port = cp.processor->port(i);
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

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
