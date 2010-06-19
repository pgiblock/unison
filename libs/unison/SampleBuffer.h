#ifndef UNISON_SAMPLE_BUFFER_H
#define UNISON_SAMPLE_BUFFER_H

#define DEFAULT_CHANNELS 2

namespace Unison
{

/** A float is a common abstraction for a sample.  We use float all throughout 
 * Unison - manipulating integer samples will cause bad aliasing.  Why not 
 * make it int anyways? because the SndFile loader is not the only file 
 * format - perhaps some formats only support float.
 */
typedef float sample_t;

typedef sample_t sampleFrame[DEFAULT_CHANNELS];

/** A buffer of samples
 *
 * When a buffer of samples is replayed, it should recreate a wave.  A frame 
 * is a timeslice, and a sample is the value of a wave at a particular time.
 */
class SampleBuffer
{
  public:
    SampleBuffer(float *buf, int frames, int channels, int samplerate):
      m_data(buf), m_frames(frames), m_channels(channels), m_samplerate(samplerate)
  {}

  private:
    sample_t *m_data;
    int m_frames;
    int m_channels;
    int m_samplerate;
};

}

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
