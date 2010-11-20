/*
 * TestRingBuffer.cpp
 *
 * Just some really basic tests for the RingBuffer
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

#include <unison/RingBuffer.h>

#include <iostream>

using namespace Unison;

bool singlePushAndPop ()
{
  RingBuffer<int> rb(4);
  int v = 1234;
  rb.write(&v, 1);

  bool sizeOk = (rb.writeSpace() == 2);

  int chk = 0;
  rb.read(&chk, 1);

  bool dataOk = (chk == v);

  return sizeOk && dataOk;
}


bool pushAndPopAreFifo ()
{
  const int in[] = {0,1,2,3,4,5,6,7};
  RingBuffer<int> rb(10);

  // Fill it two at a time
  for(int i=0; i<8; i+=2) {
    rb.write(&in[i], 2);
  }

  // Read back 3 at a time
  int out[3];
  int j = 0;
  int got = 0;
  do {
    got = rb.read(out, 3);
    // Compare
    for (int k=0; k<got; ++k) {
      if (out[k] != in[j+k]) {
        return false;
      }
    }
    j += got;

  } while (got);

  // Should have 8 out
  return j == 8; 
}


bool pushOverflowSingles ()
{
  RingBuffer<int> rb(4);
  int v = 1234;
  for (int i=0; i< 3; ++i) {
    int r = rb.write(&v, 1);
    // succeed for first three
    if (r != 1) {
      return false;
    }
  }

  // Fourth one fails
  if (rb.write(&v, 1) != 0) {
    return false;
  }

  // And no room left
  return rb.writeSpace() == 0;
}


bool pushAndPopOverBoundary ()
{
  const int in[] = {0,1,2,3,4,5,6,7};
  int out[8];

  RingBuffer<int> rb(8);

  // Fill it half way, and read it back out
  rb.write(in, 5);
  if (rb.read(out, 5) != 5) {
    return false;
  }

  // Should be caught-up with writer
  if (rb.readSpace() != 0) {
    return false;
  }
  if (rb.writeSpace() != 7) {
    return false;
  }

  // Fill it more, causing a wrap
  // This used to cause a segfault
  if (rb.write(in, 5) != 5) {
    return false;
  }

  if (rb.readSpace() != 5) {
    return false;
  }
  if (rb.read(out, 5) != 5) {
    return false;
  }

  return true;
}

int main (int argc, char* argv[])
{
  RingBuffer<int> rb(8);
  bool spp = singlePushAndPop();
  bool ppf = pushAndPopAreFifo();
  bool pos = pushOverflowSingles();
  bool ppb = pushAndPopOverBoundary();
  std::cout << "  singlePushPop: "          << spp?"OK":"FAIL" << std::endl;
  std::cout << "  pushAndPopAreFifo: "      << ppf?"OK":"FAIL" << std::endl;
  std::cout << "  pushOverflowSingles: "    << pos?"OK":"FAIL" << std::endl;
  std::cout << "  pushAndPopOverBoundary: " << ppb?"OK":"FAIL" << std::endl;

  return (spp + ppf + pos + ppb -  4);
}

// vim: tw=90 ts=8 sw=2 sts=2 et sta noai
