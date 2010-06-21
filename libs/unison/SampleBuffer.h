/*
 * SampleBuffer.h
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

#ifndef UNISON_SAMPLE_BUFFER_H
#define UNISON_SAMPLE_BUFFER_H

#include "unison/types.h"

namespace Unison {


/**
 * A buffer of samples.
 * When a buffer of samples is replayed, it should recreate a wave.  A frame is
 * a timeslice, and a sample is the value of a wave at that particular time.
 *
 * The difference between a SampleBuffer and an AudioBuffer is that the former
 * represents an "audio resource" - something that is loaded, recorded,
 * generated.. something that cannot be modified (at least for now).
 * AudioBuffer, on the other hand, does not represent an external resource, and
 * CAN be modified - it acts like a FIFO between 2 different processors.  For
 * example, an outputPort and an inputPort communicate via a shared
 * AudioBuffer.
 */
class SampleBuffer
{
  public:
    static const int DEFAULT_CHANNELS = 2;

    class ConstIterator
    {
    };

    typedef ConstIterator const_iterator;


    /**
     * Construct a SampleBuffer from an existing array of samples.
     * SampleBuffer does not gain control of the original buffer, but copies
     * it instead. */
    SampleBuffer (float *buf, int frames, int channels, int samplerate);

    /**
     * Create a silent SampleBuffer with specified properties */
    SampleBuffer (int frames, int channels, int samplerate);

    SampleBuffer (const SampleBuffer& sb);

    ~SampleBuffer ();

    /**
     * @return the raw sample data for this buffer */
    inline const sample_t *samples ()
    {
      return m_data;
    }

    /**
     * @return the number of frames per channel */
    inline nframes_t frames ()
    {
      return m_frames;
    }

    /**
     * @return the total number of frames across all channels. This is
     * primarily for calculating total size of the data. */
    inline nframes_t totalFrames ()
    {
      return m_frames * m_channels;
    }

    /**
     * @return the number of channels for this buffer.  Guaranteed to be
     * at least 1. */
    inline int channels ()
    {
      return m_channels;
    }

    /**
     * @return the sample rate of this buffer */
    inline nframes_t samplerate ()
    {
      return m_samplerate;
    }

  private:
    sample_t *m_data;
    nframes_t m_frames;
    int       m_channels;
    nframes_t m_samplerate;
};

}

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
