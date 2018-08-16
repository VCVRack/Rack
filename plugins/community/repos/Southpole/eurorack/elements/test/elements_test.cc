// Copyright 2014 Olivier Gillet.
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
#include <xmmintrin.h>

#include "elements/dsp/exciter.h"
#include "elements/dsp/part.h"
#include "elements/dsp/resonator.h"
#include "elements/dsp/voice.h"

using namespace elements;
using namespace stmlib;

const uint32_t kSampleRate = 32000;
const uint16_t kAudioBlockSize = 32;

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
  l = ::kSampleRate;
  fwrite(&l, 4, 1, fp);
  l = static_cast<uint32_t>(::kSampleRate) * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
  s = 2 * num_channels;
  fwrite(&s, 2, 1, fp);
  s = 16;
  fwrite(&s, 2, 1, fp);
  
  fwrite("data", 4, 1, fp);
  l = num_samples * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
}

void TestResonator() {
  FILE* fp = fopen("elements_resonator.wav", "wb");
  write_wav_header(fp, ::kSampleRate * 40, 1);
  
  Resonator resonator;
  resonator.Init();
  resonator.set_frequency(110.0f / ::kSampleRate);
  resonator.set_geometry(0.2f);
  resonator.set_brightness(0.4f);
  resonator.set_damping(0.4f);
  resonator.set_position(0.5f);
  // resonator.set_resolution(1);  
  
  float impulse = 1.0f;
  float noise_level = 0.0f;
  for (uint32_t i = 0; i < ::kSampleRate * 40; ++i) {
    uint16_t tri = (i / 2);
    tri = tri > 32767 ? 65535 - tri : tri;
    uint16_t tri2 = (i / 25);
    tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;

    resonator.set_position(tri / 32768.0f);
     
    if (i % (::kSampleRate / 8) == 0 && (rand() % 8 > 2)) {
      float frequencies[5] = { 110.0f, 220.0f * powf(2, 3/12.0f), 220.0f, 880.0f, 55.0f };
      resonator.set_frequency(frequencies[rand() % 5] / ::kSampleRate);
      resonator.set_geometry((rand() % 32768) / 32768.0f);
      resonator.set_brightness((rand() % 32768) / 32768.0f);
      resonator.set_damping((rand() % 32768) / 32768.0f);
      noise_level = (rand() % 4) == 1 ? 1.0f : 0.0f;
      if (noise_level) {
        resonator.set_damping(0.7f);
        resonator.set_brightness((rand() % 32768) / 65535.0f);
      } else{
        impulse = 1.0f;
      }
    }
    
    impulse = impulse * 0.99f;

    float bow_strength = 0.0f;
    float output;
    float aux;
    float input = impulse;
    input += ((rand() % 32768) - 16384) / 65535.0f * noise_level;
    resonator.Process(&bow_strength, &input, &output, &aux, 1);

    output = output * 32768.0f;
    if (output > 32767) output = 32767;
    if (output < -32767) output = -32767;
    short output_sample = output;
    fwrite(&output_sample, sizeof(int16_t), 1, fp);
  }
  fclose(fp);
}

void TestExciter() {
  FILE* fp = fopen("elements_exciter.wav", "wb");
  write_wav_header(fp, ::kSampleRate * 10, 4);
  
  float diffuser_buffer[1024];
  
  Exciter exciter;
  exciter.Init();
  exciter.set_model(EXCITER_MODEL_PLECTRUM);
  exciter.set_parameter(0.7f);
  exciter.set_timbre(0.5f);
  exciter.set_signature(0.1f);
  
  Resonator resonator;
  resonator.Init();
  resonator.set_frequency(262.0f / ::kSampleRate / 2);
  resonator.set_geometry(0.3f);
  resonator.set_brightness(0.8f);
  resonator.set_damping(0.3f);
  resonator.set_position(0.1f);
  resonator.set_resolution(48);

  bool previous_gate = false;
  for (uint32_t i = 0; i < ::kSampleRate * 10; ++i) {
    uint16_t tri = (i / 8);
    tri = tri > 32767 ? 65535 - tri : tri;

    uint16_t tri2 = (i * 1.5);
    tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;
    
    bool gate = (i % (::kSampleRate / 2)) < (::kSampleRate / 4);
    
    uint8_t flags = 0;
    if (gate) flags |= EXCITER_FLAG_GATE;
    if (gate && !previous_gate) flags |= EXCITER_FLAG_RISING_EDGE;
    if (!gate && previous_gate) flags |= EXCITER_FLAG_FALLING_EDGE;
    previous_gate = gate;
    
    exciter.set_parameter(tri / 32768.0f);
    // exciter.set_timbre(tri / 32768.0f);

    float bow_strength = 0.0f;
    float output[4];
    short output_sample[4];
    exciter.Process(flags, &output[0], 1);
    // output[0] *= 0.15;
    output[1] = exciter.damping();
    resonator.Process(&bow_strength, &output[0], &output[2], &output[3], 1);
    for (int j = 0; j < 4; ++j) {
      output[j] *= 32767.0f;
      if (output[j] > 32767) output[j] = 32767;
      if (output[j] < -32767) output[j] = -32767;
      output_sample[j] = output[j];
    }
    fwrite(output_sample, sizeof(int16_t), 4, fp);
  }
  fclose(fp);
}

void TestVoice() {
  FILE* fp = fopen("elements_voice.wav", "wb");
  write_wav_header(fp, ::kSampleRate * 20, 4);
  
  Voice voice;
  Patch p;
  p.exciter_envelope_shape = 0.95f;
  p.exciter_bow_level = 0.5f;
  p.exciter_bow_timbre = 0.6f;
  p.exciter_blow_level = 0.0f;
  p.exciter_blow_meta = 0.5f;
  p.exciter_blow_timbre = 0.2f;
  p.exciter_strike_level = 0.0f;
  p.exciter_strike_meta = 0.5f;
  p.exciter_strike_timbre = 0.5f;
  p.resonator_geometry = 0.2f;
  p.resonator_brightness = 0.9f;
  p.resonator_damping = 0.3f;
  p.resonator_position = 0.3f;

  voice.Init();
  
  for (uint32_t i = 0; i < ::kSampleRate * 20; ++i) {
    uint16_t tri = (i / 8);
    tri = tri > 32767 ? 65535 - tri : tri;
    p.resonator_damping = tri / 32768.0;
    
    bool gate = (i % (::kSampleRate * 4)) < (::kSampleRate * 2);
    float blow_in = 0.0f;
    float strike_in = 0.0f;
    float raw_out = 0.0f;
    float center = 0.0f;
    float sides = 0.0f;
    
    voice.Process(
        p,
        262.0f / ::kSampleRate / 2.0f,
        1.0f,
        gate,
        &blow_in,
        &strike_in,
        &raw_out,
        &center,
        &sides, 1);
    
    float output[4];
    short output_sample[4];
    output[0] = center + (sides) * 0.5f;
    output[1] = center - (sides) * 0.5f;
    output[2] = center;
    output[3] = raw_out;
    
    for (int j = 0; j < 4; ++j) {
      output[j] *= 32767.0f * (j != 3 ? 0.5f : 1.0f);
      if (output[j] > 32767) output[j] = 32767;
      if (output[j] < -32767) output[j] = -32767;
      output_sample[j] = output[j];
    }
    fwrite(output_sample, sizeof(int16_t), 4, fp);
  }
  fclose(fp);
}

void TestPart() {
  FILE* fp = fopen("elements_part.wav", "wb");
  write_wav_header(fp, ::kSampleRate * 20, 2);

  uint16_t reverb_buffer[32768];
  Part part;
  part.Init(reverb_buffer);

  Patch* p = part.mutable_patch();
  
  p->exciter_envelope_shape = 0.0f;
  p->exciter_bow_level = 0.0f;
  p->exciter_bow_timbre = 0.0f;
  p->exciter_blow_level = 0.0f;
  p->exciter_blow_meta = 0.0f;
  p->exciter_blow_timbre = 0.0f;
  p->exciter_strike_level = 0.5f;
  p->exciter_strike_meta = 0.5f;
  p->exciter_strike_timbre = 0.3f;
  p->resonator_geometry = 0.4f;
  p->resonator_brightness = 0.7f;
  p->resonator_damping = 0.8f;
  p->resonator_position = 0.3f;
  p->space = 0.1f;
  
  
  // p->exciter_envelope_shape = 0.99f;
  // p->exciter_bow_level = 0.0f;
  // p->exciter_bow_timbre = 0.6f;
  // p->exciter_blow_level = 0.0f;
  // p->exciter_blow_meta = 0.0f;
  // p->exciter_blow_timbre = 0.0f;
  // p->exciter_strike_level = 0.6f;
  // p->exciter_strike_meta = 0.5f;
  // p->exciter_strike_timbre = 0.5f;
  // p->resonator_geometry = 0.85f;
  // p->resonator_brightness = 1.0f;
  // p->resonator_damping = 0.6f;
  // p->resonator_position = 0.3f;
  // p->space = 0.1f;

  // p->exciter_envelope_shape = 0.0f;
  // p->exciter_bow_level = 0.0f;
  // p->exciter_bow_timbre = 0.5f;
  // p->exciter_blow_level = 0.0f;
  // p->exciter_blow_meta = 0.0f;
  // p->exciter_blow_timbre = 0.5f;
  // p->exciter_strike_level = 1.0f;
  // p->exciter_strike_meta = 0.2f;
  // p->exciter_strike_timbre = 0.6f;
  // p->resonator_geometry = 0.25f;
  // p->resonator_brightness = 0.3f;
  // p->resonator_damping = 0.5f;
  // p->resonator_position = 0.4f;
  // p->space = 1.0;

  // p->exciter_envelope_shape = 0.99f;
  // p->exciter_bow_level = 0.6f;
  // p->exciter_bow_timbre = 0.6f;
  // p->exciter_blow_level = 0.0f;
  // p->exciter_blow_meta = 0.0f;
  // p->exciter_blow_timbre = 0.0f;
  // p->exciter_strike_level = 0.0f;
  // p->exciter_strike_meta = 0.0f;
  // p->exciter_strike_timbre = 0.0f;
  // p->resonator_geometry = 0.25f;
  // p->resonator_brightness = 0.8f;
  // p->resonator_damping = 0.3f;
  // p->resonator_position = 0.3f;
  // p->space = 0.9f;

  float sequence[] = { 69.0f, 57.0f, 45.0f, 57.0f, 69.0f };
  // float sequence[] = { 19, 19, 19, 19, 19 };
  int sequence_counter = -1;

  float silence[16];
  std::fill(&silence[0], &silence[16], 0.0f);

  for (uint32_t i = 0; i < ::kSampleRate * 20; i += 16) {
    uint16_t tri = (i * 1);
    tri = tri > 32767 ? 65535 - tri : tri;

    uint16_t tri2 = (i / 6);
    tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;

    float main[16];
    float aux[16];
    
    // p->resonator_position = int(256.0f * tri / 32768.0f) / 256.0f;
    // p->resonator_geometry = 0.5f + 0.5f * tri / 32768.0f;
    
    //p->exciter_strike_meta = 0.0f + 1.0f * tri / 32768.0f;
    

    if (i % (::kSampleRate * 2) == 0) {
      sequence_counter = (sequence_counter + 1) % 5;
    }

    PerformanceState performance;
    performance.note = sequence[sequence_counter] - 12.0f;
    performance.modulation = 0.0f; /*i & 16 ? 60.0f : -60.0f;
    if (i > ::kSampleRate * 5) {
      performance.modulation = 0;
    }*/
    performance.strength = 0.5f;
    performance.gate = (i % (::kSampleRate / 1)) < (::kSampleRate / 2);

    part.Process(performance, silence, silence, main, aux, 16);

    for (size_t j = 0; j < 16; ++j) {
      float output[2];
      short output_sample[2];
      output[0] = main[j];
      output[1] = aux[j];

      for (int k = 0; k < 2; ++k) {
        output[k] *= 32767.0f;
        if (output[k] > 32767) output[k] = 32767;
        if (output[k] < -32767) output[k] = -32767;
        output_sample[k] = output[k];
      }
      fwrite(output_sample, sizeof(int16_t), 2, fp);
    }
  }
  fclose(fp);
}


void TestEasterEgg() {
  FILE* fp = fopen("elements_easter_egg.wav", "wb");
  write_wav_header(fp, ::kSampleRate * 20, 2);

  uint16_t reverb_buffer[32768];
  Part part;
  part.Init(reverb_buffer);

  Patch* p = part.mutable_patch();
  
  part.set_easter_egg(true);
  
  // p->exciter_envelope_shape = 1.0f;
//   p->exciter_bow_level = 0.0f;
//   p->exciter_bow_timbre = 1.0f;
//   p->exciter_blow_level = 0.0f;
//   p->exciter_blow_meta = 0.27f;
//   p->exciter_blow_timbre = 0.3f;
//   p->exciter_strike_level = 1.0f;
//   p->exciter_strike_meta = 0.5f;
//   p->exciter_strike_timbre = 0.8f;
//   p->resonator_geometry = 0.3f;
//   p->resonator_brightness = 0.2f;
//   p->resonator_damping = 0.9f;
//   p->resonator_position = 0.0f;
//   p->space = 0.5f;
  
  p->exciter_envelope_shape = 0.2f;
  p->exciter_bow_level = 0.52f;
  p->exciter_bow_timbre = 0.8f;

  p->exciter_blow_level = 0.5f;
  p->exciter_blow_meta = 0.5f;
  p->exciter_blow_timbre = 0.0f;

  p->exciter_strike_level = 0.0f;
  p->exciter_strike_meta = 0.83f;
  p->exciter_strike_timbre = 0.5f;

  p->resonator_geometry = 0.0f;
  p->resonator_brightness = 1.0f;
  p->resonator_damping = 0.0f;
  p->resonator_position = 0.0f;
  p->space = 0.2f;

  float sequence[] = { 69.0f, 57.0f, 45.0f, 57.0f, 55.0f };
  int sequence_counter = -1;

  float silence[16];
  std::fill(&silence[0], &silence[16], 0.0f);

  for (uint32_t i = 0; i < ::kSampleRate * 20; i += 16) {
    uint16_t tri = (i / 2);
    tri = tri > 32767 ? 65535 - tri : tri;

    uint16_t tri2 = (i * 3);
    tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;

    float main[16];
    float aux[16];
    
    // p->exciter_blow_meta = 0.5f + 0.5f * (tri / 32768.0f);
    // p->resonator_brightness = 0.0f + 1.0f * tri2 / 32768.0f;
    // p->resonator_position = 0.0f + 0.3f * tri / 32768.0f;
    // p->resonator_brightness = 0.0f + 1.0f * tri2 / 32768.0f;

    if (i % (::kSampleRate / 2) == 0) {
      sequence_counter = (sequence_counter + 1) % 5;
    }

    PerformanceState performance;
    //performance.note = sequence[sequence_counter] - 12;
    performance.note = 96.0f + (tri / 32768.0f) * 48.0;
    performance.modulation = 0.0f;
    performance.strength = 1.0f;
    performance.gate = true; // (i % (::kSampleRate / 2)) < (::kSampleRate / 4);

    part.Process(performance, silence, silence, main, aux, 16);

    for (size_t j = 0; j < 16; ++j) {
      float output[2];
      short output_sample[2];
      output[0] = main[j];
      output[1] = aux[j];

      for (int k = 0; k < 2; ++k) {
        output[k] *= 32767.0f;
        if (output[k] > 32767) output[k] = 32767;
        if (output[k] < -32767) output[k] = -32767;
        output_sample[k] = output[k];
      }
      fwrite(output_sample, sizeof(int16_t), 2, fp);
    }
  }
  fclose(fp);
}

void TestFilterAccuracy() {
  Svf f;
  
  for (int i = 0; i < 128; ++i) {
    float midi_note = i / 1.0f;
    float frequency = 440.0f * powf(2.0f, (midi_note - 69.0f) / 12.0f);
    frequency /= ::kSampleRate;
    
    float g[4];
    f.set_f_q<FREQUENCY_EXACT>(frequency, 0.5f);
    g[0] = f.g();
    f.set_f_q<FREQUENCY_ACCURATE>(frequency, 0.5f);
    g[1] = f.g();
    f.set_f_q<FREQUENCY_FAST>(frequency, 0.5f);
    g[2] = f.g();
    f.set_f_q<FREQUENCY_DIRTY>(frequency, 0.5f);
    g[3] = f.g();
    
    printf("Frequency: %f", frequency * ::kSampleRate);
    for (int j = 0; j < 4; ++j) {
      float error_cts = logf(atanf(g[j]) / M_PI / frequency) / logf(2) * 1200;
      printf("\t%.2f", error_cts);
    }
    printf("\n");
  }
}


int main(void) {
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  // TestFilterAccuracy();
  TestPart();
  // TestExciter();
  // TestResonator();
  // TestEasterEgg();
}
