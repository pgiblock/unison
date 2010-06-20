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

/** A sampleFrame is 2 floats (for stereo). */
typedef sample_t sampleFrame[DEFAULT_CHANNELS];

/** A buffer of samples.
 *
 * When a buffer of samples is replayed, it should recreate a wave.  A frame 
 * is a timeslice, and a sample is the value of a wave at a particular time.
 *
 * The difference between a SampleBuffer and an AudioBuffer is that the former 
 * represents an "audio resource" - something that is loaded, recorded, 
 * generated.. something that cannot be modified (at least for now).  
 * AudioBuffer, on the other hand, does not represent an external resource, 
 * and CAN be modified - it acts like a FIFO between 2 different processors.  
 * For example, an outputPort and an inputPort communicate via a shared 
 * AudioBuffer.
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
