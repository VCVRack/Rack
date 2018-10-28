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

#include "marbles/cv_reader_channel.h"
#include "marbles/note_filter.h"
#include "marbles/ramp/ramp_divider.h"
#include "marbles/ramp/ramp_extractor.h"
#include "marbles/random/distributions.h"
#include "marbles/random/output_channel.h"
#include "marbles/random/random_generator.h"
#include "marbles/random/random_sequence.h"
#include "marbles/random/random_stream.h"
#include "marbles/random/t_generator.h"
#include "marbles/random/x_y_generator.h"
#include "marbles/scale_recorder.h"
#include "marbles/test/fixtures.h"
#include "marbles/test/ramp_checker.h"
#include "stmlib/test/wav_writer.h"
#include "stmlib/utils/random.h"

using namespace marbles;
using namespace std;
using namespace stmlib;


void TestBetaDistribution() {
  // Plot result with:
  // import numpy
  // import pylab
  // data = numpy.loadtxt('marbles_histograms.txt')
  // n = 0
  // for i in xrange(9):
  //   for j in xrange(13):
  //     pylab.subplot(9, 13, n + 1)
  //     pylab.plot(data[(n * 101):((n + 1) * 101)])
  //     pylab.gca().get_xaxis().set_visible(False)
  //     pylab.gca().get_yaxis().set_visible(False)
  //     n += 1
  // pylab.show()
  FILE* fp = fopen("marbles_histograms.txt", "w");
  for (int i = 0; i < 9; ++i) {
    for (int j = 0; j < 13; ++j) {
      float bias = float(i) / 8.0f;
      float range = float(j) / 12.0f;
      vector<int> histogram(101);
      for (int n = 0; n < 1000000; ++n) {
        float value = BetaDistributionSample(Random::GetFloat(), range, bias);
        histogram[int(value * 100.0f)]++;
      }
      for (int n = 0; n < 101; ++n) {
        fprintf(fp, "%d\n", histogram[n]);
      }
    }
  }
  fclose(fp);
}

void TestQuantizer() {
  // Plot result with:
  // import numpy
 //  import pylab
 //
  // data = numpy.loadtxt('marbles_quantizer.txt')
  // pylab.figure(figsize=(25,5))
  // for i in xrange(9):
  //   pylab.subplot(1, 9, i + 1)
  //   indices = numpy.where(data[:, 0] == i)[0]
  //   pylab.plot(data[indices, 1], data[indices, 2])
  // pylab.show()
  FILE* fp = fopen("marbles_quantizer.txt", "w");
  
  Quantizer q;
  Scale scale;
  scale.InitMajor();
  q.Init(scale);
  
  for (int i = 0; i <= 8; ++i) {
    float amount = float(i) / 8.0f;
    for (int j = 0; j <= 4000; ++j) {
      float value = j / 1000.0f - 2.0f;
      fprintf(fp, "%d %f %f\n", i, value, q.Process(value, amount, false));
    }
  }
  fclose(fp);
}

void TestQuantizerNoise() {
  // Plot result with:
  // import numpy
  // import pylab
  //
  // data = numpy.loadtxt('marbles_quantizer_hysteresis.txt')
  // pylab.plot(data)
  // pylab.show()
  FILE* fp = fopen("marbles_quantizer_hysteresis.txt", "w");
  
  Quantizer q;
  Scale scale;
  scale.InitTenth();
  q.Init(scale);
  
  for (int j = 0; j <= 4000; ++j) {
    float noise = (rand() % 500) / 250.0f - 1.0f;
    float tri = j / 2000.0f;
    if (tri >= 1.0f) tri = 2.0f - tri;
    float value = 1.0f * tri + noise * 1.0f / 60.0f;
    float result = q.Process(value, 0.18f, true);
    fprintf(fp, "%f %f\n", value, result);
  }
  fclose(fp);
}

void TestRampExtractorClockBug() {
  WavWriter wav_writer(2, ::kSampleRate, 20);
  wav_writer.Open("marbles_ramp_extractor_clock_bug.wav");
  
  RandomGenerator random_generator;
  RandomStream random_stream;
  random_generator.Init(33);
  random_stream.Init(&random_generator);
  
  PulseGenerator pulse_generator;
  for (size_t i = 0; i < 700; ++i) {
    int t = (rand() % 8) + 796;
    pulse_generator.AddPulses(t, t >> 1, 1);
  }
  
  TGenerator generator;
  generator.Init(&random_stream, kSampleRate);
  generator.set_model(T_GENERATOR_MODEL_COMPLEMENTARY_BERNOULLI);
  generator.set_rate(0.0f);
  generator.set_pulse_width_mean(0.5f);
  generator.set_pulse_width_std(0.0f);
  generator.set_bias(0.5f);
  generator.set_jitter(0.0f);
  generator.set_deja_vu(0.0f);
  generator.set_length(8);
  generator.set_range(T_GENERATOR_RANGE_1X);
  
  MasterSlaveRampGenerator ms_ramp_generator;
  Ramps ramps = ms_ramp_generator.ramps();
  
  float phase = 0.0f;
  for (size_t i = 0; i < ::kSampleRate * 10; i += kAudioBlockSize) {
    phase += 0.0001f;
    if (phase >= 1.0f) {
      phase -= 1.0f;
    }
    float tri = phase < 0.5f ? 2.0f * phase : 2.0f - 2.0f * phase;
    bool gate[kAudioBlockSize * 2];
    GateFlags clock[kAudioBlockSize];
    pulse_generator.Render(clock, kAudioBlockSize);
    generator.set_rate(tri * 24.0f - 12.0f);
    generator.Process(
        true,
        clock, 
        ramps,
        gate,
        kAudioBlockSize);
    for (size_t j = 0; j < kAudioBlockSize; ++j) {
      float s[6];
      s[0] = clock[j] ? 1.0f : 0.0f;
      s[1] = ramps.external[j];
      wav_writer.Write(s, 2, 32767.0f);
    }
  }
}

void TestRampExtractorPause() {
  WavWriter wav_writer(6, ::kSampleRate, 10);
  wav_writer.Open("marbles_ramp_pause.wav");
  
  RandomGenerator random_generator;
  RandomStream random_stream;
  random_generator.Init(33);
  random_stream.Init(&random_generator);
  
  TGenerator generator;
  generator.Init(&random_stream, kSampleRate);
  generator.set_model(T_GENERATOR_MODEL_COMPLEMENTARY_BERNOULLI);
  generator.set_rate(0.0f);
  generator.set_pulse_width_mean(0.0f);
  generator.set_pulse_width_std(0.0f);
  generator.set_bias(0.9f);
  generator.set_jitter(0.0f);
  generator.set_deja_vu(0.0f);
  generator.set_length(8);
  generator.set_range(T_GENERATOR_RANGE_0_25X);
  
  ClockGeneratorPatterns patterns(PAUSE_PATTERNS);
  MasterSlaveRampGenerator ms_ramp_generator;
  Ramps ramps = ms_ramp_generator.ramps();
  
  for (size_t i = 0; i < ::kSampleRate * 10; i += kAudioBlockSize) {
    bool gate[kAudioBlockSize * 2];
    patterns.Render(kAudioBlockSize);
    generator.Process(
        true,
        patterns.clock(), 
        ramps,
        gate,
        kAudioBlockSize);
    for (size_t j = 0; j < kAudioBlockSize; ++j) {
      float s[6];
      s[5] = s[0] = patterns.clock()[j] & GATE_FLAG_HIGH ? 0.8f : 0.0f;
      s[1] = ramps.external[j];
      s[2] = ramps.master[j];
      s[3] = ramps.slave[0][j];
      s[4] = ramps.slave[1][j];
      wav_writer.Write(s, 6, 32767.0f);
    }
  }
}

void TestRampDivider(PatternDifficulty difficulty, const char* file_name) {
  WavWriter wav_writer(4, ::kSampleRate, 10);
  wav_writer.Open(file_name);
  
  ClockGeneratorPatterns patterns(difficulty);
  RampExtractor ramp_extractor;
  RampDivider ramp_divider;
  RampDivider ramp_divider_double;
  RampDivider ramp_divider_half;
  ramp_extractor.Init(0.25f);
  ramp_divider.Init();
  ramp_divider_double.Init();
  ramp_divider_half.Init();
  
  RampChecker ramp_checker[4];
  
  Ratio r8x;
  r8x.p = 8;
  r8x.q = 1;
  
  Ratio r2x;
  r2x.p = 2;
  r2x.q = 1;
  
  Ratio half;
  half.p = 1;
  half.q = 2;
  
  Ratio one;
  one.p = 1;
  one.q = 1;
  
  for (size_t i = 0; i < ::kSampleRate * 10; i += kAudioBlockSize) {
    float ramp[kAudioBlockSize];
    float divided_ramp[kAudioBlockSize];
    float divided_ramp_double[kAudioBlockSize];
    float divided_ramp_half[kAudioBlockSize];
    // switch ((i / (kSampleRate / 4)) % 3) {
    //   case 0:
    //     ramp_divider.set_ratio(1, 2);
    //     break;
    //   case 1:
    //     ramp_divider.set_ratio(4, 1);
    //     break;
    //   case 2:
    //     ramp_divider.set_ratio(3, 2);
    //     break;
    // }
    patterns.Render(kAudioBlockSize);
    
    ramp_extractor.Process(one, true, patterns.clock(), ramp, kAudioBlockSize);
    ramp_divider.Process(r8x, ramp, divided_ramp, kAudioBlockSize);
    ramp_divider_double.Process(r2x, divided_ramp, divided_ramp_double, kAudioBlockSize);
    ramp_divider_half.Process(half, divided_ramp, divided_ramp_half, kAudioBlockSize);
    for (size_t j = 0; j < kAudioBlockSize; ++j) {
      float s[4];
      s[0] = ramp[j];
      s[1] = divided_ramp[j];
      s[2] = divided_ramp_double[j];
      s[3] = divided_ramp_half[j];
      wav_writer.Write(s, 4, 32767.0f);
    }
    ramp_checker[0].Check(ramp, kAudioBlockSize);
    ramp_checker[1].Check(divided_ramp, kAudioBlockSize);
    ramp_checker[2].Check(divided_ramp_double, kAudioBlockSize);
    ramp_checker[3].Check(divided_ramp_half, kAudioBlockSize);
  }
}

void TestOutputChannel() {
  WavWriter wav_writer(2, ::kSampleRate, 3);
  wav_writer.Open("marbles_random_voltage.wav");

  RampExtractor ramp_extractor;
  RandomGenerator random_generator;
  RandomStream random_stream;
  RandomSequence random_sequence;
  OutputChannel output_channel;
  PulseGenerator pulse_generator;

  ramp_extractor.Init(0.25f);
  random_generator.Init(32);
  random_stream.Init(&random_generator);
  random_sequence.Init(&random_stream);
  output_channel.Init();
  
  // Steady clock
  pulse_generator.AddPulses(400, 10, 250);
  pulse_generator.AddPulses(200, 10, 250);
  
  output_channel.set_register_mode(false);
  output_channel.set_steps(0.5f);
  output_channel.set_bias(0.5f);
  
  Ratio one;
  one.p = 1;
  one.q = 1;
  
  for (size_t i = 0; i < ::kSampleRate * 3; i += kAudioBlockSize) {
    GateFlags gate_flags[kAudioBlockSize];
    float ramp[kAudioBlockSize];
    float voltage[kAudioBlockSize];
    pulse_generator.Render(gate_flags, kAudioBlockSize);
    ramp_extractor.Process(one, false, gate_flags, ramp, kAudioBlockSize);
    output_channel.set_spread(wav_writer.triangle(3));
    output_channel.Process(&random_sequence, ramp, voltage, kAudioBlockSize, 1);
    for (size_t j = 0; j < kAudioBlockSize; ++j) voltage[j] *= 0.1f;
    wav_writer.Write(ramp, voltage, kAudioBlockSize);
  }
}

void TestXYGenerator() {
  WavWriter wav_writer(4, ::kSampleRate, 10);
  wav_writer.Open("marbles_xy.wav");
  
  RandomGenerator random_generator;
  RandomStream random_stream;
  random_generator.Init(32);
  random_stream.Init(&random_generator);
  
  XYGenerator generator;
  generator.Init(&random_stream, ::kSampleRate);
  
  GroupSettings x_settings, y_settings;

  x_settings.control_mode = CONTROL_MODE_IDENTICAL;
  x_settings.voltage_range = VOLTAGE_RANGE_FULL;
  x_settings.register_mode = false;
  x_settings.register_value = 0.0f;
  x_settings.spread = 0.8f;
  x_settings.bias = 0.8f;
  x_settings.steps = 0.3f;
  x_settings.deja_vu = 0.0f;
  x_settings.length = 8;
  x_settings.ratio.p = 1;
  x_settings.ratio.q = 1;

  y_settings.control_mode = CONTROL_MODE_IDENTICAL;
  y_settings.voltage_range = VOLTAGE_RANGE_FULL;
  y_settings.register_mode = false;
  y_settings.register_value = 0.0f;
  y_settings.spread = 0.5f;
  y_settings.bias = 0.5f;
  y_settings.steps = 0.1f;
  y_settings.deja_vu = 0.0f;
  y_settings.length = 8;
  y_settings.ratio.p = 1;
  y_settings.ratio.q = 8;
  
  ClockGeneratorPatterns patterns(TRICKY_PATTERNS);
  MasterSlaveRampGenerator ms_ramp_generator;
  
  for (size_t i = 0; i < ::kSampleRate * 10; i += kAudioBlockSize) {
    patterns.Render(kAudioBlockSize);
    ms_ramp_generator.Process(patterns.clock(), kAudioBlockSize);
    
    float samples[kAudioBlockSize * 4];
    generator.Process(
        CLOCK_SOURCE_INTERNAL_T1_T2_T3,
        x_settings,
        y_settings,
        patterns.clock(),
        ms_ramp_generator.ramps(),
        samples,
        kAudioBlockSize);
    wav_writer.Write(samples, kAudioBlockSize * 4, 3276.7f);
  }
}

void TestXYGeneratorASR() {
  WavWriter wav_writer(4, ::kSampleRate, 10);
  wav_writer.Open("marbles_xy_asr.wav");
  
  RandomGenerator random_generator;
  RandomStream random_stream;
  random_generator.Init(32);
  random_stream.Init(&random_generator);
  
  XYGenerator generator;
  generator.Init(&random_stream, ::kSampleRate);
  
  GroupSettings x_settings, y_settings;

  x_settings.control_mode = CONTROL_MODE_IDENTICAL;
  x_settings.voltage_range = VOLTAGE_RANGE_FULL;
  x_settings.register_mode = false;
  x_settings.register_value = 0.0f;
  x_settings.spread = 0.2f;
  x_settings.bias = 0.7f;
  x_settings.steps = 0.5f;
  x_settings.deja_vu = 0.0f;
  x_settings.length = 8;
  x_settings.ratio.p = 1;
  x_settings.ratio.q = 1;

  y_settings.control_mode = CONTROL_MODE_IDENTICAL;
  y_settings.voltage_range = VOLTAGE_RANGE_FULL;
  y_settings.register_mode = false;
  y_settings.register_value = 0.0f;
  y_settings.spread = 0.5f;
  y_settings.bias = 0.5f;
  y_settings.steps = 0.1f;
  y_settings.deja_vu = 0.0f;
  y_settings.length = 8;
  y_settings.ratio.p = 1;
  y_settings.ratio.q = 8;
  
  ClockGeneratorPatterns patterns(TRICKY_PATTERNS);
  MasterSlaveRampGenerator ms_ramp_generator;
  
  for (size_t i = 0; i < ::kSampleRate * 10; i += kAudioBlockSize) {
    patterns.Render(kAudioBlockSize);
    ms_ramp_generator.Process(patterns.clock(), kAudioBlockSize);
    
    float samples[kAudioBlockSize * 4];
    
    x_settings.register_mode = true;
    x_settings.register_value = wav_writer.triangle(1);
    
    generator.Process(
        CLOCK_SOURCE_EXTERNAL,
        x_settings,
        y_settings,
        patterns.clock(),
        ms_ramp_generator.ramps(),
        samples,
        kAudioBlockSize);
    wav_writer.Write(samples, kAudioBlockSize * 4, 3276.7f);
  }
}

void TestTGenerator() {
  WavWriter wav_writer(6, ::kSampleRate, 10);
  wav_writer.Open("marbles_t.wav");
  
  RandomGenerator random_generator;
  RandomStream random_stream;
  random_generator.Init(33);
  random_stream.Init(&random_generator);
  
  TGenerator generator;
  generator.Init(&random_stream, kSampleRate);
  generator.set_model(T_GENERATOR_MODEL_COMPLEMENTARY_BERNOULLI);
  generator.set_rate(24.0f);
  generator.set_pulse_width_mean(0.0f);
  generator.set_pulse_width_std(0.0f);
  generator.set_bias(0.9f);
  generator.set_jitter(0.5f);
  generator.set_deja_vu(0.0f);
  generator.set_length(8);
  generator.set_range(T_GENERATOR_RANGE_4X);
  
  ClockGeneratorPatterns patterns(FRIENDLY_PATTERNS);
  MasterSlaveRampGenerator ms_ramp_generator;
  Ramps ramps = ms_ramp_generator.ramps();
  
  RampChecker ramp_checker[4];
  for (size_t i = 0; i < ::kSampleRate * 10; i += kAudioBlockSize) {
    bool gate[kAudioBlockSize * 2];
    patterns.Render(kAudioBlockSize);
    generator.Process(
        true,
        patterns.clock(), 
        ramps,
        gate,
        kAudioBlockSize);
    if (i >= kSampleRate * 5) {
      // generator.set_deja_vu(1.0f);
      // generator.set_jitter(0.5f);
    }
    for (size_t j = 0; j < kAudioBlockSize; ++j) {
      float s[6];
      s[0] = ramps.external[j];
      s[1] = ramps.master[j];
      s[2] = ramps.slave[0][j];
      s[3] = ramps.slave[1][j];
      s[4] = gate[j * 2] ? 1.0f : 0.0f;
      s[5] = gate[j * 2 + 1] ? 1.0f : 0.0f;
      wav_writer.Write(s, 6, 32767.0f);
    }
    ramp_checker[0].Check(ramps.external, kAudioBlockSize);
    ramp_checker[1].Check(ramps.master, kAudioBlockSize);
    ramp_checker[2].Check(ramps.slave[0], kAudioBlockSize);
    ramp_checker[3].Check(ramps.slave[1], kAudioBlockSize);
  }
}

void TestTGeneratorSuperFastClock() {
  WavWriter wav_writer(6, ::kSampleRate, 10);
  wav_writer.Open("marbles_t_super_fast.wav");
  
  RandomGenerator random_generator;
  RandomStream random_stream;
  random_generator.Init(33);
  random_stream.Init(&random_generator);
  
  TGenerator generator;
  generator.Init(&random_stream, kSampleRate);
  generator.set_model(T_GENERATOR_MODEL_COMPLEMENTARY_BERNOULLI);
  generator.set_rate(60.0f);
  generator.set_pulse_width_mean(0.0f);
  generator.set_pulse_width_std(0.0f);
  generator.set_bias(0.5f);
  generator.set_jitter(0.0f);
  generator.set_deja_vu(0.0f);
  generator.set_length(8);
  generator.set_range(T_GENERATOR_RANGE_4X);
  
  ClockGeneratorPatterns patterns(FAST_PATTERNS);
  MasterSlaveRampGenerator ms_ramp_generator;
  Ramps ramps = ms_ramp_generator.ramps();
  
  for (size_t i = 0; i < ::kSampleRate * 10; i += kAudioBlockSize) {
    bool gate[kAudioBlockSize * 2];
    patterns.Render(kAudioBlockSize);
    generator.Process(
        true,
        patterns.clock(), 
        ramps,
        gate,
        kAudioBlockSize);
    for (size_t j = 0; j < kAudioBlockSize; ++j) {
      float s[6];
      s[0] = ramps.external[j];
      s[1] = ramps.master[j];
      s[2] = ramps.slave[0][j];
      s[3] = ramps.slave[1][j];
      s[4] = gate[j * 2] ? 1.0f : 0.0f;
      s[5] = gate[j * 2 + 1] ? 1.0f : 0.0f;
      wav_writer.Write(s, 6, 32767.0f);
    }
  }
}

void TestTGeneratorRampIntegrity(
    bool internal_clock,
    float rate,
    TGeneratorModel model,
    float jitter) {
  printf("Testing ramp integrity for intclock = %d\trate = %04.3f\tmodel = %d\tjitter = %04.3f\n", internal_clock, rate, model, jitter);
  
  RandomGenerator random_generator;
  RandomStream random_stream;
  random_generator.Init(33);
  random_stream.Init(&random_generator);

  TGenerator generator;
  generator.Init(&random_stream, kSampleRate);
  generator.set_model(model);
  generator.set_rate(rate + 36.0f);
  generator.set_pulse_width_mean(0.0f);
  generator.set_pulse_width_std(0.0f);
  generator.set_bias(0.7f);
  generator.set_jitter(jitter);
  if (internal_clock) {
    generator.set_range(T_GENERATOR_RANGE_4X);
  } else {
    generator.set_range(T_GENERATOR_RANGE_1X);
  }

  ClockGeneratorPatterns patterns(FRIENDLY_PATTERNS);
  MasterSlaveRampGenerator ms_ramp_generator;
  Ramps ramps = ms_ramp_generator.ramps();

  RampChecker ramp_checker[4];
  for (size_t i = 0; i < ::kSampleRate * 10; i += kAudioBlockSize) {
    bool gate[kAudioBlockSize * 2];
    patterns.Render(kAudioBlockSize);
    generator.Process(
        !internal_clock,
        patterns.clock(), 
        ramps,
        gate,
        kAudioBlockSize);
    ramp_checker[0].Check(ramps.external, kAudioBlockSize);
    ramp_checker[1].Check(ramps.master, kAudioBlockSize);
    ramp_checker[2].Check(ramps.slave[0], kAudioBlockSize);
    ramp_checker[3].Check(ramps.slave[1], kAudioBlockSize);
  }
}


void TestTGeneratorRampIntegrity() {
  for (size_t clock_source = 0; clock_source < 2; ++clock_source) {
    for (size_t model = 0; model < 6; ++model) {
      for (size_t jitter = 0; jitter < 5; ++jitter) {
        for (size_t rate = 0; rate < 5; ++rate) {
          TestTGeneratorRampIntegrity(
              clock_source,
              float(rate) * 12.0f - 24.0f,
              TGeneratorModel(model),
              float(jitter) * 0.2f);
        }
      }
    }
  }
}

void TestScaleRecorder() {
  int prelude[] = { 0, 4, 7, 7, 12, 12, 16, 16, 7, 7, 12, 12, 16, 0, 0, 4, 16,
    4, 7, 7, 12, 12, 16, 16, 7, 7, 12, 12, 16, 0, 0, 4, 16, 2, 9, 9, 14, 14,
    17, 17, 9, 9, 14, 14, 17, 0, 0, 2, 17, 2, 9, 9, 14, 14, 17, 17, 9, 9, 14,
    14, 17, -1, 0, 2, 17, 2, 7, 7, 14, 14, 17, 17, 7, 7, 14, 14, 17, -1, -1,
    2, 17, 2, 7, 7, 14, 14, 17, 17, 7, 7, 14, 14, 17, -1, 0, 2, 17, 4, 7, 7,
    12, 12, 16, 16, 7, 7, 12, 12, 16, 0, 0, 4, 16, 4, 7, 7, 12, 12, 16, 16, 7,
    7, 12, 12, 16, 0, 0, 4, 16, 4, 9, 9, 16, 16, 21, 21, 9, 9, 16, 16, 21, 0,
    0, 4, 21, 4, 9, 9, 16, 16, 21, 21, 9, 9, 16, 16, 21, 0, 0, 4, 21, 2, 6, 6,
    9, 9, 14, 14, 6, 6, 9, 9, 14, 0, 0, 2, 14, 2, 6, 6, 9, 9, 14, 14, 6, 6, 9,
    9, 14, -1, 0, 2, 14, 2, 7, 7, 14, 14, 19, 19, 7, 7, 14, 14, 19, -1, -1, 2,
    19, 2, 7, 7, 14, 14, 19, 19, 7, 7, 14, 14, 19, -1, -1, 2, 19, 0, 4, 4, 7,
    7, 12, 12, 4, 4, 7, 7, 12, -1, -1, 0, 12, 0, 4, 4, 7, 7, 12, 12, 4, 4, 7,
    7, 12, -3, -1, 0, 12, 0, 4, 4, 7, 7, 12, 12, 4, 4, 7, 7, 12, -3, -3, 0, 12,
    0, 4, 4, 7, 7, 12, 12, 4, 4, 7, 7, 12, -10, -3, 0, 12, -3, 2, 2, 6, 6, 12,
    12, 2, 2, 6, 6, 12, -10, -10, -3, 12, -3, 2, 2, 6, 6, 12, 12, 2, 2, 6, 6,
    12, -10, -5, -3, 12, -1, 2, 2, 7, 7, 11, 11, 2, 2, 7, 7, 11, -5, -5, -1,
    11, -1, 2, 2, 7, 7, 11, 11, 2, 2, 7, 7, 11, -5, -5, -1, 11, -2, 4, 4, 7, 7,
    13, 13, 4, 4, 7, 7, 13, -5, -5, -2, 13, -2, 4, 4, 7, 7, 13, 13, 4, 4, 7, 7,
    13, -7, -5, -2, 13, -3, 2, 2, 9, 9, 14, 14, 2, 2, 9, 9, 14, -7, -7, -3, 14,
    -3, 2, 2, 9, 9, 14, 14, 2, 2, 9, 9, 14, -7, -7, -3, 14, -4, 2, 2, 5, 5, 11,
    11, 2, 2, 5, 5, 11, -7, -7, -4, 11, -4, 2, 2, 5, 5, 11, 11, 2, 2, 5, 5, 11,
    -8, -7, -4, 11, -5, 0, 0, 7, 7, 12, 12, 0, 0, 7, 7, 12, -8, -8, -5, 12, -5,
    0, 0, 7, 7, 12, 12, 0, 0, 7, 7, 12, -8, -8, -5, 12, -7, -3, -3, 0, 0, 5, 5,
    -3, -3, 0, 0, 5, -8, -8, -7, 5, -7, -3, -3, 0, 0, 5, 5, -3, -3, 0, 0, 5,
    -10, -8, -7, 5, -7, -3, -3, 0, 0, 5, 5, -3, -3, 0, 0, 5, -10, -10, -7, 5,
    -7, -3, -3, 0, 0, 5, 5, -3, -3, 0, 0, 5, -17, -10, -7, 5, -10, -5, -5, -1,
    -1, 5, 5, -5, -5, -1, -1, 5, -17, -17, -10, 5, -10, -5, -5, -1, -1, 5, 5,
    -5, -5, -1, -1, 5, -17, -12, -10, 5, -8, -5, -5, 0, 0, 4, 4, -5, -5, 0, 0,
    4, -12, -12, -8, 4, -8, -5, -5, 0, 0, 4, 4, -5, -5, 0, 0, 4, -12, -12, -8,
    4, -5, -2, -2, 0, 0, 4, 4, -2, -2, 0, 0, 4, -12, -12, -5, 4, -5, -2, -2, 0,
    0, 4, 4, -2, -2, 0, 0, 4, -19, -12, -5, 4, -7, -3, -3, 0, 0, 4, 4, -3, -3,
    0, 0, 4, -19, -19, -7, 4, -7, -3, -3, 0, 0, 4, 4, -3, -3, 0, 0, 4, -19,
    -18, -7, 4, -12, -3, -3, 0, 0, 3, 3, -3, -3, 0, 0, 3, -18, -18, -12, 3,
    -12, -3, -3, 0, 0, 3, 3, -3, -3, 0, 0, 3, -18, -16, -12, 3, -7, -1, -1, 0,
    0, 2, 2, -1, -1, 0, 0, 2, -16, -16, -7, 2, -7, -1, -1, 0, 0, 2, 2, -1, -1,
    0, 0, 2, -17, -16, -7, 2, -7, -5, -5, -1, -1, 2, 2, -5, -5, -1, -1, 2, -17,
    -17, -7, 2, -7, -5, -5, -1, -1, 2, 2, -5, -5, -1, -1, 2, -17, -17, -7,
    2, -8, -5, -5, 0, 0, 4, 4, -5, -5, 0, 0, 4, -17, -17, -8, 4, -8, -5, -5, 0,
    0, 4, 4, -5, -5, 0, 0, 4, -17, -17, -8, 4, -10, -5, -5, 0, 0, 5, 5, -5, -5,
    0, 0, 5, -17, -17, -10, 5, -10, -5, -5, 0, 0, 5, 5, -5, -5, 0, 0, 5, -17,
    -17, -10, 5,-10, -5, -5, -1, -1, 5, 5, -5, -5, -1, -1, 5, -17, -17, -10, 5,
    -10, -5, -5,-1, -1, 5, 5, -5, -5, -1, -1, 5, -17, -17, -10, 5, -9, -3, -3,
    0, 0, 6, 6, -3,-3, 0, 0, 6, -17, -17, -9, 6, -9, -3, -3, 0, 0, 6, 6, -3,
    -3, 0, 0, 6, -17,-17, -9, 6, -8, -5, -5, 0, 0, 7, 7, -5, -5, 0, 0, 7, -17,
    -17, -8, 7, -8, -5,-5, 0, 0, 7, 7, -5, -5, 0, 0, 7, -17, -17, -8, 7, -10,
    -5, -5, 0, 0, 5, 5, -5,-5, 0, 0, 5, -17, -17, -10, 5, -10, -5, -5, 0, 0, 5,
    5, -5, -5, 0, 0, 5, -17,-17, -10, 5, -10, -5, -5, -1, -1, 5, 5, -5, -5, -1,
    -1, 5, -17, -17, -10, 5,-10, -5, -5, -1, -1, 5, 5, -5, -5, -1, -1, 5, -24,
    -17, -10, 5, -12, -5, -5,-2, -2, 4, 4, -5, -5, -2, -2, 4, -24, -24, -12, 4,
    -12, -5, -5, -2, -2, 4, 4,-5, -5, -2, -2, 4, -24, -24, -12, 4, -12, -7, -7,
    -3, -3, 0, 0, 5, 0, 5, -3, 0,-3, 0, -3, 0, -7, -3, -7, -3, -7, -3, -10, -7,
    -10, -7, -10, -7, -12, -10, -24,-24, -10, 7, 7, 11, 11, 14, 14, 17, 14, 17,
    11, 14, 11, 14, 11, 14, 7, 11, 7,11, 2, 11, 2, 5, 4, 5, 2, 4, -10, 2, -24,
    -24, -12, 4, 7, 12, -24, -12, 4, 7, 12
  };
  
  ScaleRecorder recorder;
  recorder.Init();
  for (size_t i = 0; i < sizeof(prelude) / sizeof(int); ++i) {
    float voltage = prelude[i] / 12.0f + Random::GetFloat() * 0.0001f;
    recorder.NewNote(voltage);
    recorder.UpdateVoltage(voltage);
    recorder.AcceptNote();
  }
  
  Scale s;
  recorder.ExtractScale(&s);
  
  for (int i = 0; i < s.num_degrees; ++i) {
    printf("%f %d\n", s.degree[i].voltage, s.degree[i].weight);
  }
}

int main(void) {
  // Test distributions and value processors.
  // TestBetaDistribution();
  // TestQuantizer();
  // TestQuantizerNoise();

  // Ramp tests.
  // TestRampExtractor(FRIENDLY_PATTERNS, "marbles_ramp_extractor_friendly.wav");
  // TestRampExtractor(TRICKY_PATTERNS, "marbles_ramp_extractor_tricky.wav");
  // TestRampExtractor(FAST_PATTERNS, "marbles_ramp_extractor_fast.wav");
  // TestRampDivider(TRICKY_PATTERNS, "marbles_ramp_divider_tricky.wav");

  // TestRampExtractorClockBug();
  // TestRampExtractorPause();

  // TestOutputChannel();
  // TestXYGenerator();
  // TestXYGeneratorASR();
  // TestTGeneratorRampIntegrity();
  TestTGenerator();
  
  // TestScaleRecorder();
}
