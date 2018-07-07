// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------


#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <xmmintrin.h>

#include "clouds/dsp/granular_processor.h"
#include "clouds/resources.h"

using namespace clouds;
using namespace std;
using namespace stmlib;

const size_t kSampleRate = 32000;
const size_t kBlockSize = 32;

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

void TestDSP() {
  size_t duration = 15;

  FILE* fp_in = fopen("audio_samples/sine.wav", "rb");
  FILE* fp_out = fopen("clouds.wav", "wb");

  size_t remaining_samples = kSampleRate * duration;
  write_wav_header(fp_out, remaining_samples, 2);
  fseek(fp_in, 48, SEEK_SET);
  
  uint8_t large_buffer[118784];
  uint8_t small_buffer[65536 - 128]; 
  
  GranularProcessor processor;
  processor.Init(
      &large_buffer[0], sizeof(large_buffer),
      &small_buffer[0],sizeof(small_buffer));

  processor.set_num_channels(2);
  processor.set_low_fidelity(false);
  processor.set_playback_mode(PLAYBACK_MODE_GRANULAR);
  
  Parameters* p = processor.mutable_parameters();
  
  size_t block_counter = 0;
  float phase_ = 0.0f;
  bool synthetic = false;
  processor.Prepare();
  float pot_noise = 0.0f;
  while (remaining_samples) {
    // uint16_t tri = (remaining_samples * 0.4);
    // tri = tri > 32767 ? 65535 - tri : tri;
    // float triangle = tri / 32768.0f;
    
    p->gate = false;
    p->trigger = false;// || (block_counter & 2047) > 1024;
    p->freeze = false; // || (block_counter & 2047) > 1024;
    p->granular.reverse = true;
    pot_noise += 0.05f * ((Random::GetSample() / 32768.0f) * 0.05f - pot_noise);
    p->position = Random::GetFloat();//triangle * 0.0f + 0.5f;
    p->size = Random::GetFloat();//0.99f;
    p->pitch = Random::GetFloat() * 24.0f; //0.0f + (triangle > 0.5f ? 1.0f : 0.0f) * 0.0f;
    p->density = 0.99f;
    p->texture = 0.7f;
    p->dry_wet = 1.0f;
    p->stereo_spread = 0.0f;
    p->feedback = 0.0f;
    p->reverb = 0.0f;

    ++block_counter;
    ShortFrame input[kBlockSize];
    ShortFrame output[kBlockSize];
    
    if (synthetic) {
      for (size_t i = 0; i < kBlockSize; ++i) {
        phase_ += 400.0f / kSampleRate; // (block_counter & 512 ? 110.0f : 220.0f) / kSampleRate;
        while (phase_ >= 1.0) {
          phase_ -= 1.0;
        }
        input[i].l = 16384.0f * sinf(phase_ * M_PI * 2);
        input[i].r = 32768.0f * (phase_ - 0.5);
        // input[i].r = input[i].l = 0;
      }
      remaining_samples -= kBlockSize;
    } else {
      if (fread(
              input,
              sizeof(ShortFrame),
              kBlockSize,
              fp_in) != kBlockSize) {
        break;
      }
      remaining_samples -= kBlockSize;
    }
    processor.Process(input, output, kBlockSize);
    processor.Prepare();
    fwrite(output, sizeof(ShortFrame), kBlockSize, fp_out);
  }
  fclose(fp_out);
  fclose(fp_in);
}

int main(void) {
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  TestDSP();
  // TestGrainSize();
}
