
#include <benchmark/benchmark.h>

#include "dsp/buffer.hpp"

using namespace bogaudio::dsp;

struct BMOverlappingBuffer : OverlappingBuffer<float> {
  BMOverlappingBuffer(int size, int o) : OverlappingBuffer(size, o) {}
  void process(float* samples) override {}
};

static void BM_OverlappingBuffer(benchmark::State& state) {
  BMOverlappingBuffer b(1024, 2);
  int i = 0;
  for (auto _ : state) {
    b.step(i++);
  }
}
BENCHMARK(BM_OverlappingBuffer);


static void _averagingBuffer(benchmark::State& state, int n, int m) {
  AveragingBuffer<float> b(n, m);
  for (int i = 0; i < m; ++i) {
    float* frame = b.getInputFrame();
    std::fill_n(frame, n, M_PI);
    b.commitInputFrame();
  }
  float pi = 0.0;
  for (auto _ : state) {
    b.getInputFrame();
    b.commitInputFrame();
    pi = b.getAverages()[0];
  }
  const float e = 0.00001;
  assert(pi > M_PI - e && pi < M_PI + e);
}

static void BM_AveragingBufferSmallN(benchmark::State& state) {
  _averagingBuffer(state, 1024, 3);
}
BENCHMARK(BM_AveragingBufferSmallN);

static void BM_AveragingBufferLargeN(benchmark::State& state) {
  _averagingBuffer(state, 1024, 100);
}
BENCHMARK(BM_AveragingBufferLargeN);
