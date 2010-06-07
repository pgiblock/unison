/*
 * JackEngine.cpp
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
#include <jack/jack.h>

#include "unison/JackEngine.h"
#include "unison/Session.h"

namespace Unison
{

JackEngine::JackEngine () :
  m_session(NULL),
  m_client(NULL),
  m_myPorts(),
  m_bufferLength(0),
  m_sampleRate(0),
  m_freewheeling(false),
  m_running(false)
{
  initClient();
}


JackEngine::~JackEngine ()
{
  jack_client_close(m_client);
}


void JackEngine::initClient()
{
  QString name = tr("Unison Studio");

  // JACK stuff
  m_client = jack_client_open(name.toLatin1(), JackNullOption, NULL);
  if (!m_client) {
    // TODO-NOW: Fail more gracefully
    qFatal("Failed to connect to JACK.");
  }
  else {
    qDebug() << "Connected to JACK.\n";
  }

  m_bufferLength = jack_get_buffer_size(m_client);
  m_sampleRate = jack_get_sample_rate(m_client);

  jack_on_shutdown (m_client, &JackEngine::shutdown, this);
  jack_set_buffer_size_callback(m_client, &JackEngine::bufferSizeCb, this);
  jack_set_freewheel_callback(  m_client, &JackEngine::freewheelCb, this);
  jack_set_graph_order_callback(m_client, &JackEngine::graphOrderCb, this);
  jack_set_process_callback(    m_client, &JackEngine::processCb, this);
  jack_set_sample_rate_callback(m_client, &JackEngine::sampleRateCb, this);
  jack_set_sync_callback(       m_client, &JackEngine::syncCb, this);
  jack_set_thread_init_callback(m_client, &JackEngine::threadInitCb, this);
  jack_set_xrun_callback(       m_client, &JackEngine::xrunCb, this );

  // TODO: Timebase CB..

}


void JackEngine::activate ()
{
  if (!m_running) {
    m_running = jack_activate(m_client) == 0;
  }
}


void JackEngine::deactivate ()
{
  jack_deactivate(m_client);
  m_running = false;
}


void JackEngine::setSession (Session * session) {
  if (!m_session) {
    m_session = session;
  }
}


void JackEngine::removeSession () {
  // Should probably queue this action until processing()
  qWarning() << "JackEngine.removeSession currently unimplemented.";
}


JackPort* JackEngine::registerPort (QString name, PortDirection direction)
{
  // Build Jack flags, currently just direction. Which, is reversed since
  // our Ports' directions are relative to Unison's connections but
  // jack_port_t's direction is relative to Jack's graph.
  JackPortFlags flag;
  switch (direction) {
    case INPUT:
      flag = JackPortIsOutput;
      break;
    case OUTPUT:
    default:
      flag = JackPortIsInput;
      break;
  }

  jack_port_t* port = jack_port_register(
      client(), name.toLatin1(), JACK_DEFAULT_AUDIO_TYPE, flag, 0 );
  if (port) {
    JackPort* myPort = new JackPort( *this, port );
    m_myPorts.append( myPort );
    qDebug() << "Jack port registered: " << myPort->name();
    return myPort;
  }
  qWarning() << "Jack port registration failed for port: " << name;
  return NULL;
}

void JackEngine::unregisterPort (JackPort * port)
{
  jack_port_unregister(client(), port->jackPort());
  delete port;
}


nframes_t JackEngine::bufferLength () const
{
  return m_bufferLength;
}


nframes_t JackEngine::sampleRate () const
{
  return m_sampleRate;
}


bool JackEngine::isFreewheeling () const
{
  return m_freewheeling;
}


int JackEngine::portCount () const
{
  return m_myPorts.count();
}


JackPort* JackEngine::port (int index) const
{
  return m_myPorts[index];
}


JackPort* JackEngine::port (QString name) const
{
  return NULL; // TODO: implement
}


void JackEngine::shutdown (void* a)
{
  JackEngine* engine = static_cast<JackEngine*>(a);
  engine->m_running = false;
  // probably need to signal
}


int JackEngine::bufferSizeCb (nframes_t nframes, void* a)
{
  JackEngine* engine = static_cast<JackEngine*>(a);
  qDebug() << "JACK buffer size changed";
  qCritical() << "Buffer size changes currently not supported";
  engine->m_bufferLength = nframes;
  // TODO-NOW: somehow update or inform ports/buffers of bufferSize change.
  return 0;
}


void JackEngine::freewheelCb (int starting, void* a)
{
  JackEngine* engine = static_cast<JackEngine*>(a);
  qDebug() << "JACK freewheeling " << starting;
  qCritical() << "Freewheeling mode currently not supported";
  engine->m_freewheeling = starting;
}


int JackEngine::graphOrderCb (void* a)
{
  JackEngine* engine = static_cast<JackEngine*>(a);
  qDebug() << "JACK graph order changed";
  // TODO-NOW: somehow ensure our graph is recompiled.
  return 0;
}


int JackEngine::processCb (nframes_t nframes, void* a)
{
  JackEngine* engine = static_cast<JackEngine*>(a);
  if (engine->m_session) {

    // Aquire JACK buffers
    for (int i=0; i<engine->portCount(); ++i) {
      Port *port = engine->port(i);
      port->connectToBuffer(engine->m_session->bufferProvider());

      // Re-acquire buffers on ports connected to JACK
      foreach (Port *other, port->connectedPorts()) {
        other->connectToBuffer(engine->m_session->bufferProvider());
      }
    }

    ProcessingContext context( nframes );
    engine->m_session->process(context);

  }
  return 0;
}


int JackEngine::sampleRateCb (nframes_t nframes, void* a) {
  JackEngine* engine = static_cast<JackEngine*>(a);
  qDebug() << "JACK sampling rate changed";
  qCritical() << "Sampling rate changes currently not supported";
  engine->m_sampleRate = nframes;
  // TODO: inform Session, Sequencer, MetricMap about sampleRate change.
  return 0;
}


int JackEngine::syncCb (jack_transport_state_t, jack_position_t*, void* eng) {
  //qDebug() << "JACK sync";
  return 0;
}


void JackEngine::threadInitCb (void* engine) {
  qDebug() << "JACK thread init";
}


void JackEngine::timebaseCb (jack_transport_state_t, nframes_t, jack_position_t*, int, void*) {
  //qCritical() << "Timebase master currently not supported";
}


int JackEngine::xrunCb (void* engine) {
  //qWarning() << "XRun occured";
  return 0;
}


} // Unison

// vim: ts=8 sw=2 sts=2 et sta noai
