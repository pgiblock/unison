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
    SampleBuffer(float *buf, int frames)
  {
    //m_data = new 
  }
  private:
    sampleFrame *m_data;
    int m_frames;
};

}

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
