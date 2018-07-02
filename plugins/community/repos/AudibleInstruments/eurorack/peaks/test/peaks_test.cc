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

#include "peaks/processors.h"

#include "stmlib/test/wav_writer.h"

using namespace peaks;
using namespace stmlib;

const uint32_t kSampleRate = 48000;

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

void TestFMDrum() {
  WavWriter wav_writer(1, kSampleRate, 10);
  wav_writer.Open("fm_drum.wav");

  processors[0].Init(1);
  processors[0].set_control_mode(CONTROL_MODE_FULL);
  processors[0].set_function(PROCESSOR_FUNCTION_FM_DRUM);
  processors[0].set_parameter(0, 25000);
  processors[0].set_parameter(1, 8192);
  processors[0].set_parameter(2, 65535);
  processors[0].set_parameter(3, 0);

  uint32_t period = kSampleRate / 2;
  for (uint32_t i = 0; i < kSampleRate * 10 ; ++i) {
    uint16_t tri = i;
    tri = tri > 32767 ? 65535 - tri : tri;
    uint8_t gate_flag = 0;
    if (i % period < (period / 4)) {
      control |= CONTROL_GATE;
    }
    if (i % period == 0) {
      control |= GATE_FLAG_RISING;
    }
    int16_t s = processors[0].Process(control);
    wav_writer.WriteFrames(&s, 1);
    processors[0].Buffer();
  }
}

void TestPatternPredictor() {
  stmlib::PatternPredictor<32, 8> pattern_predictor;
  
  pattern_predictor.Init();
  
  uint32_t pattern[] = { 500, 400, 1800, 300, 200, 150, 610, 320 };
  
  for (int length = 1; length < 8; ++length) {
    for (int counter = 0; counter < 40; ++counter) {
      uint32_t duration = pattern[counter % length];
      uint32_t tolerance = duration / 20;
      uint32_t deviation = (rand() % tolerance) - tolerance / 2;
      
      uint32_t prediction = pattern_predictor.Predict(duration + deviation);
      uint32_t expected = pattern[(counter + 1) % length];
      float error = (float(prediction) - float(expected)) / float(prediction);
      if (fabsf(error) > 0.06f && counter > 2 * length) {
        printf("%d %d %d %f\n", counter, prediction, expected, error);
      }
    }
  }
}

int main(void) {
  TestFMDrum();
  TestPatternPredictor();
}
