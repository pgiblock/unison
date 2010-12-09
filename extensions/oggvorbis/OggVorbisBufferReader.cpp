/*
 * OggVorbisBufferReader.cpp
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

#include <QFile>
#include <QtDebug>
#include "vorbis/vorbisfile.h"

#include "OggVorbisBufferReader.hpp"
#include "unison/SampleBuffer.hpp"
#include "unison/endian_handling.h"

using namespace OggVorbis::Internal;
using namespace Core;

const int BYTES_PER_INT_SAMPLE = sizeof(uint16_t);

size_t qfileReadCallback( void * _ptr, size_t _size, size_t _n, void * _udata )
{
  return static_cast<QFile *>( _udata )->read( (char*) _ptr,
                _size * _n );
}

int qfileSeekCallback( void * _udata, ogg_int64_t _offset, int _whence )
{
  QFile * f = static_cast<QFile *>( _udata );

  if( _whence == SEEK_CUR )
  {
    f->seek( f->pos() + _offset );
  }
  else if( _whence == SEEK_END )
  {
    f->seek( f->size() + _offset );
  }
  else
  {
    f->seek( _offset );
  }
  return 0;
}

int qfileCloseCallback( void * _udata )
{
  delete static_cast<QFile *>( _udata );
  return 0;
}

long qfileTellCallback( void * _udata )
{
  return static_cast<QFile *>( _udata )->pos();
}

Unison::SampleBuffer *OggVorbisBufferReader::read (const QString &filename)
{
  qDebug() << "OggVorbisBufferReader called to read" << filename;

  static ov_callbacks callbacks =
  {
    qfileReadCallback,
    qfileSeekCallback,
    qfileCloseCallback,
    qfileTellCallback
  } ;

  OggVorbis_File vf;

  QFile * f = new QFile( filename );
  if( f->open( QFile::ReadOnly ) == false )
  {
    delete f;
    return 0;
  }

  int err = ov_open_callbacks( f, &vf, NULL, 0, callbacks );

  if( err < 0 )
  {
    switch( err )
    {
      case OV_EREAD:
        qDebug( "sampleBuffer::decodeSampleOGGVorbis():"
            " media read error\n" );
        break;
      case OV_ENOTVORBIS:
        /*				qDebug( "sampleBuffer::decodeSampleOGGVorbis():"
                                        " not an Ogg Vorbis file\n" );*/
        break;
      case OV_EVERSION:
        qDebug( "sampleBuffer::decodeSampleOGGVorbis():"
            " vorbis version mismatch\n" );
        break;
      case OV_EBADHEADER:
        qDebug( "sampleBuffer::decodeSampleOGGVorbis():"
            " invalid Vorbis bitstream header\n" );
        break;
      case OV_EFAULT:
        qDebug( "sampleBuffer::decodeSampleOgg(): "
            "internal logic fault\n" );
        break;
    }
    delete f;
    return 0;
  }

  ov_pcm_seek( &vf, 0 );

  int channels = ov_info( &vf, -1 )->channels;
  int samplerate = ov_info( &vf, -1 )->rate;

  ogg_int64_t total = ov_pcm_total( &vf, -1 );

  float **pcm;
  float *buf = new Unison::sample_t[total * channels];

  int bitstream = 0;
  long framesRead = 0;

  int j = 0;
  do
  {
    framesRead = ov_read_float(&vf, &pcm, 1024, &bitstream);

    for (int i = 0; i < framesRead; i++) {
            buf[j++] = pcm[0][i];
            buf[j++] = pcm[1][i];
    };
  }
  while( framesRead != 0 && bitstream == 0 );

  ov_clear( &vf );
  Unison::SampleBuffer *sampleBuffer = new Unison::SampleBuffer(buf, total, channels, samplerate);
  return sampleBuffer;
}

// vim: ts=8 sw=2 sts=2 et sta noai
