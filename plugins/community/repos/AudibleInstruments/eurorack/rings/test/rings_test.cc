// Copyright 2015 Olivier Gillet.
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

#include "rings/dsp/part.h"
#include "rings/dsp/onset_detector.h"
#include "rings/dsp/string_synth_part.h"
#include "rings/dsp/string_synth_oscillator.h"
#include "rings/dsp/string_synth_voice.h"

#include "stmlib/test/wav_writer.h"
#include "stmlib/dsp/units.h"
#include "stmlib/utils/random.h"

using namespace rings;
using namespace stmlib;

const uint32_t kSampleRate = 48000;
const uint16_t kAudioBlockSize = 24;

uint16_t reverb_buffer[65536];

void TestModal() {
  WavWriter wav_writer(2, ::kSampleRate, 20);
  wav_writer.Open("rings_modal.wav");

  Part part;
  part.Init(reverb_buffer);

  Patch patch;
  
  patch.structure = 0.25f;
  patch.brightness = 0.3f;
  patch.damping = 0.95f;
  patch.position = 0.75f;
  
  float sequence[] = { 69.0f, 57.0f, 45.0f, 57.0f, 69.0f };
  int sequence_counter = -1;

  float impulse = 1.0f;
  
  part.set_polyphony(4);
  part.set_model(RESONATOR_MODEL_MODAL);
  
  for (uint32_t i = 0; i < ::kSampleRate * 20; i += kAudioBlockSize) {
    uint16_t tri = (i * 1);
    tri = tri > 32767 ? 65535 - tri : tri;

    uint16_t tri2 = (i / 6);
    tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;

    float in[kAudioBlockSize];
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    
    PerformanceState performance;
    performance.strum = false;
    
    if (i % (::kSampleRate * 2) == 0) {
      sequence_counter = (sequence_counter + 1) % 5;
      impulse = 1.0f;
      performance.strum = true;
    }
    
    for (size_t j = 0; j < kAudioBlockSize; ++j) {
      in[j] = impulse;
      impulse = impulse * 0.99f;
    }
    
    performance.note = sequence[sequence_counter] - 12.0f;
    performance.tonic = 0.0f;
    performance.fm = tri / 32768.0f;
    performance.internal_exciter = false;

    part.Process(performance, patch, in, out, aux, kAudioBlockSize);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestString() {
  WavWriter wav_writer(2, ::kSampleRate, 20);
  wav_writer.Open("rings_string.wav");
  
  Part part;
  part.Init(reverb_buffer);

  Patch patch;
  
  //patch.structure = 0.8f;
  patch.brightness = 0.3f;
  patch.damping = 0.8f;
  patch.position = 0.08f;
  patch.structure = 0.25f;
  
  float sequence[] = { 69.0f, 57.0f, 45.0f, 57.0f, 69.0f };
  int sequence_counter = -1;

  part.set_polyphony(3);
  part.set_model(RESONATOR_MODEL_STRING);
  
  for (uint32_t i = 0; i < ::kSampleRate * 20; i += kAudioBlockSize) {
    uint16_t tri = (i / 2);
    tri = tri > 32767 ? 65535 - tri : tri;

    uint16_t tri2 = (i / 6);
    tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;

    float in[kAudioBlockSize];
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    
    // patch.position = 0.75f * tri / 32768.0f + 0.125f;
    //patch.structure = 0.5f * tri2 / 32768.0f + 0.5f;
    //patch.structure = 0.05f;
    patch.position = 0.85f;
    
    PerformanceState performance;
    performance.strum = false;
    performance.internal_exciter = true;
    
    if (i % (::kSampleRate /4 ) == 0) {
      sequence_counter = (sequence_counter + 1) % 5;
      performance.strum = true;
    }
    
    performance.note = sequence[sequence_counter] - 45.0f;
    performance.tonic = 45.0f - 12.0f;
    performance.fm = tri / 32768.0f * 0.00f - 0.0f + 0.0f * Random::GetFloat();

    part.Process(performance, patch, in, out, aux, kAudioBlockSize);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestFM() {
  FILE* fp_in = fopen("audio_samples/funk_kit.wav", "rb");
  fseek(fp_in, 48, SEEK_SET);
  
  WavWriter wav_writer(2, ::kSampleRate, 20);
  wav_writer.Open("rings_fm.wav");

  Part part;
  part.Init(reverb_buffer);

  Patch patch;
  
  patch.brightness = 0.9f;
  patch.damping = 0.5f;
  patch.position = 0.5f;
  patch.structure = 0.5f;
  
  float sequence[] = { 69.0f, 57.0f, 45.0f, 57.0f, 69.0f };
  int sequence_counter = -1;

  part.set_polyphony(1);
  part.set_model(RESONATOR_MODEL_FM_VOICE);
  
  typedef struct { short l; short r; } ShortFrame;
  
  for (uint32_t i = 0; i < ::kSampleRate * 20; i += kAudioBlockSize) {
    ShortFrame input_frame[kAudioBlockSize];
    bool no_input = true || fread(
            input_frame,
            sizeof(ShortFrame),
            kAudioBlockSize,
            fp_in) != kAudioBlockSize;
    
    uint16_t tri = (i / 2);
    tri = tri > 32767 ? 65535 - tri : tri;

    uint16_t tri2 = (i / 6);
    tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;

    float in[kAudioBlockSize];
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    
    // patch.brightness = tri / 32768.0f;
    //patch.position = tri / 32768.0f;
    
    PerformanceState performance;
    performance.strum = false;
    performance.internal_exciter = true;
    
    if (i % (::kSampleRate / 2) == 0) {
      sequence_counter = (sequence_counter + 1) % 6;
      performance.strum = true;
    }
    
    performance.note = /*sequence[sequence_counter] - 45.0f*/ - 3.0f - 12.0f * (sequence_counter & 1);
    performance.tonic = 45.0f;
    performance.fm = tri / 32768.0f * 0.00f - 0.0f + 0.0f * Random::GetFloat();
    
    for (size_t j = 0; j < kAudioBlockSize; ++j) {
      in[j] = no_input ? 0.0f : input_frame[j].l / 32768.0f;
    }

    part.Process(performance, patch, in, out, aux, kAudioBlockSize);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
  fclose(fp_in);
}


void TestLowDelay() {
  FILE* fp_in = fopen("audio_samples/funk_kit.wav", "rb");
  fseek(fp_in, 48, SEEK_SET);
  
  WavWriter wav_writer(2, ::kSampleRate, 20);
  wav_writer.Open("rings_low_delay.wav");
  
  Part part;
  part.Init(reverb_buffer);

  Patch patch;
  
  patch.brightness = 1.0f;
  patch.damping = 0.9f;
  patch.position = 0.5f;
  patch.structure = 0.5f;
  
  float sequence[] = { 69.0f, 57.0f, 45.0f, 57.0f, 69.0f };
  int sequence_counter = -1;

  part.set_polyphony(1);
  part.set_model(RESONATOR_MODEL_STRING);
  
  typedef struct { short l; short r; } ShortFrame;
  
  for (uint32_t i = 0; i < ::kSampleRate * 20; i += kAudioBlockSize) {
    ShortFrame input_frame[kAudioBlockSize];
    bool no_input = fread(
            input_frame,
            sizeof(ShortFrame),
            kAudioBlockSize,
            fp_in) != kAudioBlockSize;
    
    uint16_t tri = (i / 2);
    tri = tri > 32767 ? 65535 - tri : tri;

    uint16_t tri2 = (i / 6);
    tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;

    float in[kAudioBlockSize];
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    
    // patch.brightness = tri / 32768.0f;
    //patch.position = tri / 32768.0f;
    
    PerformanceState performance;
    performance.strum = false;
    performance.internal_exciter = false;
    
    performance.note = 0.0f;
    performance.tonic = 19.0f;
    performance.fm = -48.0f;
    
    for (size_t j = 0; j < kAudioBlockSize; ++j) {
      in[j] = no_input ? 0.0f : input_frame[j].l / 32768.0f;
    }

    part.Process(performance, patch, in, out, aux, kAudioBlockSize);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
  fclose(fp_in);
}

void TestPitchAccuracy() {
  for (int32_t octave = 0; octave < 8; ++octave) {
    int32_t note = octave * 12 + 9;
    char name[80];
    sprintf(name, "rings_note_%d.wav", note);
    
    WavWriter wav_writer(1, ::kSampleRate, 5);
    wav_writer.Open(name);
  
    Part part;
    part.Init(reverb_buffer);

    Patch patch;
    patch.brightness = 0.95f;
    patch.damping = 0.85f;
    patch.position = 0.6f;
    patch.structure = 0.25f;

    part.set_polyphony(1);
    part.set_model(RESONATOR_MODEL_STRING);
    for (uint32_t i = 0; i < ::kSampleRate * 5; i += kAudioBlockSize) {
      float in[kAudioBlockSize];
      float out[kAudioBlockSize];
      float aux[kAudioBlockSize];
      std::fill(&in[0], &in[kAudioBlockSize], 0.0f);
      
      PerformanceState performance;
      performance.strum = i == 0;
      performance.internal_exciter = true;
      performance.note = 0.0f;
      performance.tonic = note;
      performance.fm = 0.0f;
      
      part.Process(performance, patch, in, out, aux, kAudioBlockSize);
      wav_writer.Write(out, kAudioBlockSize);
    }
  }
}

void TestNoteFilter() {
  const size_t kControlRate = ::kSampleRate / kAudioBlockSize;
  
  NoteFilter note_filter;
  note_filter.Init(kControlRate, 0.001f, 0.010f, 0.050f, 0.004f);
  
  // The ADC range is 84.0
  // The maximum ADC noise is 1/256.0 sample.
  
  size_t note_counter = 0;
  
  FILE* fp = fopen("rings_pitch_filter.txt", "w");
  
  float note = 0.0f;
  for (size_t i = 0; i < kControlRate * 2; ++i) {
    float t = i / float(kControlRate);
    if (note_counter == 0) {
      note_counter = kControlRate >> 1;
      note = roundf(Random::GetFloat() * 84.0f) + 19.0f;
    }
    --note_counter;
    float adc_noise = Random::GetFloat() * 0.32f - 0.16f;
    float filtered_note = note_filter.Process(note + adc_noise, false);
    
    fprintf(
        fp,
        "%f\t%f\t%f\t%f\n",
        t,
        note + adc_noise,
        filtered_note,
        note_filter.stable_note());
  }
  fclose(fp);
  
  // Plot results with:
  // import numpy
  // import pylab
  //
  // data = numpy.loadtxt('rings_pitch_filter.txt')
  // t = data[:, 0]
  // adc_pitch = data[:, 1]
  // pitch = data[:, 2]
  // stable_pitch = data[:, 3]
  //
  // pylab.plot(t, adc_pitch, 'r')
  // pylab.plot(t, pitch, 'b')
  // pylab.plot(t, stable_pitch, 'g')
  // pylab.show()
}

void TestOnsetDf() {
  FILE* fp = fopen("rings_onset_df.wav", "wb");
  //FILE* fp_in = fopen("audio_samples/tapping_48k.wav", "rb");
  FILE* fp_in = fopen("audio_samples/funk_kit.wav", "rb");
  fseek(fp_in, 48, SEEK_SET);
  
  WavWriter wav_writer(2, ::kSampleRate, 20);
  wav_writer.Open("rings_onset_df.wav");
  OnsetDetector detector;
  detector.Init(
      8.0f / ::kSampleRate,
      160.0f / ::kSampleRate,
      1600.0f / ::kSampleRate,
      2000.0f,
      0.05f);

  typedef struct { short l; short r; } ShortFrame;
  
  float sawtooth = 0.0f;
  for (uint32_t i = 0; i < ::kSampleRate * 20; i += kAudioBlockSize) {
    ShortFrame input_frame[kAudioBlockSize];
    bool no_input = fread(
            input_frame,
            sizeof(ShortFrame),
            kAudioBlockSize,
            fp_in) != kAudioBlockSize;
    
    float in[kAudioBlockSize];
    float out[kAudioBlockSize];

    for (size_t j = 0; j < kAudioBlockSize; ++j) {
      in[j] = no_input ? 0.0f : input_frame[j].l / 32768.0f;
    }
    
    // for (size_t j = 0; j < kAudioBlockSize; ++j) {
    //   in[j] = 2.0f * sawtooth - 1.0f;
    //   sawtooth += 220.0f / ::kSampleRate;
    //   if (sawtooth >= 1.0f) sawtooth -= 1.0f;
    // }
    
    float onset_df = detector.Process(in, kAudioBlockSize);
    fill(&out[0], &out[kAudioBlockSize], onset_df);
    wav_writer.Write(in, out, kAudioBlockSize);
  }
  fclose(fp_in);
}

void TestGain() {
  uint32_t block_duration = ::kSampleRate * 5;
  WavWriter wav_writer(2, ::kSampleRate, 20);
  wav_writer.Open("rings_gain.wav");
  

  Part part;
  part.Init(reverb_buffer);

  Patch patch;
  
  part.set_polyphony(1);
  for (uint32_t i = 0; i < block_duration * 4; i += kAudioBlockSize) {
    if (i % block_duration == 0) {
      part.set_model(ResonatorModel(i / block_duration));
    }
    
    float in[kAudioBlockSize];
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    
    patch.brightness = 0.5f;
    patch.position = 0.8f;
    patch.structure = 0.25f;
    patch.damping = 0.7f;
    
    PerformanceState performance;
    performance.strum = (i % (::kSampleRate / 2)) == 0;
    performance.internal_exciter = (i % block_duration) < (block_duration / 2);
    performance.note = 0.0f;
    performance.tonic = 48.0f;
    performance.fm = 0.0f;
    
    if (performance.internal_exciter) {
      fill(&in[0], &in[kAudioBlockSize], 0.0f);
    } else {
      for (size_t j = 0; j < kAudioBlockSize; ++j) {
        in[j] = (Random::GetFloat() * 2.0f - 1.0f) * 0.25f;
      }
    }
    
    part.Process(performance, patch, in, out, aux, kAudioBlockSize);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

void TestStringSynthOscillator() {
  WavWriter wav_writer(1, ::kSampleRate, 10);
  wav_writer.Open("rings_string_synth_oscillator.wav");
  StringSynthOscillator osc;
  osc.Init();
  while (!wav_writer.done()) {
    float out[kAudioBlockSize];
    fill(&out[0], &out[kAudioBlockSize], 0.0f);
    osc.Render<OSCILLATOR_SHAPE_DARK_SQUARE, false>(
        100.0f / 48000.00f, 0.5f, 0.5f, out, kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize, 32767.0f);
  }
}

void TestStringSynthVoice() {
  WavWriter wav_writer(1, ::kSampleRate, 10);
  wav_writer.Open("rings_string_synth_voice.wav");
  
  StringSynthVoice<3> voice;
  voice.Init();
  
  float amplitudes[6] = { 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f };
  
  while (!wav_writer.done()) {
    float out[kAudioBlockSize];
    fill(&out[0], &out[kAudioBlockSize], 0.0f);
    float base_frequency = (100.0f / 48000.0f) * (wav_writer.triangle(1) >= 0.5f ? 1.0f : 1024.0f);
    voice.Render(
        base_frequency,
        amplitudes,
        3,
        out,
        kAudioBlockSize);
    wav_writer.Write(out, kAudioBlockSize);
  }
}

void TestStringSynthPart() {
  WavWriter wav_writer(2, ::kSampleRate, 48);
  wav_writer.Open("rings_string_synth.wav");
  
  StringSynthPart part;
  part.Init(reverb_buffer);

  Patch patch;
  patch.brightness = 0.3f;
  patch.position = 0.0f;
  patch.structure = 0.42f;

  
  float sequence[] = { 69.0f, 57.0f, 45.0f, 57.0f, 69.0f };
  int sequence_counter = -1;

  part.set_polyphony(2);
  part.set_fx(FX_REVERB);
  
  for (uint32_t i = 0; i < ::kSampleRate * 48; i += kAudioBlockSize) {
    uint16_t tri = (i / 3);
    tri = tri > 32767 ? 65535 - tri : tri;

    uint16_t tri2 = (i / 11);
    tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;
    
    float in[kAudioBlockSize];
    float out[kAudioBlockSize];
    float aux[kAudioBlockSize];
    
    fill(&in[0], &in[kAudioBlockSize], 0.0f);
    
    PerformanceState performance;
    performance.strum = false;
    performance.internal_exciter = true;
    patch.brightness = tri2 / 32768.0f;
    //patch.damping = 0.6f + tri / 32768.0f * 0.2f;
    patch.damping = 0.8f;
    
    
    if (i % (::kSampleRate * 2) == 0 && i < ::kSampleRate * 40) {
      sequence_counter = (sequence_counter + 1) % 5;
      performance.strum = true;
    }
    
    performance.note = sequence[sequence_counter] - 45.0f;
    performance.tonic = 45.0f - 2.0f;
    performance.fm = tri2 / 32768.0f * 0.10f - 0.0f + 0.0f * Random::GetFloat();

    part.Process(performance, patch, in, out, aux, kAudioBlockSize);
    wav_writer.Write(out, aux, kAudioBlockSize);
  }
}

int main(void) {
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  TestNoteFilter();
  TestModal();
  TestString();
  // TestFM();
  // TestLowDelay();
  TestPitchAccuracy();
  // TestOnsetDf();
  TestGain();
  TestStringSynthOscillator();
  TestStringSynthVoice();
  TestStringSynthPart();
}
