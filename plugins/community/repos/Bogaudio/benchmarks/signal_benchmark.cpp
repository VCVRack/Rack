
#include <vector>

#include <benchmark/benchmark.h>

#include "dsp/oscillator.hpp"
#include "dsp/noise.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

static void BM_DecibelsToAmplitude(benchmark::State& state) {
  std::vector<float> buf = { 10.0f, 6.0f, 3.0f, 0.0f, -3.0f, -6.0f, -10.0f, -30.0f, -60.0f };
  int i = 0;
  for (auto _ : state) {
    i = ++i % buf.size();
    benchmark::DoNotOptimize(decibelsToAmplitude(buf.at(i)));
  }
}
BENCHMARK(BM_DecibelsToAmplitude);

static void BM_AmplitudeToDecibels(benchmark::State& state) {
  std::vector<float> buf = { 0.0001f, 0.0001f, 0.001f, 0.01, 0.1f, 0.3f, 0.5f, 0.8f, 1.0f, 1.5f, 2.0f, 5.0f, 10.0f };
  int i = 0;
  for (auto _ : state) {
    i = ++i % buf.size();
    benchmark::DoNotOptimize(amplitudeToDecibels(buf.at(i)));
  }
}
BENCHMARK(BM_AmplitudeToDecibels);

static void BM_Amplifier(benchmark::State& state) {
  WhiteNoiseGenerator r;
  const int n = 256;
  float samples[n];
  for (int i = 0; i < n; ++i) {
    samples[i] = r.next();
  }
  std::vector<float> decibels = { 10.0f, 6.0f, 3.0f, 0.0f, -3.0f, -6.0f, -10.0f, -30.0f, -60.0f };

  Amplifier a;
  int i = 0, j = 0;
  for (auto _ : state) {
    i = ++i % decibels.size();
    j = ++j % n;
    a.setLevel(decibels.at(i));
    benchmark::DoNotOptimize(a.next(samples[j]));
  }
}
BENCHMARK(BM_Amplifier);

static void BM_RMS_Short(benchmark::State& state) {
  SineOscillator o(500.0, 100.0);
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = o.next() * 5.0f;
  }
  RootMeanSquare rms(44100.0, 0.05);
  int i = 0;
  for (auto _ : state) {
    i = ++i % n;
    benchmark::DoNotOptimize(rms.next(buf[i]));
  }
}
BENCHMARK(BM_RMS_Short);

static void BM_RMS_Long(benchmark::State& state) {
  SineOscillator o(500.0, 100.0);
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = o.next() * 5.0f;
  }
  RootMeanSquare rms(44100.0, 1.0);
  int i = 0;
  for (auto _ : state) {
    i = ++i % n;
    benchmark::DoNotOptimize(rms.next(buf[i]));
  }
}
BENCHMARK(BM_RMS_Long);

static void BM_RMS_Modulating(benchmark::State& state) {
  SineOscillator o(500.0, 100.0);
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = o.next() * 5.0f;
  }
	std::minstd_rand g;
	std::uniform_real_distribution<float> r(0.0f, 1.0f);
  RootMeanSquare rms(44100.0, 1.0);
  int i = 0;
  for (auto _ : state) {
    i = ++i % n;
    if (i % 50 == 0) {
      rms.setSensitivity(r(g));
    }
    benchmark::DoNotOptimize(rms.next(buf[i]));
  }
}
BENCHMARK(BM_RMS_Modulating);

static void BM_PucketteEnvelopeFollower(benchmark::State& state) {
  SineOscillator o(500.0, 100.0);
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = o.next() * 5.0f;
  }
  PucketteEnvelopeFollower pef;
  int i = 0;
  for (auto _ : state) {
    i = ++i % n;
    benchmark::DoNotOptimize(pef.next(buf[i]));
  }
}
BENCHMARK(BM_PucketteEnvelopeFollower);

static void BM_SlewLimiter(benchmark::State& state) {
  WhiteNoiseGenerator r;
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = r.next();
  }
  SlewLimiter sl(44100.0, 1.0f);
  int i = 0;
  for (auto _ : state) {
    i = ++i % n;
    benchmark::DoNotOptimize(sl.next(buf[i]));
  }
}
BENCHMARK(BM_SlewLimiter);

static void BM_ShapedSlewLimiter(benchmark::State& state) {
  WhiteNoiseGenerator r;
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = r.next();
  }
  ShapedSlewLimiter sl(44100.0, 1.0f, 0.5f);
  int i = 0;
  for (auto _ : state) {
    i = ++i % n;
    benchmark::DoNotOptimize(sl.next(buf[i]));
  }
}
BENCHMARK(BM_ShapedSlewLimiter);

static void BM_Panner(benchmark::State& state) {
  SineOscillator o(500.0, 100.0);
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = o.next() * 5.0f;
  }
  Panner p;
  int i = 0;
  float l, r;
  for (auto _ : state) {
    i = ++i % n;
    p.next(buf[i], l, r);
  }
}
BENCHMARK(BM_Panner);

static void BM_Panner_Modulating(benchmark::State& state) {
  SineOscillator o(500.0, 100.0);
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = o.next() * 5.0f;
  }
  std::minstd_rand g;
	std::uniform_real_distribution<float> r(-1.0f, 1.0f);
  Panner p;
  int i = 0;
  float l, rr;
  for (auto _ : state) {
    i = ++i % n;
    p.setPan(r(g));
    p.next(buf[i], l, rr);
  }
}
BENCHMARK(BM_Panner_Modulating);

static void BM_DelayLine(benchmark::State& state) {
  SineOscillator o(500.0, 100.0);
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = o.next() * 5.0f;
  }
  DelayLine dl(44100.0, 1000.0, 1.0);
  int i = 0;
  for (auto _ : state) {
    i = ++i % n;
    benchmark::DoNotOptimize(dl.next(buf[i]));
  }
}
BENCHMARK(BM_DelayLine);

static void BM_Saturator(benchmark::State& state) {
  SineOscillator o(500.0, 100.0);
  const int n = 256;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = o.next() * 20.0f;
  }
  Saturator s;
  int i = 0;
  for (auto _ : state) {
    i = ++i % n;
    benchmark::DoNotOptimize(s.next(buf[i]));
  }
}
BENCHMARK(BM_Saturator);

static void BM_CompressorSoftKnee(benchmark::State& state) {
  int i = 0;
  Compressor c;
  for (auto _ : state) {
    i = ++i % 15;
    benchmark::DoNotOptimize(c.compressionDb((float)(i + 5), 10.0f, 2.0f, true));
  }
}
BENCHMARK(BM_CompressorSoftKnee);

static void BM_NoiseGateSoftKnee(benchmark::State& state) {
  int i = 0;
  NoiseGate ng;
  for (auto _ : state) {
    i = ++i % 15;
    benchmark::DoNotOptimize(ng.compressionDb(0.0f - (float)(i * 3), 0.0f, 2.0f, true));
  }
}
BENCHMARK(BM_NoiseGateSoftKnee);
