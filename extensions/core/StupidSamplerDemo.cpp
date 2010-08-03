/*
 * StupidSamplerDemo.cpp
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

#include "Engine.h"
#include "StupidSamplerDemo.h"
#include <unison/Backend.h>
#include <unison/BackendPort.h>
#include <unison/BufferProvider.h>
#include <unison/Patch.h>
#include <unison/SampleBuffer.h>

#define UNISON_BUFFER_LENGTH 1024

using namespace Unison;

namespace Core {
  namespace Demo {


SamplerPort::SamplerPort (Sampler *sampler, const QString &id, const QString &name) :
  m_parent(sampler),
  m_id(id),
  m_name(name)
{}

SamplerPort::~SamplerPort ()
{}

QString SamplerPort::id () const
{
  return m_id;
}

QString SamplerPort::name () const
{
  return m_name;
}

Unison::PortType SamplerPort::type () const
{
  return AUDIO_PORT;
}

Unison::PortDirection SamplerPort::direction () const
{
  return OUTPUT;
}

float SamplerPort::value () const
{
  return 0.0f;
}

void SamplerPort::setValue (float value)
{
  Q_UNUSED(value);
}

float SamplerPort::defaultValue () const
{
  return 0.0f;
}

bool SamplerPort::isBounded () const
{
  return true;
}

float SamplerPort::minimum () const
{
  return 0.0f;
}

float SamplerPort::maximum () const
{
  return 0.0f;
}

bool SamplerPort::isToggled () const
{
  return false;
}

Node* SamplerPort::parent () const
{
  return m_parent;
}

const QSet<Unison::Node* const> SamplerPort::interfacedNodes () const
{
  QSet<Node* const> p;
  p.insert( m_parent );
  return p;
}

void SamplerPort::connectToBuffer ()
{
  BufferProvider *provider = m_parent->bufferProvider();
  switch (direction()) {
    case INPUT:
      acquireInputBuffer(*provider, UNISON_BUFFER_LENGTH);
      break;

    case OUTPUT:
      acquireOutputBuffer(*provider, UNISON_BUFFER_LENGTH);
      break;
  }
  // TODO: Do something!
}


Sampler::Sampler(QString name) :
  Processor(),
  m_name(name),
  m_sampleBuff(NULL),
  m_cnt(0.0f),
  m_fcnt(0)
{
  m_ports[0] = new SamplerPort(this, "left", "left");
  m_ports[1] = new SamplerPort(this, "right", "right");
}

int Sampler::portCount () const
{
  return 2;
}

Port* Sampler::port (int idx) const
{
  return m_ports[idx];
}

Port* Sampler::port (QString name) const
{
  // TODO: Find port with name
}

void Sampler::activate (BufferProvider *bp)
{
  m_bufferProvider=bp;
}

void Sampler::deactivate ()
{
}

void Sampler::setSampleBuffer (SampleBuffer *buff)
{
  m_sampleBuff = buff;
}

void Sampler::process (const ProcessingContext & context)
{
  if (!m_sampleBuff) {
    return;
  }

  sample_t *data0 = (sample_t*)(m_ports[0]->buffer()->data());
  sample_t *data1 = (sample_t*)(m_ports[1]->buffer()->data());

  // Sample looper
  for(int i=0; i< context.bufferSize(); ++i) {
    sample_t s = m_sampleBuff->samples()[m_fcnt];
    data0[i] = s;
    data1[i] = s;
    // Advance
    m_fcnt= (m_fcnt+1)%(m_sampleBuff->frames());
  }

}

Unison::BufferProvider *Sampler::bufferProvider ()
{
  return m_bufferProvider;
}




StupidSamplerDemo::StupidSamplerDemo (Patch* parent, QString name) :
  m_name(name)
{
  m_outPorts[0] = Engine::backend()->registerPort(name + "/out 1", INPUT);
  m_outPorts[1] = Engine::backend()->registerPort(name + "/out 2", INPUT);

  m_sampler = new Sampler(name);

  m_sampler->activate(Engine::bufferProvider());
  parent->add(m_sampler);

  m_sampler->port(0)->connect(m_outPorts[0]);
  m_sampler->port(1)->connect(m_outPorts[1]);
}


StupidSamplerDemo::~StupidSamplerDemo ()
{
  Engine::backend()->unregisterPort(m_outPorts[0]);
  Engine::backend()->unregisterPort(m_outPorts[1]);
}


void StupidSamplerDemo::setSampleBuffer (Unison::SampleBuffer *buff)
{
  m_sampler->setSampleBuffer(buff);
}


  } // Demo
} // Core

// vim: ts=8 sw=2 sts=2 et sta noai
