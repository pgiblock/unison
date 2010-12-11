/*
 * AlsaBackend.cpp
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

#include "AlsaBackend.hpp"

#include <core/Engine.hpp>

#include <QDebug>

using namespace Unison;

namespace Alsa {
  namespace Internal {

Unison::Backend* AlsaBackendProvider::createBackend()
{
  return new AlsaBackend(*Core::Engine::bufferProvider(), m_inputDevice, m_outputDevice);
}


AlsaBackend::AlsaBackend(Unison::BufferProvider &bp, QString inputDevice, QString outputDevice) :
  m_bufferProvider(bp),
  m_inputDevice(inputDevice),
  m_outputDevice(outputDevice),
  m_ports(),
  m_pcmInput(NULL),
  m_pcmOutput(NULL),
  m_bufferLength(0),
  m_sampleRate(0),
  m_periodSize(0)
{
  connectToAlsa();
}

AlsaBackend::~AlsaBackend ()
{
  disconnectFromAlsa();
}

int AlsaBackend::set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params)
{
  int err, dir;

  // choose all parameters
  err = snd_pcm_hw_params_any(handle, params);
  if (err) {
    qCritical() << "Broken configuration for playback: no configurations available: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }

  // set hardware resampling
  err = snd_pcm_hw_params_set_rate_resample(handle, params, 1);
  if (err) {
    qCritical() << "Resampling setup failed for playback: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }

  // set the interleaved read/write format
  err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  if (err) {
    qCritical() << "Access type not available for playback: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }

  // set the sample format
  err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT);
  if (err) {
    qCritical() << "Sample format not available for playback: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }

  // set the count of channels
  int channelCount = 2;
  err = snd_pcm_hw_params_set_channels(handle, params, channelCount);
  if (err) {
    qCritical() << "Channel count " << channelCount << " not available for playbacks: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }

  // set the stream rate
  unsigned int requestedRate = sampleRate();
  unsigned int actualRate = requestedRate;
  err = snd_pcm_hw_params_set_rate_near(handle, params, &actualRate, 0);
  if (err) {
    qCritical() << "Rate " << requestedRate << "Hz not available for playback: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }
  if (requestedRate != actualRate) {
    qCritical() << "Rate doesn't match (requested " << requestedRate << "Hz, get " << actualRate << "Hz)";
    Q_ASSERT(false);
    return -EINVAL;
  }

  // set the buffer time
  unsigned int buffer_time_nanoseconds = 1000000 * 10; // 10 ms lag. TODO: make this dynamically configurable at runtime
  err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time_nanoseconds, &dir);
  if (err) {
    qCritical() << "Unable to set buffer time " << buffer_time_nanoseconds << " nanosec for playback: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }

  snd_pcm_uframes_t bufferLength;
  err = snd_pcm_hw_params_get_buffer_size(params, &bufferLength);
  if (err) {
    qCritical() << "Unable to get buffer size for playback: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }
  m_bufferLength = (Unison::nframes_t) bufferLength;

  // set the period time.
  unsigned int period_time = 100000;
  err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
  if (err) {
    qCritical() << "Unable to set period time " << period_time << " for playback: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }
  snd_pcm_uframes_t period_size;
  err = snd_pcm_hw_params_get_period_size(params, &period_size, &dir);
  if (err) {
    qCritical() << "Unable to get period size for playback: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }

  // write the parameters to device
  err = snd_pcm_hw_params(handle, params);
  if (err) {
    qCritical() << "Unable to set hw params for playback: " << snd_strerror(err);
    Q_ASSERT(false);
    return err;
  }

  return 0; // no error
}

int AlsaBackend::set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
  int err;

  // get the current swparams
  err = snd_pcm_sw_params_current(handle, swparams);
  if (err) {
    qCritical() << "Unable to determine current swparams for playback: " << snd_strerror(err);
    return err;
  }

  // start the transfer when the buffer is almost full:
  // (buffer_size / avail_min) * avail_min
  err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (m_bufferLength / m_periodSize) * m_periodSize);
  if (err) {
    qCritical() << "Unable to set start threshold mode for playback: " << snd_strerror(err);
    return err;
  }

  // allow the transfer when at least period_size samples can be processed
  // or disable this mechanism when period event is enabled (aka interrupt like style processing)
  err = snd_pcm_sw_params_set_avail_min(handle, swparams, (snd_pcm_uframes_t) m_periodSize);
  if (err) {
    qCritical() << "Unable to set avail min for playback: " << snd_strerror(err);
    return err;
  }

  // write the parameters to the playback device
  err = snd_pcm_sw_params(handle, swparams);
  if (err) {
    qCritical() << "Unable to set sw params for playback: " << snd_strerror(err);
    return err;
  }

  return 0; // no error
}


bool AlsaBackend::connectToAlsa()
{
  int err;

  snd_pcm_hw_params_t * hwparams = NULL;
  snd_pcm_sw_params_t * swparams = NULL;

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);

  Q_ASSERT(hwparams != NULL);
  Q_ASSERT(swparams != NULL);

  err = snd_pcm_open(&m_pcmOutput, m_outputDevice.toAscii(), SND_PCM_STREAM_PLAYBACK, 0);
  if (err) {
    qCritical() << "Playback open error: " << snd_strerror(err);
    Q_ASSERT(false);
    return false;
  }

  err = set_hwparams(m_pcmOutput, hwparams);
  if (err)
    return false;

  err = set_swparams(m_pcmOutput, swparams);
  if (err)
    return false;

  return true;
}

bool AlsaBackend::disconnectFromAlsa()
{
  // close the device
  snd_pcm_close(m_pcmOutput);
  m_pcmOutput = NULL;

  return true;
}

bool AlsaBackend::activate ()
{
  return connectToAlsa();
}


bool AlsaBackend::deactivate ()
{
  return disconnectFromAlsa();
}

AlsaPort* AlsaBackend::registerPort (const QString& name, PortDirection direction)
{
  AlsaPort* port = new AlsaPort(*this, name, direction);

  if (!port->registerPort()) {
    qWarning() << "Alsa port registration failed for port: " << name;
    Q_ASSERT(false);
    delete port;
    return NULL;
  } else {
    qDebug() << "Alsa port registered: " << port->name();
    port->activate(m_bufferProvider);
    m_ports.append(port);
    return port;
  }
}

void AlsaBackend::unregisterPort (BackendPort* port)
{
  AlsaPort * alsa_port = dynamic_cast<AlsaPort*>(port);
  // TODO: actually remove the port from m_ports
  // and use a better data structure to avoid O(n) deletion
  delete alsa_port;
}

nframes_t AlsaBackend::bufferLength () const
{
  return m_bufferLength;
}


nframes_t AlsaBackend::sampleRate () const
{
  return m_sampleRate;
}

  } // Internal
} // Alsa


// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
