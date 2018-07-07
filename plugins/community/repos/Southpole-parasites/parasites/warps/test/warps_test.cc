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

#include "stmlib/test/wav_writer.h"
#include "stmlib/utils/random.h"

#include "warps/dsp/modulator.h"
#include "warps/dsp/sample_rate_converter.h"
#include "warps/resources.h"

using namespace warps;
using namespace std;
using namespace stmlib;

const size_t kSampleRate = 96000;
const size_t kBlockSize = 96;

template<typename T>
void TestSRCUp(const char* name) {
  size_t ratio = 6;
  WavWriter wav_writer(1, kSampleRate * ratio, 10);
  wav_writer.Open(name);
  
  T src_up;
  src_up.Init();
  
  float phase = 0.0f;
  while (!wav_writer.done()) {
    float samples_in[kBlockSize];
    float samples_out[kBlockSize * ratio];
    float instantenous_frequency = fmod(
        4.0f * wav_writer.progress(),
        0.5f);
    for (size_t i = 0; i < kBlockSize; ++i) {
      samples_in[i] = sinf(phase * 2 * M_PI);
      phase += instantenous_frequency;
      if (phase >= 1.0f) {
        phase -= 1.0f;
      }
    }
    
    src_up.Process(samples_in, samples_out, kBlockSize);
    wav_writer.Write(samples_out, kBlockSize * ratio, 16067.0f);
  }
}

void TestSRC96To576To96() {
  size_t ratio = 6;
  WavWriter wav_writer(1, kSampleRate, 10);
  wav_writer.Open("warps_src_96_576_96.wav");
  
  SampleRateConverter<SRC_UP, 6, 48> src_up;
  SampleRateConverter<SRC_DOWN, 6, 48> src_down;
  src_up.Init();
  src_down.Init();
  
  float phase = 0.0f;
  while (!wav_writer.done()) {
    float samples_in[kBlockSize];
    float samples_out[kBlockSize * ratio];
    float instantenous_frequency = fmod(
        2.0f * wav_writer.progress(),
        0.5f);
    for (size_t i = 0; i < kBlockSize; ++i) {
      samples_in[i] = sinf(phase * 2 * M_PI);
      phase += instantenous_frequency;
      if (phase >= 1.0f) {
        phase -= 1.0f;
      }
    }
    src_up.Process(samples_in, samples_out, kBlockSize);
    src_down.Process(samples_out, samples_in, kBlockSize * ratio);
    wav_writer.Write(samples_in, kBlockSize, 32200.0f);
  }
}

void TestModulator() {
  FILE* fp_in = fopen("audio_samples/modulation_96k.wav", "rb");
  WavWriter wav_writer(2, kSampleRate, 15);
  wav_writer.Open("warps_modulator.wav");
  
  fseek(fp_in, 48, SEEK_SET);
  
  Modulator modulator;
  modulator.Init(kSampleRate);
  
  Parameters* p = modulator.mutable_parameters();
  
  float phi = 0.0f;
  float filtered_noise = 0.0f;

  while (!wav_writer.done()) {
    float triangle = wav_writer.triangle();
    float square = 0.5f * SoftClip(32.0f * (triangle - 0.5f)) + 0.5f;
    float noise = (Random::GetFloat() - 0.5f) / 128.0f;
    filtered_noise += (noise - filtered_noise) * 0.1f;
    
    p->carrier_shape = 1;
    p->channel_drive[0] = 0.625f;
    p->channel_drive[1] = 0.0f;
    p->modulation_algorithm = 0.125f + 0.0f * square + 1.0f * filtered_noise;
    p->modulation_parameter = 0.0f + 1.0f * triangle;
    p->note = 48.0f + phi;

    ShortFrame input[kBlockSize];
    ShortFrame output[kBlockSize];
    
    if (fread(
            input,
            sizeof(ShortFrame),
            kBlockSize,
            fp_in) != kBlockSize) {
      break;
    }
    
    for (size_t i = 0; i < kBlockSize; ++i) {
      input[i].r = input[i].l;
      input[i].l = 0;
    }
    modulator.Process(input, output, kBlockSize);
    wav_writer.WriteFrames((short*)(output), kBlockSize);
  }
  fclose(fp_in);
}

void TestEasterEgg() {
  FILE* fp_in = fopen("audio_samples/ericderr96.wav", "rb");
  
  WavWriter wav_writer(2, kSampleRate, 15);
  wav_writer.Open("warps_easter_egg.wav");
  
  fseek(fp_in, 48, SEEK_SET);
  
  Modulator modulator;
  modulator.Init(kSampleRate);
  modulator.set_feature_mode(FEATURE_MODE_CHEBYSCHEV);
  
  Parameters* p = modulator.mutable_parameters();
  
  float phi = 0.0f;
  float filtered_noise = 0.0f;
  while (!wav_writer.done()) {
    uint16_t tri = (wav_writer.remaining_frames() / 8);
    tri = tri > 32767 ? 65535 - tri : tri;
    float triangle = 0.5f * SoftClip(8.0f * (tri / 32768.0f - 0.5f)) + 0.5f;
    float square = 0.5f * SoftClip(32.0f * (tri / 32768.0f - 0.5f)) + 0.5f;
    float noise = (Random::GetFloat() - 0.5f) / 128.0f;
    filtered_noise += (noise - filtered_noise) * 0.1f;
    
    p->raw_algorithm_pot = 1.0f + 0.0f * triangle + 0.0f * square;
    p->raw_algorithm_cv = 0.0f;
    p->raw_algorithm = 0.0f;// + 0.0f * triangle;// + 0.1f * filtered_noise;
    p->modulation_algorithm = 0.3f;// + 1.0f * triangle;
    p->carrier_shape = 0;
    p->channel_drive[0] = 0.5f;
    p->channel_drive[1] = 0.0f;
    p->raw_level[0] = 0.6f;
    p->raw_level[1] = 1.0f;
    p->modulation_parameter = 1.0f;// + 0.05f * filtered_noise;
    p->note = 48.0f + phi;

    ShortFrame input[kBlockSize];
    ShortFrame output[kBlockSize];
    
    if (fread(
            input,
            sizeof(ShortFrame),
            kBlockSize,
            fp_in) != kBlockSize) {
      break;
    }
    
    for (size_t i = 0; i < kBlockSize; ++i) {
      input[i].r = input[i].r;
      input[i].l = input[i].l;
    }
    modulator.Process(input, output, kBlockSize);
    wav_writer.WriteFrames((short*)(output), kBlockSize);
  }
  fclose(fp_in);
}

void TestOscillators() {
  WavWriter wav_writer(2, kSampleRate, 15);
  wav_writer.Open("warps_oscillator.wav");
  
  Modulator modulator;
  modulator.Init(kSampleRate);
  
  Parameters* p = modulator.mutable_parameters();
  
  p->carrier_shape = 1;
  p->channel_drive[0] = 0.0f;
  p->channel_drive[1] = 0.0f;
  p->modulation_algorithm = 0.0f;
  p->modulation_parameter = 0.0f;

  ShortFrame input[kBlockSize];
  ShortFrame zero;
  zero.l = zero.r = 0.0f;
  fill(&input[0], &input[kBlockSize], zero);

  while (!wav_writer.done()) {
    float triangle = wav_writer.triangle();
    p->note = 20 + triangle * 0;

    ShortFrame output[kBlockSize];
    modulator.Process(input, output, kBlockSize);
    wav_writer.WriteFrames((short*)(output), kBlockSize);
  }
}

void TestFilterBankReconstruction() {
  FilterBank fb;
  fb.Init(96000.0);
  
  size_t num_blocks = 1000;
  const size_t block_size = 96;
  
  FILE* fp_out = fopen("warps_filterbank_ir_float32.bin", "wb");

  float in[block_size];
  float out[block_size];
  
  for (size_t i = 0; i < num_blocks; ++i) {
    fill(&in[0], &in[block_size], 0.0f);
    if (i == 0) {
      in[0] = 1.0f;
    }
    fb.Analyze(in, block_size);
    
    for (int32_t j = 0; j < kNumBands; ++j) {
      if (false) {
        float* samples = fb.band(j).samples;
        size_t size = block_size / fb.band(j).decimation_factor;
        fill(&samples[0], &samples[size], 0.0f);
      }
    }
    
    fb.Synthesize(out, block_size);
    fwrite(out, sizeof(float), block_size, fp_out);
  }
  fclose(fp_out);
  
  // The impulse response and frequency responses can be seen with the following
  // python program:
  //
  // import numpy
  // import pylab
  //
  // x = numpy.fromfile('warps_filterbank_ir_float32.bin', dtype=numpy.float32)
  // pylab.subplot(211)
  // pylab.plot(x[:500])
  // pylab.subplot(212)
  // xf = numpy.fft.rfft(x)
  // N = len(xf)
  // f = numpy.arange(0.0, N) / N * 48000
  // pylab.semilogx(f, 20 * numpy.log10(numpy.abs(xf + 1e-8)))
  // pylab.show()
}

void TestSineTransition() {
  WavWriter wav_writer(2, kSampleRate, 15);
  wav_writer.Open("warps_sine_transition.wav");
  
  Modulator modulator;
  modulator.Init(kSampleRate);
  
  Parameters* p = modulator.mutable_parameters();
  
  while (!wav_writer.done()) {
    float triangle = wav_writer.triangle();
    p->carrier_shape = 1;
    p->channel_drive[0] = 0.0;
    p->channel_drive[1] = 0.0f;
    p->modulation_algorithm = 0.0f + 0.125f * triangle;
    p->modulation_parameter = 0.0f;
    p->note = 36.0f;

    ShortFrame input[kBlockSize];
    ShortFrame output[kBlockSize];
    
    for (size_t i = 0; i < kBlockSize; ++i) {
      input[i].r = 0;
      input[i].l = 0;
    }
  
    modulator.Process(input, output, kBlockSize);
    wav_writer.WriteFrames((short*)output, kBlockSize);
  }
}

void TestGain() {
  WavWriter wav_writer(2, kSampleRate, 10);
  wav_writer.Open("warps_gain.wav");
  
  Modulator modulator;
  modulator.Init(kSampleRate);
  
  Parameters* p = modulator.mutable_parameters();
  
  float phase = 0.0f;
  while (!wav_writer.done()) {
    float triangle = wav_writer.triangle();
    
    p->carrier_shape = 0;
    p->channel_drive[0] = triangle * 1.0f;
    p->channel_drive[1] = 0.0f;
    p->modulation_algorithm = 0.0f + 0.125f * 0.0f;
    p->modulation_parameter = 0.0f;
    p->note = 36.0f;

    ShortFrame input[kBlockSize];
    ShortFrame output[kBlockSize];
    
    for (size_t i = 0; i < kBlockSize; ++i) {
      input[i].l = 20480.0f * sinf(phase * 2 * M_PI);
      input[i].r = 20480.0f * sinf(phase * 2 * M_PI);
      phase += 220.0f / kSampleRate;
      if (phase >= 1.0f) {
        phase -= 1.0f;
      }
    }
  
    modulator.Process(input, output, kBlockSize);
    wav_writer.WriteFrames((short*)output, kBlockSize);
  }
}

void TestQuadratureOscillator() {
  WavWriter wav_writer(2, kSampleRate, 10);
  wav_writer.Open("warps_quadrature.wav");
  QuadratureOscillator q;
  q.Init(kSampleRate);
  while (!wav_writer.done()) {
    float triangle = wav_writer.triangle();
    float x[2];
    q.Render(triangle, 100.8f, &x[0], &x[1], 1);
    wav_writer.Write(x, 2, 32767.0f);
  }
}

int main(void) {
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  //TestSRCUp<SampleRateConverter<SRC_UP, 6, 48> >("warps_src_up_fir_48.wav");
  //TestSRC96To576To96();
  // TestModulator();
  TestEasterEgg();
  //TestOscillators();
  //TestFilterBankReconstruction();
  //TestSineTransition();
  //TestGain();
  //TestQuadratureOscillator();
}
