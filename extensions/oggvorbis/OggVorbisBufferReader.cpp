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
#include "vorbis/vorbisfile.h"

#include "OggVorbisBufferReader.h"
#include "unison/SampleBuffer.h"
#include "unison/endian_handling.h"

using namespace OggVorbis::Internal;
using namespace Core;

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
  static ov_callbacks callbacks =
  {
    qfileReadCallback,
    qfileSeekCallback,
    qfileCloseCallback,
    qfileTellCallback
  } ;

  OggVorbis_File vf;

  int frames = 0;

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
        printf( "sampleBuffer::decodeSampleOGGVorbis():"
            " media read error\n" );
        break;
      case OV_ENOTVORBIS:
        /*				printf( "sampleBuffer::decodeSampleOGGVorbis():"
                                        " not an Ogg Vorbis file\n" );*/
        break;
      case OV_EVERSION:
        printf( "sampleBuffer::decodeSampleOGGVorbis():"
            " vorbis version mismatch\n" );
        break;
      case OV_EBADHEADER:
        printf( "sampleBuffer::decodeSampleOGGVorbis():"
            " invalid Vorbis bitstream header\n" );
        break;
      case OV_EFAULT:
        printf( "sampleBuffer::decodeSampleOgg(): "
            "internal logic fault\n" );
        break;
    }
    delete f;
    return 0;
  }

  ov_pcm_seek( &vf, 0 );

  int channels = ov_info( &vf, -1 )->channels;
  //_samplerate = ov_info( &vf, -1 )->rate;

  ogg_int64_t total = ov_pcm_total( &vf, -1 );

  float *buf = new Unison::sample_t[total * channels];
  int bitstream = 0;
  long bytes_read = 0;

  do
  {
    bytes_read = ov_read( &vf, (char *) &buf[frames * channels],
        ( total - frames ) * channels *
        Unison::BYTES_PER_INT_SAMPLE,
        isLittleEndian() ? 0 : 1,
        Unison::BYTES_PER_INT_SAMPLE, 1, &bitstream );
    if( bytes_read < 0 )
    {
      break;
    }
    frames += bytes_read / ( channels * Unison::BYTES_PER_INT_SAMPLE );
  }
  while( bytes_read != 0 && bitstream == 0 );

  ov_clear( &vf );
  return NULL;
}

// vim: ts=8 sw=2 sts=2 et sta noai
