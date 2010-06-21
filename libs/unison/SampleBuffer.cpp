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

#include "unison/SampleBuffer.h"

#include <QtGlobal>
#include <cstring>

using namespace Unison;

const int SampleBuffer::DEFAULT_CHANNELS;

SampleBuffer::SampleBuffer (float *buf, int frames, int channels, int samplerate) :
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

// vim: ts=8 sw=2 sts=2 et sta noai
