#ifndef UNISON_SAMPLE_BUFFER_H
#define UNISON_SAMPLE_BUFFER_H

#define DEFAULT_CHANNELS 2

namespace Unison
{

typedef float sample_t;
typedef sample_t sampleFrame[DEFAULT_CHANNELS];

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
