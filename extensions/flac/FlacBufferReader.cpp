/*
 * FlacBufferReader.cpp
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

#include <FLAC++/decoder.h>

#include "FlacBufferReader.hpp"
#include "unison/SampleBuffer.hpp"

using namespace Flac::Internal;
using namespace Core;

static FLAC__uint64 total_samples = 0;
static unsigned sample_rate = 0;
static unsigned channels = 0;
static unsigned bps = 0;

static bool write_little_endian_uint16(FILE *f, FLAC__uint16 x)
{
  return
    fputc(x, f) != EOF &&
    fputc(x >> 8, f) != EOF
  ;
}

static bool write_little_endian_int16(FILE *f, FLAC__int16 x)
{
  return write_little_endian_uint16(f, (FLAC__uint16)x);
}

static bool write_little_endian_uint32(FILE *f, FLAC__uint32 x)
{
  return
    fputc(x, f) != EOF &&
    fputc(x >> 8, f) != EOF &&
    fputc(x >> 16, f) != EOF &&
    fputc(x >> 24, f) != EOF
  ;
}

class OurDecoder: public FLAC::Decoder::File {
  public:
    OurDecoder(FILE *f_): FLAC::Decoder::File(), f(f_) { }
  protected:
    FILE *f;

    virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
    virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata);
    virtual void error_callback(::FLAC__StreamDecoderErrorStatus status);
};

::FLAC__StreamDecoderWriteStatus OurDecoder::write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
  const FLAC__uint32 total_size = (FLAC__uint32)(total_samples * channels * (bps/8));
  size_t i;

  if(total_samples == 0) {
    fprintf(stderr, "ERROR: this example only works for FLAC files that have a total_samples count in STREAMINFO\n");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  if(channels != 2 || bps != 16) {
    fprintf(stderr, "ERROR: this example only supports 16bit stereo streams\n");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }

  /* write WAVE header before we write the first frame */
  if(frame->header.number.sample_number == 0) {
    if(
      fwrite("RIFF", 1, 4, f) < 4 ||
      !write_little_endian_uint32(f, total_size + 36) ||
      fwrite("WAVEfmt ", 1, 8, f) < 8 ||
      !write_little_endian_uint32(f, 16) ||
      !write_little_endian_uint16(f, 1) ||
      !write_little_endian_uint16(f, (FLAC__uint16)channels) ||
      !write_little_endian_uint32(f, sample_rate) ||
      !write_little_endian_uint32(f, sample_rate * channels * (bps/8)) ||
      !write_little_endian_uint16(f, (FLAC__uint16)(channels * (bps/8))) || /* block align */
      !write_little_endian_uint16(f, (FLAC__uint16)bps) ||
      fwrite("data", 1, 4, f) < 4 ||
      !write_little_endian_uint32(f, total_size)
    ) {
      fprintf(stderr, "ERROR: write error\n");
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
  }

  /* write decoded PCM samples */
  for(i = 0; i < frame->header.blocksize; i++) {
    if(
      !write_little_endian_int16(f, (FLAC__int16)buffer[0][i]) ||  /* left channel */
      !write_little_endian_int16(f, (FLAC__int16)buffer[1][i])     /* right channel */
    ) {
      fprintf(stderr, "ERROR: write error\n");
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
  }

  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void OurDecoder::metadata_callback(const ::FLAC__StreamMetadata *metadata)
{
  /* print some stats */
  if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    /* save for later */
    total_samples = metadata->data.stream_info.total_samples;
    sample_rate = metadata->data.stream_info.sample_rate;
    channels = metadata->data.stream_info.channels;
    bps = metadata->data.stream_info.bits_per_sample;

    fprintf(stderr, "sample rate    : %u Hz\n", sample_rate);
    fprintf(stderr, "channels       : %u\n", channels);
    fprintf(stderr, "bits per sample: %u\n", bps);
#ifdef _MSC_VER
    fprintf(stderr, "total samples  : %I64u\n", total_samples);
#else
    fprintf(stderr, "total samples  : %llu\n", total_samples);
#endif
  }
}

void OurDecoder::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
  fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}


Unison::SampleBuffer *FlacBufferReader::read (const QString &filename)
{
  //OurDecoder(filename);
  return NULL;
}

// vim: ts=8 sw=2 sts=2 et sta noai
