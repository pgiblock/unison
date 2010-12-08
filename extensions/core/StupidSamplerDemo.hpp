/*
 * StupidSamplerDemo.hpp
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

#ifndef STUPID_SAMPLER_DEMO_HPP
#define STUPID_SAMPLER_DEMO_HPP

#include <QString>

#include <unison/Port.hpp>
#include <unison/Processor.hpp>

namespace Unison {
  class BackendPort;
  class BufferProvider;
  class Patch;
  class SampleBuffer;
}

namespace Core {
  namespace Demo {

class Sampler;

class SamplerPort : public Unison::Port
{
  public:
    SamplerPort (Sampler *sampler, const QString &id, const QString &name);
    ~SamplerPort ();

    QString id () const;
    QString name () const;

    Unison::PortType type () const;
    Unison::PortDirection direction () const;

    float value () const;
    void setValue (float value);

    float defaultValue () const;

    bool isBounded () const;

    float minimum () const;
    float maximum () const;

    bool isToggled () const;

    Node* parent () const;

    const QSet<Unison::Node* const> interfacedNodes () const;

    void connectToBuffer ();

  private:
    Sampler *m_parent;
    QString m_id;
    QString m_name;
};


class Sampler : public Unison::Processor
{
  public:
    Sampler (QString name);
    int portCount () const;
    Unison::Port* port (int idx) const;
    Unison::Port* port (const QString& name) const;
    void activate (Unison::BufferProvider& bp);
    void deactivate ();
    void setSampleBuffer (Unison::SampleBuffer *buff);
    void process (const Unison::ProcessingContext& context);
    QString name() const
    {
      return m_name;
    }

  private:
    QString m_name;
    Unison::SampleBuffer* m_sampleBuff;
    Unison::Port* m_ports[2];
    double m_cnt;
    Unison::nframes_t m_fcnt;

};

/** A stupid sampler demo.
 *
 * This lets someone test sample-buffer loading code. LV2 doesn't have a
 * feature that allows us to pass in a "waveform".  We will develop this
 * functionality, but this is a quick hack for testing purposes. */
class StupidSamplerDemo
{


  public:
    StupidSamplerDemo (Unison::Patch *parent, QString name);
    ~StupidSamplerDemo ();

    QString name () const
    {
      return m_name;
    }

    void setSampleBuffer (Unison::SampleBuffer *buff);

  private:
    QString m_name;
    Sampler* m_sampler;
    Unison::BackendPort* m_outPorts[2];
};

  } // Demo
} // Core

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
