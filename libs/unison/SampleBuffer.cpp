/*
 * SampleBuffer.cpp
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

#include "unison/SampleBuffer.hpp"

#include <QtGlobal>
#include <cstring>

namespace Unison {

const int SampleBuffer::DEFAULT_CHANNELS;
const int SampleBuffer::LEFT_CHANNEL;
const int SampleBuffer::RIGHT_CHANNEL;

SampleBuffer::SampleBuffer (float* buf, int frames, int channels, int samplerate) :
  m_data(0),
  m_frames(frames),
  m_channels(channels),
  m_samplerate(samplerate)
{
  Q_ASSERT(channels > 0);
  if (frames > 0) {
    m_data = new sample_t[totalFrames()];
    std::memcpy(m_data, buf, totalFrames()*sizeof(sample_t));
  }
}


SampleBuffer::SampleBuffer (int frames, int channels, int samplerate) :
  m_data(0),
  m_frames(frames),
  m_channels(channels),
  m_samplerate(samplerate)
{
  Q_ASSERT(channels > 0);
  if (frames > 0) {
    m_data = new sample_t[totalFrames()];
    std::memset(m_data, 0x00, totalFrames()*sizeof(sample_t));
  }
}


SampleBuffer::SampleBuffer (const SampleBuffer& sb) :
    m_data(0),
    m_frames(sb.m_frames),
    m_channels(sb.m_channels),
    m_samplerate(sb.m_samplerate)
{
    std::memcpy(m_data, sb.m_data, m_frames*m_channels*sizeof(sample_t));
}


SampleBuffer::~SampleBuffer ()
{
    if (m_data) {
        delete m_data;
    }
}


int SampleBuffer::seek (nframes_t frame)
{
  if (frame <= m_frames) {
    m_pos = m_data + (frame * m_channels);
    return frame;
  }
  return -1;
}


int SampleBuffer::write (sample_t* ptr, nframes_t frames)
{
  size_t chunkSize = frames * m_channels;
  if (m_pos + chunkSize > (void*)m_frames) {
    return 0;
  }

  std::memcpy(m_pos, ptr, frames * m_channels * sizeof(sample_t));
  return frames;
}

} // Unison

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
