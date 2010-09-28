/*
 * RingBuffer.h
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

#ifndef UNISON_RING_BUFFER_H_
#define UNISON_RING_BUFFER_H_

#include <QObject>

namespace Unison {

/**
 * A lock-free ringbuffer derived from the implementation written by Paul Davis for JACK.
 *
 * The key attribute of a ringbuffer is that it can be safely accessed by two threads
 * simultaneously -- one reading from the buffer and the other writing to it -- without
 * using any synchronization or mutual exclusion primitives.  For this to work correctly,
 * there can only be a single reader and a single writer thread.  Their identities cannot
 * be interchanged.
 */
template <typename T>
class RingBuffer
{

  public:
    /**
     * Contruct a ringbuffer, specifying the internal buffer size.
     * @param size  The size of the buffer to be allocated
     */
    RingBuffer (const int size);

    /**
     * Destory the ringbuffer and allocated space.
     */
    virtual ~RingBuffer ();

    /**
     * Reset the read and write pointers, making an empty buffer.
     * This is not thread safe.
     */
    void reset ();

    /**
     * Get the size of the ringbuffer
     * @param the ringbuffer size in elements.
     */
    int capacity () const
    {
      return m_size;
    }

    /**
     * Return the number of elements available for writing.
     * @return the amount of free space (in elements) available for writing.
     */
    int writeSpace () const;

    /**
     * Return the number of elements available for reading.
     * @return the number of elements available to read.
     */
    int readSpace () const;

    /**
     * Read data from the ringbuffer.  Caller must check the return value and call this
     * function again if necessary, or use the read method which does this automatically.
     * @param dest a pointer to a buffer where data read from the ringbuffer will go.
     * @param cnt the number of elements to read.
     * @return the number of elements read, which may range from 0 to cnt.
     */
    int readChunk (T* dest, int cnt);

    /**
     * Read data from the ringbuffer, wrapping when end of buffer is reached.  Like the
     * @f readChunk() function, but will automatically wrap to the beginning of the buffer
     * if the end is reached.
     * @param dest a pointer to a buffer where data read from the ringbuffer will go.
     * @param cnt the number of elements to read.
     * @return true if the buffer contains enough data to fulfill the request
     */
    int read (T* dest, int cnt);

    /**
     * Read from the ringbuffer without moving the read pointer.  A convenient way to
     * inspect the data in a continous fashion. The cost is that the data must be copied
     * into the destination buffer.  Caller must check the return value and call this
     * function again if necessary, or use the peek method which does this automatically.
     * @param dest a pointer to a buffer where data read from the ringbuffer will go.
     * @param cnt the number of elements to read.
     * @return the number of elements read, which may range from 0 to cnt.
     */
    int peekChunk (T* dest, int cnt) const;

    /**
     * Read data from the ringbuffer, wrapping when end of buffer is reached.  Like the
     * peekChunk() function, but will automatically wrap to the beginning of the buffer if
     * the end is reached.
     * @param dest a pointer to a buffer where data read from the ringbuffer will go.
     * @param cnt the number of elements to read.
     * @return true if the buffer contains enough data to fulfill the request
     */
    int peek (T* dest, int cnt) const;

    /* TODO: Non-copy peek (ala, jack_rb_get_read_vector() */

    /**
     * Advance the read pointer by the specified number of elements.  The data is not
     * copied, and is lost forever.  This is useful to avoid a copy if a reader determines
     * that the next chunk of data can be ignored.
     *
     * @params cnt the number of elements to advance
     * @return true if there was enough space, false if canceled due to too little space
     */
    bool skip (int cnt);

    /**
     * Write data to the ringbuffer. Writes at most cnt elements from src.
     *
     * @param src a pointer to a buffer where data written to the ringbuffer is read.
     * @param cnt the maximum number of elements to write.
     *
     * @return the actual count of elements copied
     */
    int write (const T* src, int cnt);

  private:
    QAtomicInt m_writePtr;
    QAtomicInt m_readPtr;

    int m_size; ///< Size in elements
    T* m_data;  ///< The buffer contents
};


template<typename T>
RingBuffer<T>::RingBuffer (const int size) :
  m_size(size),
  m_data(reinterpret_cast<T*>( new char[size * sizeof(T)] ))
{
  reset();
}


template<typename T>
RingBuffer<T>::~RingBuffer ()
{
  char* data = reinterpret_cast<char*>(m_data);
  delete[] data;
}


template<typename T>
void RingBuffer<T>::reset ()
{
  m_writePtr = 0;
  m_readPtr = 0;
  Q_ASSERT(readSpace() == 0);
  Q_ASSERT(writeSpace() == m_size-1);
}


template<typename T>
int RingBuffer<T>::readSpace () const
{
  const int w = m_writePtr;
  const int r = m_readPtr;

  if (w > r) {
    return w - r;
  }
  else {
    return (w - r + m_size) % m_size;
  }
}


template<typename T>
int RingBuffer<T>::writeSpace () const
{
  const int w = m_writePtr;
  const int r = m_readPtr;

  if (w > r) {
    return ((r - w + m_size) % m_size) - 1;
  }
  else if (w < r) {
    return (r - w) - 1;
  }
  else {
    return m_size - 1;
  }
}


template<typename T>
int RingBuffer<T>::readChunk (T* dest, int cnt)
{
  const int readPtr = m_readPtr;

  const int readable = (readPtr + cnt < m_size) ?
                        cnt :
                        m_size - readPtr;

  memcpy(dest, &m_data[readPtr], readable*sizeof(T));

  m_readPtr = (readPtr + readable) % m_size;

  return readable;
}


template<typename T>
int RingBuffer<T>::read (T* dest, int cnt)
{
  cnt = std::min(cnt, readSpace());

  // Read as much as possible until end of buffer
  const int readable = readChunk(dest, cnt);

  // Read any remainder from the front of buffer
  if (readable < cnt) {
    return readable + readChunk(dest + readable, cnt - readable);
  }

  return readable;
}


template<typename T>
int RingBuffer<T>::peekChunk (T* dest, int cnt) const
{
  const int readPtr = m_readPtr;

  const int readable = (readPtr + cnt < m_size) ?
                        cnt :
                        m_size - readable;

  memcpy(dest, &m_data[readPtr], readable*sizeof(T));

  return readable;
}


template<typename T>
int RingBuffer<T>::peek (T* dest, int cnt) const
{
  cnt = std::min(cnt, readSpace());

  // Read as much as possible until end of buffer
  const int readable = peekChunk(dest, cnt);

  // Read the rest from the front of the buffer
  if (readable < cnt) {
    return readable + peekChunk(dest + readable, cnt - readable);
  }

  return readable;
}


template<typename T>
bool RingBuffer<T>::skip (int cnt)
{
  if (cnt > readSpace()) {
    qWarning("Invalid attempt to skip past the end of RingBuffer");
    return false;
  }

  const int readPtr = m_readPtr;
  m_readPtr = (readPtr + cnt) % m_size;
  return true;
}


// XXX: Inline??
template<typename T>
int RingBuffer<T>::write(const T* src, int cnt)
{
  const int writePtr = m_writePtr;
  const int freeCnt = writeSpace();
  if (freeCnt == 0) {
    return 0;
  }

  cnt = std::min(cnt, freeCnt);

  // Simple case, no wrap-around
  if (writePtr + cnt <= m_size) {
    memcpy(&m_data[writePtr], src, cnt*sizeof(T));
    // Must do modulo here since the write pointer may wrap back to position 0
    m_writePtr = writePtr + cnt % m_size;
  }
  else {
    const size_t writable = m_size - writePtr;
    memcpy(&m_data[writePtr], src, writable * sizeof(T));
    memcpy(m_data, src+writable, (cnt - writable) * sizeof(T));
    m_writePtr = cnt - writable;
  }
  return cnt;
}

} // Unison

#endif

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
