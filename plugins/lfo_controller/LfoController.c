/*
 * LfoController.c - Implementation of a very basic LFO controller
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <lv2.h>

#define CONTROL_OUTPUT_PORT_INDEX  0

struct LfoController
{
  uint64_t cnt;
  double sampleRate;
  float* outData;
};
static LV2_Handle instantiate (const LV2_Descriptor* descriptor,
    double sampleRate, const char* bundlePath,
    const LV2_Feature* const* hostFeatures)
{
  struct LfoController* lfo;
  lfo = malloc(sizeof(struct LfoController));

  lfo->cnt = 0;
  lfo->sampleRate = sampleRate;

  return (LV2_Handle)lfo;
}

#define LFO_PTR ((struct LfoController*)lv2instance)

static void connectPort (LV2_Handle lv2instance, uint32_t port, void* data)
{
  switch (port)
  {
    case CONTROL_OUTPUT_PORT_INDEX:
      LFO_PTR->outData = data;
      break;
  }
}

static void cleanup (LV2_Handle lv2instance)
{
  free(LFO_PTR);
}


static void run (LV2_Handle lv2instance, uint32_t sampleCount)
{
  double periodSecs   = 10.0;
  double  periodFrames = LFO_PTR->sampleRate * periodSecs;
  LFO_PTR->cnt+=sampleCount;
  while (LFO_PTR->cnt > periodFrames) {
    LFO_PTR->cnt -= periodFrames;
  }

  LFO_PTR->outData[0] = -69.0 * ((double)LFO_PTR->cnt) / ((double)periodFrames);
}

static LV2_Descriptor g_lv2descriptor =
{
  .URI = "http://unisonstudio.org/plugins/LfoController",
  .instantiate = instantiate,
  .connect_port = connectPort,
  .run = run,
  .cleanup = cleanup,
};

const LV2_Descriptor* lv2_descriptor (uint32_t index)
{
  if (index == 0) {
    return &g_lv2descriptor;
  }

  return NULL;
}
/* vim: ts=8 sw=2 sts=2 et sta noai
 */
