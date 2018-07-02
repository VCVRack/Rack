// Copyright 2013 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "tides/generator.h"

using namespace tides;
using namespace stmlib;

const uint32_t kSampleRate = 48000;

struct StereoSample {
  uint16_t l, r;
  
  StereoSample(const GeneratorSample& s) {
    l = (s.unipolar >> 1);
    r = s.bipolar;
  }
};

struct TriggerPair {
  uint16_t l, r;
  
  TriggerPair(const GeneratorSample& s) {
    l = (s.flags & FLAG_END_OF_ATTACK) ? 32767 : 0;
    r = (s.flags & FLAG_END_OF_RELEASE) ? 32767 : 0;
  }
};

void write_wav_header(FILE* fp, int num_samples, int num_channels) {
  uint32_t l;
  uint16_t s;
  
  fwrite("RIFF", 4, 1, fp);
  l = 36 + num_samples * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
  fwrite("WAVE", 4, 1, fp);
  
  fwrite("fmt ", 4, 1, fp);
  l = 16;
  fwrite(&l, 4, 1, fp);
  s = 1;
  fwrite(&s, 2, 1, fp);
  s = num_channels;
  fwrite(&s, 2, 1, fp);
  l = kSampleRate;
  fwrite(&l, 4, 1, fp);
  l = static_cast<uint32_t>(kSampleRate) * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
  s = 2 * num_channels;
  fwrite(&s, 2, 1, fp);
  s = 16;
  fwrite(&s, 2, 1, fp);
  
  fwrite("data", 4, 1, fp);
  l = num_samples * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
}


int main(void) {
  FILE* fp = fopen("lfo.wav", "wb");
  write_wav_header(fp, kSampleRate * 10, 2);
  
  /*for (uint16_t i = 0; i < 128; ++i) {
    for (uint16_t j = 0; j < 128; ++j) {
      Generator g;
      g.Init();
      g.set_range(GENERATOR_RANGE_HIGH);
      g.set_mode(GENERATOR_MODE_LOOPING);
      g.set_pitch(60 << 7);
      g.set_slope((j - 64) * 512);
      int32_t min = 0;
      int32_t max = 0;
      for (uint32_t k = 0; k < kSampleRate; ++k) {
        GeneratorSample s = g.Process(0);
        g.FillBufferSafe();
        if (s.bipolar < min) {
          min = s.bipolar;
        } else if (s.bipolar > max) {
          max = s.bipolar;
        }
      }
      printf("%d\t%d\t%d\t%d\n", (i << 7), (j - 64) * 512, min, max);
    }
  }*/
  
  Generator g;
  g.Init();
  g.set_range(GENERATOR_RANGE_HIGH);
  g.set_mode(GENERATOR_MODE_AR);
  g.set_slope(-32768);
  g.set_shape(0);
  g.set_smoothness(0);
  g.set_sync(false);
  
  // uint16_t period_pattern[] = { 400, 400, 200, 800 };
  // uint16_t step = 0;
  uint32_t counter = 0;
  // uint16_t period = period_pattern[step];
  uint32_t period = 512;
  for (uint32_t i = 0; i < kSampleRate * 10; ++i) {
    ++counter;
    uint8_t control = 0;
    if (counter >= period) {
      // step = (step + 1) % (sizeof(period_pattern) / sizeof(uint16_t));
      // period = period_pattern[step];
      // control |= CONTROL_CLOCK_RISING;
      control |= CONTROL_GATE_RISING;
      counter = 0;
      //period = (rand() % 1024) + 2;
    }
    if (counter <= period / 4) {
      control |= CONTROL_GATE;
    }
    /*uint32_t pitch = (24 << 7) + (i >> 5);
    if (pitch > (115 << 7)) {
      pitch = 115 << 7;
    }*/
    uint16_t tri = (i * 100);
    tri = tri > 32767 ? 65535 - tri : tri;
    //g.set_slope(tri);
    g.set_pitch((48 << 7));
    // StereoSample s = StereoSample(g.Process(control * 0));
    TriggerPair s = TriggerPair(g.Process(control));
    fwrite(&s, sizeof(s), 1, fp);
    g.FillBufferSafe();
  }  
}
