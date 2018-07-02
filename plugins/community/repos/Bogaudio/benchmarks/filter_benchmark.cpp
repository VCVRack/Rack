
#include <benchmark/benchmark.h>

#include "dsp/noise.hpp"
#include "dsp/filter.hpp"
#include "dsp/decimator.hpp" // rack

using namespace bogaudio::dsp;

static void BM_LPFDecimator(benchmark::State& state) {
  WhiteNoiseGenerator r;
  const int n = 8;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = r.next();
  }
  LPFDecimator d(44100.0, n);
  for (auto _ : state) {
    benchmark::DoNotOptimize(d.next(buf));
  }
}
BENCHMARK(BM_LPFDecimator);

static void BM_CICDecimator(benchmark::State& state) {
  WhiteNoiseGenerator r;
  const int n = 8;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = r.next();
  }
  CICDecimator d(4, 8);
  for (auto _ : state) {
    benchmark::DoNotOptimize(d.next(buf));
  }
}
BENCHMARK(BM_CICDecimator);

static void BM_RackDecimator(benchmark::State& state) {
  WhiteNoiseGenerator r;
  const int n = 8;
  float buf[n];
  for (int i = 0; i < n; ++i) {
    buf[i] = r.next();
  }
  rack::Decimator<n, n> d;
  for (auto _ : state) {
    benchmark::DoNotOptimize(d.process(buf));
  }
}
BENCHMARK(BM_RackDecimator);
