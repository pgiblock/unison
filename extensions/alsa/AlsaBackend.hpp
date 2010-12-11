/*
 * AlsaBackend.hpp
 *
 * Copyright (c) 2010 Andrew Kelley <superjoe30+unison@gmail.com>
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

#ifndef UNISON_ALSA_BACKEND_H
#define UNISON_ALSA_BACKEND_H

#include "AlsaPort.hpp"

#include <unison/Backend.hpp>
#include <core/IBackendProvider.hpp>

#include <QObject>
#include <QVarLengthArray>

#include <alsa/asoundlib.h>

namespace Unison {
  class BufferProvider;
}

namespace Alsa {
  namespace Internal {

class AlsaBackendProvider : public Core::IBackendProvider
{
  Q_OBJECT
  public:
    AlsaBackendProvider(QObject* parent, QString inputDevice, QString outputDevice) :
      Core::IBackendProvider(parent),
      m_inputDevice(inputDevice),
      m_outputDevice(outputDevice)
    {}

    ~AlsaBackendProvider()
    {}

    QString displayName() const
    {
      return "alsa";
    }

    Unison::Backend* createBackend();

  private:
    QString m_inputDevice;
    QString m_outputDevice;
};



/**
 * AlsaBackend encapsulates ALSA compatibility.
 */
class AlsaBackend : public Unison::Backend
{
  Q_OBJECT

  public:
    AlsaBackend (Unison::BufferProvider& bp, QString inputDevice, QString outputDevice);
    virtual ~AlsaBackend();

    Unison::nframes_t bufferLength() const;
    Unison::nframes_t sampleRate() const;
    bool isFreewheeling() const { Q_ASSERT(false); return false; }

    AlsaPort* registerPort (const QString& name, Unison::PortDirection direction);

    void unregisterPort(Unison::BackendPort*);

    bool activate();
    bool deactivate();

    int portCount() const { return m_ports.count(); }
    AlsaPort* port(int index) const { return m_ports[index]; }
    AlsaPort* port(const QString& name) const { Q_ASSERT(false); return NULL; }

    int connect(const QString& source, const QString& dest) { Q_ASSERT(false); return 0; }
    int disconnect(const QString& source, const QString& dest) { Q_ASSERT(false); return 0; }
    int disconnect(Unison::BackendPort*) { Q_ASSERT(false); return 0; }

  private:
    Unison::BufferProvider & m_bufferProvider;
    QString m_inputDevice;
    QString m_outputDevice;
    QVarLengthArray<AlsaPort *> m_ports;

    // from microphone
    snd_pcm_t * m_pcmInput;
    // to speakers
    snd_pcm_t * m_pcmOutput;

    Unison::nframes_t m_bufferLength;
    Unison::nframes_t m_sampleRate;

    /** The interval between interrupts from the hardware.
    / * this defines the input latency. Higher values give the CPU a break, lower values
    / * reduce latency.*/
    Unison::nframes_t m_periodSize;

    /** @returns success
      */
    bool connectToAlsa();
    /** @returns success
      */
    bool disconnectFromAlsa();
    int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params);
    int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);
};

  } // Internal
} // Jack

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
