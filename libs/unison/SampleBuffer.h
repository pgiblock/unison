#ifndef UNISON_SAMPLE_BUFFER_H
#define UNISON_SAMPLE_BUFFER_H

namespace Unison
{

class SampleBuffer
{
  public:
    SampleBuffer(short *buf, int frames): m_data(buf), m_frames(frames) {}
  private:
    short *m_data;
    int m_frames;
};

}

#endif

// vim: ts=8 sw=2 sts=2 et sta noai
