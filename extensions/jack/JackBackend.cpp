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
#include <QThread>
#include <QWaitCondition>
#include <jack/jack.h>

// For connect-and-copy hackfest in processCb
#include <core/Engine.h>
#include <unison/AudioBuffer.h>
#include <unison/Commander.h>
#include <unison/Patch.h>
#include <unison/Scheduler.h>

// For pthread hack - abstract to platform-agnostic utils
#include <pthread.h>
#include <mcheck.h>

using namespace Unison;

namespace Jack {
  namespace Internal {

class JackWorkerThread : public QThread
{
  public:
    JackWorkerThread (Unison::Internal::Worker* w, QSemaphore& done);
    void run ();

    void run (ProcessingContext& ctx);

  protected:
    void setSchedulingPriority (int policy, unsigned int priority);

    Unison::Internal::Worker* m_worker;
    QSemaphore m_wait;
    // Shared state:
    QSemaphore& m_done;
    ProcessingContext* m_context;
};

JackWorkerThread::JackWorkerThread (Unison::Internal::Worker* w, QSemaphore& done) :
  m_worker(w),
  m_wait(),
  m_done(done)
{}


void JackWorkerThread::setSchedulingPriority (int policy, unsigned int priority)
{
  sched_param sp;
  sp.sched_priority = priority;
  pthread_t thread = pthread_self();
  int result = pthread_setschedparam(thread, policy, &sp);
  if (!result) {
    QDebug dbg = qDebug();
    dbg << "Setting JWT scheduling policy:";
    switch (policy) {
      case SCHED_FIFO:  dbg << "SCHED_FIFO";  break;
      case SCHED_RR:    dbg << "SCHED_RR";    break;
      case SCHED_OTHER: dbg << "SCHED_OTHER"; break;
#ifdef SCHED_BATCH
      case SCHED_BATCH: dbg << "SCHED_BATCH"; break;
#endif
      default:          dbg << "Unknown";     break;
    }
    dbg << "and priority:" << sp.sched_priority;
  }
  else {
    qDebug() << "Unable to set scheduling policy";
  }
}


void JackWorkerThread::run ()
{
  qDebug() << "JWT: started!\n";
  setSchedulingPriority(SCHED_FIFO, 10);

  while (true) {
    m_wait.acquire();

    m_worker->run(*m_context);

    m_done.release(1);
  }
  // exec(); We don't want event handling
}

void JackWorkerThread::run (ProcessingContext& ctx)
{
  m_context = &ctx;
  m_wait.release();
}


Backend* JackBackendProvider::createBackend()
{
  return new JackBackend(*Core::Engine::bufferProvider(), m_workerCount);
}


JackBackend::JackBackend (Unison::BufferProvider& bp, int workerCount) :
  m_client(NULL),
  m_myPorts(),
  m_bufferProvider(bp),
  m_bufferLength(0),
  m_sampleRate(0),
  m_freewheeling(false),
  m_running(false)
{
  Q_ASSERT(workerCount > 0);
  initClient();

  m_workers.workers = new Unison::Internal::Worker*[workerCount];
  m_workers.workerCount = workerCount;
  for (int i=0; i<workerCount; ++i) {
    m_workers.workers[i] = new Unison::Internal::Worker(m_workers);
  }

  m_workerThreads.reserve(workerCount-1);
  for (int i=0; i<workerCount-1; ++i) {
    m_workerThreads.append(new JackWorkerThread(m_workers.workers[i], m_workersDone));
    ((JackWorkerThread*)m_workerThreads[i])->start();
  }
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
    myPort->activate( m_bufferProvider );
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
  return 0;
}


int JackBackend::processCb (nframes_t nframes, void* a)
{
  JackBackend* backend = static_cast<JackBackend*>(a);
  int i;

  mtrace();
  if (!(backend->m_running)) {
    muntrace();
    return 0;
  }

  ProcessingContext context( nframes );

  // Process commands
  Unison::Internal::Commander::instance()->process(context);

  Unison::Internal::Worker** workers = backend->m_workers.workers;
  Unison::Internal::Schedule* s = backend->rootPatch()->schedule();

  for (i=0; i<backend->portCount(); ++i) {
    backend->port(i)->preProcess();
  }

  // TODO: Remove this check, it is rather a hack
  if (s->readyWorkCount!=0) {

    backend->m_workers.workLeft = s->readyWorkCount;

    int numThreads = backend->m_workerThreads.size();
    workers[numThreads]->pushReadyWorkUnsafe(s->readyWork, s->readyWorkCount);

    // TODO: Switching on this sucks, should use a funcptr or template for 0 vs 1+
    /*
    if (numThreads) {
      for (int i=0; i < numThreads; ++i) {
        ((JackWorkerThread*)backend->m_workerThreads[i])->run(context); // unblock slave
      }

      // Run us
      workers[numThreads]->run(context);

      // join with slaves TODO: spin on a count?
      backend->m_workersDone.acquire(numThreads);
    }
    else {
    */
      workers[numThreads]->run(context);
    //}
  
  } // End hacky conditional


  for (i=0; i<backend->portCount(); ++i) {
    backend->port(i)->postProcess();
  }

  muntrace();
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
  qWarning() << "XRun occured";
  return 0;
}

  } // Internal
} // Jack


// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
