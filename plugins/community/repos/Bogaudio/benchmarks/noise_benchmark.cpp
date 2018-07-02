
#include <benchmark/benchmark.h>

#include "dsp/noise.hpp"

using namespace bogaudio::dsp;

static void BM_WhiteNoise(benchmark::State& state) {
  WhiteNoiseGenerator g;
  for (auto _ : state) {
    g.next();
  }
}
BENCHMARK(BM_WhiteNoise);

static void BM_PinkNoise(benchmark::State& state) {
  PinkNoiseGenerator g;
  for (auto _ : state) {
    g.next();
  }
}
BENCHMARK(BM_PinkNoise);

static void BM_RedNoise(benchmark::State& state) {
  RedNoiseGenerator g;
  for (auto _ : state) {
    g.next();
  }
}
BENCHMARK(BM_RedNoise);

static void BM_GaussianNoise(benchmark::State& state) {
  GaussianNoiseGenerator g;
  for (auto _ : state) {
    g.next();
  }
}
BENCHMARK(BM_GaussianNoise);
