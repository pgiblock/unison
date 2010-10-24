/*
 * JackBackend.cpp
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

#include "JackBackend.h"
#include "JackPort.h"

#include <unison/Processor.h>

#include <QDebug>
#include <QWaitCondition>
#include <jack/jack.h>

// For connect-and-copy hackfest in processCb
#include <core/Engine.h>
#include <unison/AudioBuffer.h>
#include <unison/Commander.h>
#include <unison/Patch.h>
#include <unison/Scheduler.h>
#include "JackBufferProvider.h"

using namespace Unison;

namespace Jack {
  namespace Internal {

Backend* JackBackendProvider::createBackend()
{
  return new JackBackend();
}


JackBackend::JackBackend () :
  m_client(NULL),
  m_myPorts(),
  m_bufferLength(0),
  m_sampleRate(0),
  m_freewheeling(false),
  m_running(false)
{
  initClient();

  m_workers.workers = new Unison::Internal::Worker*[1];
  m_workers.workerCount = 1;
  m_workers.workers[0] = new Unison::Internal::Worker(m_workers);
}


JackBackend::~JackBackend ()
{
  jack_client_close(m_client);
}


void JackBackend::initClient ()
{
  QString name = tr("Unison Studio");

  // JACK stuff
  m_client = jack_client_open(name.toLatin1(), JackNullOption, NULL);
  m_running = false;
  if (!m_client) {
    qWarning("Failed to connect to JACK.");
  }
  else {
    qDebug("Connected to JACK.");
  }

  m_bufferLength = jack_get_buffer_size(m_client);
  m_sampleRate = jack_get_sample_rate(m_client);

  jack_on_shutdown (m_client, &JackBackend::shutdown, this);
  jack_set_buffer_size_callback(m_client, &JackBackend::bufferSizeCb, this);
  jack_set_freewheel_callback(  m_client, &JackBackend::freewheelCb, this);
  jack_set_graph_order_callback(m_client, &JackBackend::graphOrderCb, this);
  jack_set_process_callback(    m_client, &JackBackend::processCb, this);
  jack_set_sample_rate_callback(m_client, &JackBackend::sampleRateCb, this);
  jack_set_sync_callback(       m_client, &JackBackend::syncCb, this);
  jack_set_thread_init_callback(m_client, &JackBackend::threadInitCb, this);
  jack_set_xrun_callback(       m_client, &JackBackend::xrunCb, this );

  // TODO: Timebase CB..

}


bool JackBackend::activate ()
{
  if (!m_client) {
    qWarning("JACK backend cannot be activated while disconnected");
    return false;
  }

  if (!m_running) {
    m_running = (jack_activate(m_client) == 0);
  }

  return m_running;
}


bool JackBackend::deactivate ()
{
  if (m_client && m_running) {
    m_running = false;
    // TODO: Destroy ports!
    jack_deactivate(m_client);
  }
  return true;
}


bool JackBackend::reconnectToJack ()
{
  if (m_client) {
    disconnectFromJack();
    
    // wait 1/4 of a second for JACK to breathe
    QMutex dummy;
    dummy.lock();
    QWaitCondition sleep;
    sleep.wait(&dummy, 250);
  }

  initClient();

  for (int i=0; i<portCount(); ++i) {
    JackPort* p = port(i);

    // This is kind of dirty, the port should be already registered,
    // but the jack_port_t is dangling since we are disconnected.
    // so it is safe to overwrite it.
    // TODO: Instead check if it is in a zombie-state as set in disconnectFromJack
    if (p->isRegistered()) {
      if (p->registerPort()) {
        qDebug() << "Jack port re-registered: " << p->id();
      }
      else {
        qWarning() << "Jack port re-registration failed for port: " << p->id();
      }
    }
  }

  // TODO: return a proper return code
  return true;
}


bool JackBackend::disconnectFromJack ()
{
  if (m_client) {
    jack_client_close(m_client);
    m_client = NULL;
  }

  // TODO: Put ports in some zombie state

  m_running = false;
  // Return true if we are disconnected
  return true;
}


JackPort* JackBackend::registerPort (const QString& name, PortDirection direction)
{
  JackPort* myPort = new JackPort( *this, name, direction);

  if (!myPort->registerPort()) {
    qWarning() << "Jack port registration failed for port: " << name;
    delete myPort;
    return NULL;
  }
  else {
    qDebug() << "Jack port registered: " << myPort->name();
    m_myPorts.append( myPort );
    return myPort;
  }
}


void JackBackend::unregisterPort (BackendPort* port)
{
  JackPort* jackPort = dynamic_cast<JackPort*>(port);
  //Q_ASSERT_X(jackPort !=  NULL, "Jack Backend", "cannot unregister a non-Jack port");
  unregisterPort(jackPort);
}


void JackBackend::unregisterPort (JackPort* port)
{
  jack_port_unregister(client(), port->jackPort());
  delete port;
}


nframes_t JackBackend::bufferLength () const
{
  return m_bufferLength;
}


nframes_t JackBackend::sampleRate () const
{
  return m_sampleRate;
}


bool JackBackend::isFreewheeling () const
{
  return m_freewheeling;
}


int JackBackend::portCount () const
{
  return m_myPorts.count();
}


JackPort* JackBackend::port (int index) const
{
  return m_myPorts[index];
}


JackPort* JackBackend::port (const QString& name) const
{
  return NULL; // TODO: implement
}


int JackBackend::connect (const QString& source, const QString& dest)
{
  return 0;
}


int JackBackend::disconnect (const QString& source, const QString& dest)
{
  return 0;
}


int JackBackend::disconnect (Unison::BackendPort* port)
{
  return 0;
}


void JackBackend::shutdown (void* a)
{
  JackBackend* backend = static_cast<JackBackend*>(a);
  backend->m_running = false;
  // probably need to signal
}




int JackBackend::bufferSizeCb (nframes_t nframes, void* a)
{
  JackBackend* backend = static_cast<JackBackend*>(a);
  qDebug() << "JACK buffer size changed";
  backend->m_bufferLength = nframes;
  // TODO-NOW: somehow update or inform ports/buffers of bufferSize change.
  return 0;
}


void JackBackend::freewheelCb (int starting, void* a)
{
  JackBackend* backend = static_cast<JackBackend*>(a);
  qDebug() << "JACK freewheeling " << starting;
  qCritical() << "Freewheeling mode currently not supported";
  backend->m_freewheeling = starting;
}


int JackBackend::graphOrderCb (void* a)
{
  JackBackend* backend = static_cast<JackBackend*>(a);
  qDebug() << "JACK graph order changed";
  // TODO-NOW: somehow ensure our graph is recompiled.
  return 0;
}


int JackBackend::processCb (nframes_t nframes, void* a)
{
  JackBackend* backend = static_cast<JackBackend*>(a);
  JackBufferProvider nullProvider;
  int i;

  if (!(backend->m_running)) {
    return 0;
  }

  ProcessingContext context( nframes );

  Unison::Internal::Commander::instance()->process(context);


  // Aquire JACK buffers
  for (i=0; i<backend->portCount(); ++i) {
    Port* port = backend->port(i);
    port->connectToBuffer();

    // Re-acquire buffers on ports connected to JACK
    // Read note below:
    foreach (Port* other, port->connectedPorts()) {
      other->connectToBuffer();
    }

    // Copy data across directly connected jack buffers.
    // XXX: TODO: This is a super-hack.  In retrospect, it would be better if JackPort
    // simply didn't have any Buffer at all.  Just copy all the data to connected Ports
    // before process()ing, Then Fill in outgoing ports on the way out.  This
    // fixes both connections to regular Ports and to JackPorts.  It also let's
    // us remove JackBufferProvider and silly calls to connectToBuffer()...
  }

  // Run our only worker
  Unison::Internal::Worker* worker = backend->m_workers.workers[0];

  Unison::Internal::Schedule* s = backend->rootPatch()->schedule();
  worker->pushReadyWorkUnsafe(s->readyWork, s->readyWorkCount);
  worker->run(context);

  //Q_ASSERT(backend->rootPatch());
  //backend->rootPatch()->process(context);

  return 0;
}


int JackBackend::sampleRateCb (nframes_t nframes, void* a) {
  JackBackend* backend = static_cast<JackBackend*>(a);
  qDebug() << "JACK sampling rate changed";
  qCritical() << "Sampling rate changes currently not supported";
  backend->m_sampleRate = nframes;
  // TODO: inform Session, Sequencer, MetricMap about sampleRate change.
  return 0;
}


int JackBackend::syncCb (jack_transport_state_t, jack_position_t*, void* eng) {
  //qDebug() << "JACK sync";
  return 0;
}


void JackBackend::threadInitCb (void* backend) {
  qDebug() << "JACK thread init";
}


void JackBackend::timebaseCb (jack_transport_state_t, nframes_t, jack_position_t*, int, void*) {
  //qCritical() << "Timebase master currently not supported";
}


int JackBackend::xrunCb (void* backend) {
  //qWarning() << "XRun occured";
  return 0;
}

  } // Internal
} // Jack


// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
