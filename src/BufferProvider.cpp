/*
 * BufferProvider.cpp
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

#include "unison/BufferProvider.h"
#include "unison/AudioBuffer.h"
#include "unison/ControlBuffer.h"

namespace Unison
{

PooledBufferProvider::PooledBufferProvider () :
  m_audioBuffers(),
  m_controlBuffers(),
  m_zeroBuffer(NULL),
  m_periodLength(),
  m_next(0)
{}


SharedBufferPtr PooledBufferProvider::acquire (
    PortType type, nframes_t nframes)
{
  //TODO assert(nframes == m_periodLength); (or whatever)
  QStack<Buffer*>* stack;
  switch (type)
  {
    case AUDIO_PORT:
      stack = &m_audioBuffers;
      break;
    case CONTROL_PORT:
      stack = &m_controlBuffers;
      break;
    default:
      // TODO: assert(false);
      return NULL;
  }

  if (!stack->isEmpty())
  {
    return stack->pop();
  }

  //TODO ensure we are not in processing thread
  switch (type)
  {
    case AUDIO_PORT:
      std::cout << "New Audio Buffer " << nframes << " frames." << std::endl;
      return new AudioBuffer(*this, nframes);
    case CONTROL_PORT:
      std::cout << "New Control Buffer" << std::endl;
      return new ControlBuffer(*this);
    default:
      std::cout << "Couldn't create unknown port" << std::endl;
      // TODO: assert(false);
      return NULL;
  }
}


SharedBufferPtr PooledBufferProvider::zeroAudioBuffer () const
{
  return m_zeroBuffer;
}

void PooledBufferProvider::setBufferLength (nframes_t nframes)
{
  m_periodLength = nframes;

  m_zeroBuffer = acquire (AUDIO_PORT, nframes);
}

nframes_t PooledBufferProvider::bufferLength ()
{
  return m_periodLength;
}

void PooledBufferProvider::release (Buffer * buf)
{
  switch (buf->type())
  {
    case AUDIO_PORT:
      if (((AudioBuffer*)buf)->length() != bufferLength())
      {
        // FIXME
        std::cout << "Deleting buffer, wrong size!" << std::endl;
        delete buf;
      }
      else
      {
        m_audioBuffers.push(buf);
      }
      break;

    case CONTROL_PORT:
      m_controlBuffers.push(buf);
      break;

    default:
      // TODO: Unhandled port!!
      break;
  }
}

} // Unison

// vim: et ts=8 sw=2 sts=2 noai
