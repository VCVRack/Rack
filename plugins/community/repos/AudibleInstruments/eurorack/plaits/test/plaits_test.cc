// Copyright 2016 Olivier Gillet.
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

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <xmmintrin.h>

#include "plaits/dsp/dsp.h"

#include "plaits/dsp/engine/additive_engine.h"
#include "plaits/dsp/engine/bass_drum_engine.h"
#include "plaits/dsp/engine/chord_engine.h"
#include "plaits/dsp/engine/fm_engine.h"
#include "plaits/dsp/engine/grain_engine.h"
#include "plaits/dsp/engine/hi_hat_engine.h"
#include "plaits/dsp/engine/modal_engine.h"
#include "plaits/dsp/engine/string_engine.h"
#include "plaits/dsp/engine/noise_engine.h"
#include "plaits/dsp/engine/particle_engine.h"
#include "plaits/dsp/engine/snare_drum_engine.h"
#include "plaits/dsp/engine/speech_engine.h"
#include "plaits/dsp/engine/swarm_engine.h"
#include "plaits/dsp/engine/virtual_analog_engine.h"
#include "plaits/dsp/engine/waveshaping_engine.h"
#include "plaits/dsp/engine/wavetable_engine.h"

#include "plaits/dsp/fx/sample_rate_reducer.h"

#include "plaits/dsp/oscillator/formant_oscillator.h"
#include "plaits/dsp/oscillator/grainlet_oscillator.h"
#include "plaits/dsp/oscillator/harmonic_oscillator.h"
#include "plaits/dsp/oscillator/oscillator.h"
#include "plaits/dsp/oscillator/string_synth_oscillator.h"
#include "plaits/dsp/oscillator/variable_saw_oscillator.h"
#include "plaits/dsp/oscillator/variable_shape_oscillator.h"
#include "plaits/dsp/oscillator/vosim_oscillator.h"
#include "plaits/dsp/oscillator/z_oscillator.h"

#include "plaits/dsp/voice.h"

#include "stmlib/test/wav_writer.h"

using namespace std;
using namespace stmlib;
using namespace plaits;

const size_t kAudioBlockSize = 24;

char ram_block[16 * 1024];

void TestOscillator() {
  WavWriter wav_writer(1, kSampleRate, 20);
  wav_writer.Open("plaits_simple_oscillator.wav");
  
  Oscillator osc;
  osc.Init();
  
  float f = 112.0f / 48000.0f;
  
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    osc.Render<OSCILLATOR_SHAPE_SLOPE>(f, wav_writer.triangle(), out, kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestVariableShapeOscillator() {
  WavWriter wav_writer(1, kSampleRate, 20);
  wav_writer.Open("plaits_slave_oscillator.wav");
  
  VariableShapeOscillator osc;
  osc.Init();
  
  float master_f = 110.0f / 48000.0f;
  float f = 410.0f / 48000.0f;
  
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    osc.Render<true>(
      master_f,
      master_f * (1.0f + 4.0f * wav_writer.triangle()),
      0.5f,
      0.0f,
      out,
      kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestVariableSawOscillator() {
  WavWriter wav_writer(1, kSampleRate, 20);
  wav_writer.Open("plaits_variable_saw.wav");
  
  VariableSawOscillator osc;
  osc.Init();
  
  float master_f = 110.0f / 48000.0f;
  float f = 410.0f / 48000.0f;
  
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    osc.Render(
      //master_f * (1.0f + 4.0f * wav_writer.triangle()),
      62.50f / 48000.0f,
      wav_writer.triangle(),  // pw
      1.0f,  // 0 = notch , 1 = slope
      out,
      kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestStringSynthOscillator() {
  WavWriter wav_writer(1, kSampleRate, 20);
  wav_writer.Open("plaits_string_synth_oscillator.wav");
  
  StringSynthOscillator osc;
  osc.Init();
  
  float amplitudes[7] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
  float f = 127.5f / kSampleRate;
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    fill(&out[0], &out[kAudioBlockSize], 0.0f);
    
    osc.Render(f * (1.0f + 0.0f * wav_writer.triangle(3)), amplitudes, 1.0f, out, kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestHarmonicOscillator() {
  WavWriter wav_writer(1, kSampleRate, 20);
  wav_writer.Open("plaits_harmonic_oscillator.wav");
  
  HarmonicOscillator<16> osc;
  osc.Init();
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    fill(&out[0], &out[kAudioBlockSize], 0.0f);
    float f0 = 10.0f / kSampleRate;
    float amplitudes[16];
    fill(&amplitudes[0], &amplitudes[16], 0.0f);
    amplitudes[15] = 1.0f;
    osc.Render<8>(f0, amplitudes, out, kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestFormantOscillator() {
  WavWriter wav_writer(1, kSampleRate, 20);
  wav_writer.Open("plaits_formant_oscillator.wav");
  
  FormantOscillator osc;
  osc.Init();
  
  float fm = 239.7f / 48000.0f;
  float fs = 105.0f / 48000.0f;
  
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float modulation = 1.0f + 4.0f * wav_writer.triangle();
    osc.Render(fm, fs * modulation, 0.75f, out, kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestVosimOscillator() {
  WavWriter wav_writer(1, kSampleRate, 20);
  wav_writer.Open("plaits_vosim_oscillator.wav");
  
  VOSIMOscillator osc;
  osc.Init();
  
  float f0 = 105.0f / 48000.0f;
  float f1 = 1390.7f / 48000.0f;
  float f2 = 817.2f / 48000.0f;
  
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float modulation = wav_writer.triangle();
    osc.Render(f0, f1 * (1.0f + modulation), f2, modulation, out, kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestZOscillator() {
  WavWriter wav_writer(1, kSampleRate, 20);
  wav_writer.Open("plaits_z_oscillator.wav");
  
  ZOscillator osc;
  osc.Init();
  
  float f0 = 80.0f / 48000.0f;
  float f1 = 250.0f / 48000.0f;
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float modulation = wav_writer.triangle(7);
    float modulation_2 = wav_writer.triangle(11);
    osc.Render(f0, f1 * (1.0f + modulation * 8.0f), modulation_2, 0.5f, out, kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestGrainletOscillator() {
  WavWriter wav_writer(1, kSampleRate, 20);
  wav_writer.Open("plaits_grainlet_oscillator.wav");
  
  GrainletOscillator osc;
  osc.Init();
  
  float f0 = 80.0f / 48000.0f;
  float f1 = 2000.0f / 48000.0f;
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float modulation = wav_writer.triangle(7) * 0.0f;
    float modulation_2 = wav_writer.triangle(11) * 0.0f;
    float modulation_3 = wav_writer.triangle(13);
    osc.Render(f0, f1 * (1.0f + modulation * 8.0f), modulation_3, 1.0f, out, kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestAdditiveEngine() {
  WavWriter wav_writer(2, kSampleRate, 60);
  wav_writer.Open("plaits_additive_engine.wav");
  
  AdditiveEngine e;
  e.Init(NULL);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_LOW;
  p.note = 36.0f;

  for (size_t i = 0; i < kSampleRate * 60; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.morph = wav_writer.triangle(13) * 0.0f + 0.7f;
    p.timbre = wav_writer.triangle(7) * 1.0f;
    p.harmonics = wav_writer.triangle(5) * 0.5f + 0.5f;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestChordEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_chord_engine.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  ChordEngine e;
  e.Init(&allocator);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_LOW;
  p.note = 60.0f;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.harmonics = wav_writer.triangle(17) * 1.0f;
    p.morph = wav_writer.triangle(11) * 0.2f + 0.4f;
    p.timbre = wav_writer.triangle(13) * 1.0f;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestFMEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_fm_engine.wav");
  
  FMEngine e;
  e.Init(NULL);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_LOW;
  p.note = 48.0f;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.timbre = wav_writer.triangle(11);
    p.harmonics = /*wav_writer.triangle(14)*/ 0.75f;
    p.morph = /*1.0f - wav_writer.triangle(19)*/ 0.0f;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestGrainEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_grain_engine.wav");
  
  GrainEngine e;
  e.Init(NULL);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_LOW;
  p.note = 110.0f;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.note = /*84.0f + Random::GetFloat() * 0.1f + wav_writer.triangle(2) * 12.0f*/ 36.0f;
    p.timbre = wav_writer.triangle(7);
    p.morph = wav_writer.triangle(11);
    p.harmonics = wav_writer.triangle(19);
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestModalEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_modal_engine.wav");
  
  ModalEngine e;
  e.Init(NULL);
  e.Reset();
  
  EngineParameters p;
  p.accent = 0.0f;
  p.note = 36.0f;
  bool flip_flop = false;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.trigger = TRIGGER_LOW;
    if (i % (kAudioBlockSize * 2000) == 0) {
      flip_flop = !flip_flop;
      p.note = flip_flop ? 48.0f : 55.0f;
      p.trigger = TRIGGER_RISING_EDGE;
      p.accent = 1.0f;
    }
    p.timbre = wav_writer.triangle(17);
    p.harmonics = 0.25f;
    p.morph = wav_writer.triangle(7);
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestNoiseEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_noise_engine.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  NoiseEngine e;
  e.Init(&allocator);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_LOW;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.note = 84.0f;
    p.timbre = 0.0f;
    p.morph = 0.5f;
    p.harmonics = 0.0f * wav_writer.triangle(3);
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestParticleEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_particle_engine.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  ParticleEngine e;
  e.Init(&allocator);
  e.Reset();
  
  EngineParameters p;
  p.note = 96.0f;
  p.trigger = TRIGGER_LOW;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.timbre = /*wav_writer.triangle(17)*/0.5f;
    p.harmonics = /*0.5f*/ 0.7f;
    p.morph = /*0.0f*/0.7f;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestSpeechEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_speech_engine.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  SpeechEngine e;
  e.Init(&allocator);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_UNPATCHED;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.timbre = wav_writer.triangle(11) * 0.0f + 0.5f;
    p.harmonics = wav_writer.triangle(17) * 0.45f;
    p.note = 48.0f + wav_writer.triangle(1) * 0.0f;
    p.morph = wav_writer.triangle(7);
    // p.trigger = TRIGGER_LOW;
    // if (i % (kAudioBlockSize * 3000) == 0) {
    //   p.trigger = TRIGGER_RISING_EDGE;
    // }
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void GenerateStringTuningData() {
  for (int pass = 0; pass < 21; ++pass) {
    WavWriter wav_writer(1, kSampleRate, 4);
    
    char file_name[80];
    sprintf(file_name, "string_%02d.wav", pass);
    wav_writer.Open(file_name);
    
    BufferAllocator allocator(ram_block, 16384);
    StringEngine e;
    e.Init(&allocator);
    e.Reset();
    
    EngineParameters p;
    p.accent = 0.5f;
    p.note = 72.0f;
    for (size_t i = 0; i < kSampleRate * 4; i += kAudioBlockSize) {
      float out[kAudioBlockSize];
      float aux[kAudioBlockSize];
      p.trigger = i == 0 ? TRIGGER_RISING_EDGE : TRIGGER_LOW;
      p.timbre = 0.8f;
      p.morph = 0.8f;
      p.harmonics = float(pass) / 20.0f;
      bool already_enveloped;
      e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
      wav_writer.Write(out, kAudioBlockSize);
    }
  }
  
  WavWriter wav_writer(1, kSampleRate, 40);
  wav_writer.Open("string_sweep.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  StringEngine e;
  e.Init(&allocator);
  e.Reset();
  
  EngineParameters p;
  p.accent = 0.2f;
  p.note = 36.0f;
  p.timbre = 0.8f;
  p.morph = 0.8f;
  p.trigger = TRIGGER_UNPATCHED;
  for (size_t i = 0; i < kSampleRate * 40; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.harmonics = wav_writer.triangle(7);
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void GenerateModalTuningData() {
  for (int pass = 0; pass < 21; ++pass) {
    WavWriter wav_writer(1, kSampleRate, 4);
    
    char file_name[80];
    sprintf(file_name, "modal_%02d.wav", pass);
    wav_writer.Open(file_name);
    
    BufferAllocator allocator(ram_block, 16384);
    ModalEngine e;
    e.Init(&allocator);
    e.Reset();
    
    EngineParameters p;
    p.accent = 0.5f;
    p.note = 48.0f;
    for (size_t i = 0; i < kSampleRate * 4; i += kAudioBlockSize) {
      float out[kAudioBlockSize];
      float aux[kAudioBlockSize];
      p.trigger = i == (kAudioBlockSize * 1000)
          ? TRIGGER_RISING_EDGE : TRIGGER_LOW;
      p.timbre = 0.5f;
      p.morph = 0.8f;
      p.harmonics = float(pass) / 20.0f;
      bool already_enveloped;
      e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
      wav_writer.Write(out, kAudioBlockSize);
    }
  }
}

void TestStringEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_string_engine.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  StringEngine e;
  e.Init(&allocator);
  e.Reset();
  
  EngineParameters p;
  p.accent = 0.0f;
  p.note = 36.0f;
  int note = 0;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.trigger = TRIGGER_LOW;
    if (i % (kAudioBlockSize * 2000) == 0) {
      note = (note + 1) % 3;
      float notes[3] = { 48.0f, 55.0f, 36.0f };
      p.note = notes[note];
      p.trigger = TRIGGER_RISING_EDGE;
      p.accent = 0.0f;
    }
    p.timbre = 0.7f;
    p.harmonics = 0.9f;
    p.morph = 0.7f;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestSwarmEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_swarm_engine.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  SwarmEngine e;
  e.Init(&allocator);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_UNPATCHED;
  
  Limiter out_limiter;
  Limiter aux_limiter;
  
  out_limiter.Init();
  aux_limiter.Init();

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.timbre = wav_writer.triangle(33) * 0.0f + 0.5f;
    p.harmonics = 0.3f;
    p.morph = wav_writer.triangle(17);
    p.note = 48.0f;
    p.trigger = TRIGGER_LOW;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    
    out_limiter.Process(2.0f, out, kAudioBlockSize);
    aux_limiter.Process(0.8f, aux, kAudioBlockSize);
    
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestVirtualAnalogEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_virtual_analog_engine.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  VirtualAnalogEngine e;
  e.Init(&allocator);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_LOW;
  p.note = 48.0f;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    // p.timbre = wav_writer.triangle(7);
    // p.harmonics = wav_writer.triangle(11);
    // p.morph = 1.0f - wav_writer.triangle(5);
    p.timbre = wav_writer.triangle(3) * 0.0f + 0.0f;
    p.harmonics = wav_writer.triangle(19);
    p.morph = wav_writer.triangle(19) * 0.0f + 0.3f;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestWaveshapingEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_waveshaping_engine.wav");
  
  WaveshapingEngine e;
  e.Init(NULL);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_LOW;
  p.note = 48.0f;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.timbre = 0.1f + 0.9f * wav_writer.triangle(7);
    p.harmonics = 0.0f + 1.0f * wav_writer.triangle(11);
    p.morph = 0.0f + 1.0f * wav_writer.triangle(5);
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestWavetableEngine() {
  WavWriter wav_writer(2, kSampleRate, 5);
  wav_writer.Open("plaits_wavetable_engine.wav");
  
  WavetableEngine e;
  e.Init(NULL);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_LOW;
  p.note = 24.0f;

  for (size_t i = 0; i < kSampleRate * 5; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    float phi = wav_writer.triangle(1);
    p.timbre = phi > 0.9f ? 0.0f : 0.5f + 0.5f * sinf(phi * 24.3f);
    p.harmonics = wav_writer.triangle(11) * 0 + 0.0f;
    p.morph = wav_writer.triangle(5) * 0;
    
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void EnumerateWavetables() {
  WavWriter wav_writer(1, kSampleRate, 64);
  wav_writer.Open("plaits_wavetable_enumeration.wav");
  
  WavetableEngine e;
  e.Init(NULL);
  e.Reset();
  
  EngineParameters p;
  p.trigger = TRIGGER_LOW;
  
  int bank = 0;
  int division = 4;
  bool swap = true;
  
  for (int d = 0; d < division; ++d) {
    for (int column = 0; column < 8; ++column) {
      for (int row = 0; row < 8; ++row) {
        for (size_t i = 0; i < kSampleRate/ division; i += kAudioBlockSize) {
          float out[kAudioBlockSize];
          float aux[kAudioBlockSize];
          p.note = 36.0f;
          p.timbre = (swap ? column : row) / 7.0f;
          p.harmonics = bank / 2.0f;
          p.morph = (swap ? row : column) / 7.0f;
          bool already_enveloped;
          e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
          wav_writer.Write(out, kAudioBlockSize);
        }
      }
    }
  }
}

void TestSampleRateReducer() {
  WavWriter wav_writer(2, kSampleRate, 20);
  wav_writer.Open("plaits_sample_rate_reducer.wav");
  
  SampleRateReducer src;
  SineOscillator osc;
  osc.Init();
  src.Init();
  
  float f0 = 100.0f / 48000.0f;
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float in[kAudioBlockSize];
    float fx[kAudioBlockSize];
    fill(&in[0], &in[kAudioBlockSize], 0.0f);
    float f = 110 / kSampleRate;
    float a = 1.0f;
    osc.Render(f, a, in, kAudioBlockSize);
    copy(&in[0], &in[kAudioBlockSize], &fx[0]);
    src.Process<true>(0.1666f + 0.8333f * wav_writer.triangle(7), fx, kAudioBlockSize);
    wav_writer.Write(in, fx, kAudioBlockSize);
  } 
}

void TestBassDrumEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_bass_drum_engine.wav");
  
  BassDrumEngine e;
  e.Init(NULL);
  e.Reset();
  
  EngineParameters p;
  p.accent = 0.0f;
  p.note = 33.4f;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.trigger = TRIGGER_LOW;
    if (i % (kAudioBlockSize * 1000) == 0) {
      p.trigger = TRIGGER_RISING_EDGE;
      p.accent = 1.0f;
    }
    p.timbre = wav_writer.triangle(5) * 1.0f + 0.0f;
    p.harmonics = wav_writer.triangle(7) * 1.0f + 0.0f;
    p.morph = wav_writer.triangle(17) * 1.0f + 0.0f;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestSnareDrumEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_snare_drum_engine.wav");
  
  SnareDrumEngine e;
  e.Init(NULL);
  e.Reset();
  
  EngineParameters p;
  p.accent = 0.0f;
  p.note = 51.0f;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.trigger = TRIGGER_LOW;
    if (i % (kAudioBlockSize * 1000) == 0) {
      p.trigger = TRIGGER_RISING_EDGE;
      p.accent = 1.0f;
    }
    p.timbre = wav_writer.triangle(5);
    p.harmonics = wav_writer.triangle(7);
    p.morph = wav_writer.triangle(17);
    // p.timbre = 0.5f;
    // p.harmonics = 0.5f;
    // p.morph = 0.0f;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestHiHatEngine() {
  WavWriter wav_writer(2, kSampleRate, 80);
  wav_writer.Open("plaits_hi_hat_engine.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  HiHatEngine e;
  e.Init(&allocator);
  e.Reset();
  
  EngineParameters p;
  p.accent = 0.0f;

  for (size_t i = 0; i < kSampleRate * 80; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    p.trigger = TRIGGER_LOW;
    if (i % (kAudioBlockSize * 250) == 0) {
      p.trigger = TRIGGER_RISING_EDGE;
      p.accent = 1.0f;
    }
    p.note = 48.0f + wav_writer.triangle(11) * 36.0f;
    p.timbre = wav_writer.triangle(17);
    p.harmonics = wav_writer.triangle(7);
    p.morph = /*wav_writer.triangle(3)*/ 0.5f;
    bool already_enveloped;
    e.Render(p, out, aux, kAudioBlockSize, &already_enveloped);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestVoice() {
  WavWriter wav_writer(2, kSampleRate, 200);
  wav_writer.Open("plaits_voice.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  Voice v;
  
  v.Init(&allocator);
  
  Patch patch;
  Modulations modulations;
  
  patch.engine = 1;
  patch.note = 48.0f;
  patch.harmonics = 0.3f;
  patch.timbre = 0.7f;
  patch.morph = 0.7f;
  patch.frequency_modulation_amount = 0.0f;
  patch.timbre_modulation_amount = 0.0f;
  patch.morph_modulation_amount = 0.0f;
  patch.decay = 0.1f;
  patch.lpg_colour = 0.0f;
  
  modulations.note = 0.0f;
  modulations.engine = 0.0f;
  modulations.frequency = 0.0f;
  modulations.note = 0.0f;
  modulations.harmonics = 0.0f;
  modulations.morph = 0.0;
  modulations.level = 1.0f;
  modulations.trigger = 0.0f;
  modulations.frequency_patched = false;
  modulations.timbre_patched = false;
  modulations.morph_patched = false;
  modulations.trigger_patched = true;
  modulations.level_patched = false;
  
  for (size_t i = 0; i < kSampleRate * 200; i += kAudioBlockSize) {
    modulations.trigger = (i % (kAudioBlockSize * 500) <= kAudioBlockSize * 5) ? 1.0f : 0.0f;
    // modulations.level = 1.0f;
    Voice::Frame frames[kAudioBlockSize];
    v.Render(patch, modulations, frames, kAudioBlockSize);
    wav_writer.WriteFrames(&frames[0].out, kAudioBlockSize);
  }
}

void TestFMGlitch() {
  WavWriter wav_writer(2, kSampleRate, 200);
  wav_writer.Open("plaits_fm_glitch.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  Voice v;

  v.Init(&allocator);
  
  Patch patch;
  Modulations modulations;
  
  patch.engine = 4;
  patch.note = 48.0f;
  patch.harmonics = 0.5f;
  patch.timbre = 0.5f;
  patch.morph = 0.5f;
  patch.frequency_modulation_amount = 0.0f;
  patch.timbre_modulation_amount = 0.0f;
  patch.morph_modulation_amount = 0.0f;
  patch.decay = 0.1f;
  patch.lpg_colour = 0.0f;
  
  modulations.note = 0.0f;
  modulations.engine = 0.0f;
  modulations.frequency = 0.0f;
  modulations.note = 0.0f;
  modulations.harmonics = 0.0f;
  modulations.morph = 0.0;
  modulations.level = 1.0f;
  modulations.trigger = 0.0f;
  modulations.frequency_patched = true;
  modulations.timbre_patched = false;
  modulations.morph_patched = false;
  modulations.trigger_patched = false;
  modulations.level_patched = false;
  
  for (size_t i = 0; i < kSampleRate * 200; i += kAudioBlockSize) {
    Voice::Frame frames[kAudioBlockSize];
    v.Render(patch, modulations, frames, kAudioBlockSize);
    wav_writer.WriteFrames(&frames[0].out, kAudioBlockSize);
    modulations.frequency = frames[0].out;
    patch.frequency_modulation_amount = wav_writer.triangle(11) * 1.0f;
  }
}

void TestLPGAttackDecay() {
  WavWriter wav_writer(2, kSampleRate, 20);
  wav_writer.Open("plaits_lpg_attack_decay.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  Voice v;

  v.Init(&allocator);
  
  Patch patch;
  Modulations modulations;
  
  patch.engine = 1;
  patch.note = 48.0f;
  patch.harmonics = 0.5f;
  patch.timbre = 0.0f;
  patch.morph = 0.0f;
  patch.frequency_modulation_amount = 0.8f;
  patch.timbre_modulation_amount = 0.0f;
  patch.morph_modulation_amount = 0.0f;
  patch.decay = 0.1f;
  patch.lpg_colour = 0.5f;
  
  modulations.note = 0.0f;
  modulations.engine = 0.0f;
  modulations.frequency = 0.0f;
  modulations.note = 0.0f;
  modulations.harmonics = 0.0f;
  modulations.morph = 0.0;
  modulations.level = 1.0f;
  modulations.trigger = 0.0f;
  modulations.frequency_patched = false;
  modulations.timbre_patched = false;
  modulations.morph_patched = false;
  modulations.trigger_patched = true;
  modulations.level_patched = false;
    
  for (size_t i = 0; i < kSampleRate * 20; i += kAudioBlockSize) {
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];

    modulations.trigger = 0.0f;
    if (i % (kAudioBlockSize * 1000) == 0) {
      patch.note += 1.0f;
      modulations.trigger = 1.0f;
    }
    modulations.level = (i % (1000 * kAudioBlockSize)) < 100 * kAudioBlockSize ? 1.0f : 0.0f;
    
    Voice::Frame frames[kAudioBlockSize];
    v.Render(patch, modulations, frames, kAudioBlockSize);
    wav_writer.WriteFrames(&frames[0].out, kAudioBlockSize);
  }
}

void TestLimiterGlitch() {
  WavWriter wav_writer(2, kSampleRate, 50);
  wav_writer.Open("plaits_limiter_glitch.wav");
  
  BufferAllocator allocator(ram_block, 16384);
  Voice v;

  v.Init(&allocator);
  
  Patch patch;
  Modulations modulations;
  
  patch.engine = 9;
  patch.note = 36.0f;
  patch.harmonics = 0.8f;
  patch.timbre = 0.6f;
  patch.morph = 0.4f;
  patch.frequency_modulation_amount = 0.0f;
  patch.timbre_modulation_amount = 0.0f;
  patch.morph_modulation_amount = 0.0f;
  patch.decay = 0.1f;
  patch.lpg_colour = 0.0f;
  
  modulations.note = 0.0f;
  modulations.frequency = 0.0f;
  modulations.note = 0.0f;
  modulations.harmonics = 0.0f;
  modulations.morph = 0.0;
  modulations.level = 1.0f;
  modulations.frequency_patched = false;
  modulations.timbre_patched = false;
  modulations.morph_patched = false;
  modulations.trigger_patched = true;
  modulations.level_patched = false;
  
  for (size_t i = 0; i < kSampleRate * 50; i += kAudioBlockSize) {
    Voice::Frame frames[kAudioBlockSize];
    v.Render(patch, modulations, frames, kAudioBlockSize);
    wav_writer.WriteFrames(&frames[0].out, kAudioBlockSize);
    modulations.trigger = i % (100 * kAudioBlockSize) == 0 ? 1.0f : 0.0f;
    modulations.engine = wav_writer.triangle(12) * 0.2f;
    patch.frequency_modulation_amount = wav_writer.triangle(3) * 1.0f;
  }
}

int main(void) {
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  // TestFormantOscillator();
  // TestGrainletOscillator();
  // TestOscillator();
  // TestVariableShapeOscillator();
  // TestStringSynthOscillator();
  // TestStringSynthOscillator();
  // TestVosimOscillator();
  // TestZOscillator();
  // TestHarmonicOscillator();

  // TestAdditiveEngine();
  // TestChordEngine();
  TestFMEngine();
  // TestGrainEngine();
  // TestModalEngine();
  // TestStringEngine();
  // TestNoiseEngine();
  // TestParticleEngine();
  // TestSpeechEngine();
  // TestSwarmEngine();
  // TestVirtualAnalogEngine();
  // TestWaveshapingEngine();
  // TestWavetableEngine();
  // TestBassDrumEngine();
  // TestSnareDrumEngine();
  // TestHiHatEngine();
  
  // TestVariableSawOscillator();
  
  // TestSampleRateReducer();
  // TestVoice();
  // TestFMGlitch();
  // TestLimiterGlitch();
  // EnumerateWavetables();
  
  // TestLPGAttackDecay();
}
