/*
 * Processor.cpp
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

#include "Processor.hpp"

#include "Patch.hpp"
#include "Port.hpp"
#include "types.hpp"

#include <QDebug>

namespace Unison {

Processor::Processor () :
  m_parent(NULL)
{}


Processor::~Processor ()
{
  if (m_parent) {
    m_parent->remove(this);
  }
}


void Processor::setBufferLength (PortType type, nframes_t len)
{
  for (int n = 0; n < portCount(); ++n) {
    Port* p = port(n);
    if (p->type() == type) {
      p->setBufferLength(len);
    }
  }
}


Node* Processor::parent () const
{
  return m_parent;
}


void Processor::setParent (Patch* parent)
{
  Q_ASSERT(parent != NULL);
  // TODO-NOW: A bunch of assertions
  m_parent = parent;
}


const QSet<Node* const> Processor::dependencies () const
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


const QSet<Node* const> Processor::dependents () const {
  QSet<Node* const> n;
  int count = portCount();
  for (int i=0; i<count; ++i) {
    Port* p  = port(i);
    if (p->direction() == Output) {
      n += p;
    }
  }
  return n;
}



} // Unison

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
