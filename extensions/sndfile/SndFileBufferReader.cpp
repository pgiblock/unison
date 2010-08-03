/*
 * SndFileBufferReader.cpp
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

#include <QtDebug>
#include <sndfile.h>

#include "SndFileBufferReader.h"
#include "unison/SampleBuffer.h"

using namespace SndFile::Internal;
using namespace Core;

Unison::SampleBuffer *SndFileBufferReader::read (const QString &filename)
{
  qDebug() << "SndFileBufferReader called to read" << filename;
  /*f_cnt_t sampleBuffer::decodeSampleSF( const char * _f,
					int_sample_t * & _buf,
					ch_cnt_t & _channels,
					sample_rate_t & _samplerate )*/
  /*SNDFILE * snd_file;
  SF_INFO sf_info;
  f_cnt_t frames = 0;
  if( ( snd_file = sf_open( filename, SFM_READ, &sf_info ) ) != NULL )
  {
    frames = sf_info.frames;
    _buf = new int_sample_t[sf_info.channels * frames];
    if( sf_read_short( snd_file, _buf, sf_info.channels * frames )
        < sf_info.channels * frames )
    {
    }
    _channels = sf_info.channels;
    _samplerate = sf_info.samplerate;

    sf_close( snd_file );
  }
  else
  {
  }*/

  // Open file.
  SNDFILE *snd_file;
  SF_INFO sf_info;
  snd_file = sf_open(filename.toLatin1(), SFM_READ, &sf_info);
  if (snd_file == NULL)
  {
    // Error
  }

  // Read from it.
  sf_count_t amount = sf_info.channels * sf_info.frames;
  float *buf = new Unison::sample_t[amount];
  if (sf_read_float(snd_file, buf, amount) < amount)
  {
    // Error
  }

  // Close file.
  sf_close(snd_file);

  // Create and return a SampleBuffer .
  Unison::SampleBuffer *sampleBuffer = new Unison::SampleBuffer(buf, sf_info.frames, sf_info.channels, sf_info.samplerate);
  delete buf;

  return sampleBuffer;
}

// vim: ts=8 sw=2 sts=2 et sta noai
