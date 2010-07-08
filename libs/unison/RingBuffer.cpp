/*
 * RingBuffer.cpp
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

#include "RingBuffer.h"

#include <QtCore/QtDebug>

#include <malloc.h>

using namespace Unison;


template<typename T>
RingBuffer<T>::RingBuffer (int size) :
  m_size(size),
  m_data(static_cast<char*>(malloc(size)))
{
  Q_CHECK_PTR(m_data);

  reset();
}


template<typename T>
RingBuffer<T>::~RingBuffer ()
{
  free(m_data);
}


template<typename T>
void RingBuffer<T>::reset () {
  m_writePtr = 0;
  m_readPtr = 0;
  Q_ASSERT(readSpace() == 0);
  Q_ASSERT(writeSpace() == m_size-1);
}


template<typename T>
int RingBuffer<T>::writeSpace () const {
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
int RingBuffer<T>::readSpace () const {
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
int RingBuffer<T>::readChunk (T *dest, int cnt)
{
  const int readPtr = m_readPtr;

  const int readable = (readPtr + cnt < m_size) ?
                        cnt :
                        m_size - readPtr;

  memcpy(dest, &m_data[readPtr], readable);

  m_readPtr = (readPtr + readable) % m_size;

  return readable;
}


template<typename T>
bool RingBuffer<T>::read (T *dest, int cnt)
{
  if (readSpace() < cnt) {
    return false;
  }

  // Read as much as possible until end of buffer
  const int readable = readChunk(dest, cnt);

  // Read any remainder from the front of buffer
  if (readable < cnt) {
          readChunk(dest + readable, cnt - readable);
  }

  return true;
}


template<typename T>
int RingBuffer<T>::peekChunk (T *dest, int cnt)
{
  const int readPtr = m_readPtr;

  const int readable = (readPtr + cnt < m_size) ?
                        cnt :
                        m_size - readable;

  memcpy(dest, &m_data[readPtr], readable);

  return readable;
}


template<typename T>
bool RingBuffer<T>::peek (T *dest, int cnt)
{
  if (readSpace() < cnt) {
    return false;
  }

  // Read as much as possible until end of buffer
  const int readable = peekChunk(dest, cnt);

  // Read the rest from the front of the buffer
  if (readable < cnt) {
    peekChunk(cnt - readable, dest + readable);
  }

  return true;
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
int RingBuffer<T>::write(const T *src, int cnt)
{
  const int writePtr = m_writePtr;

  cnt = std::min(cnt, m_size);

  // Simple case, no wrap-around
  if (writePtr + cnt <= m_size) {
    memcpy(&m_data[writePtr], src, cnt);
    // Must do modulo here since the write pointer may wrap back to position 0
    m_writePtr = writePtr + cnt % m_size;
  }
  else {
    const size_t writable = m_size - writePtr;
    memcpy(&m_data[writePtr], src, writable);
    memcpy(m_data, src+writable, cnt - writable);
    m_writePtr = cnt - writable;
  }
  return cnt;
}

// vim: ts=8 sw=2 sts=2 et sta noai
