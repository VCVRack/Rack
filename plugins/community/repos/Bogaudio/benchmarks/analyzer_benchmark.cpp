
#include <benchmark/benchmark.h>

#include "dsp/analyzer.hpp"

using namespace bogaudio::dsp;

static void BM_HanningWindowInit(benchmark::State& state) {
  for (auto _ : state) {
    delete new HanningWindow(1024);
  }
}
BENCHMARK(BM_HanningWindowInit);

static void BM_HanningWindowApply(benchmark::State& state) {
  const int n = 1024;
  HanningWindow w(n);
  float in[n];
  std::fill_n(in, n, 1.1);
  float out[n] {};
  for (auto _ : state) {
    w.apply(in, out);
    in[0] = out[0]; // prevents the call from being optimized away?
  }
}
BENCHMARK(BM_HanningWindowApply);

static void BM_RuntimeFFT1024(benchmark::State& state) {
  const int n = 1024;
  ffft::FFTReal<float> fft(n);
  float in[n];
  std::fill_n(in, n, 1.1);
  float out[n] {};
  for (auto _ : state) {
    fft.do_fft(out, in);
  }
}
BENCHMARK(BM_RuntimeFFT1024);

static void BM_CompileTimeFFT1024(benchmark::State& state) {
  FFT1024 fft;
  const int n = 1024;
  float in[n];
  std::fill_n(in, n, 1.1);
  float out[n] {};
  for (auto _ : state) {
    fft.do_fft(out, in);
  }
}
BENCHMARK(BM_CompileTimeFFT1024);

static void BM_RuntimeFFT4096(benchmark::State& state) {
  const int n = 4096;
  ffft::FFTReal<float> fft(n);
  float in[n];
  std::fill_n(in, n, 1.1);
  float out[n] {};
  for (auto _ : state) {
    fft.do_fft(out, in);
  }
}
BENCHMARK(BM_RuntimeFFT4096);

static void BM_CompileTimeFFT4096(benchmark::State& state) {
  FFT4096 fft;
  const int n = 4096;
  float in[n];
  std::fill_n(in, n, 1.1);
  float out[n] {};
  for (auto _ : state) {
    fft.do_fft(out, in);
  }
}
BENCHMARK(BM_CompileTimeFFT4096);

static void BM_SpectrumAnalyzerStep(benchmark::State& state) {
  SpectrumAnalyzer sa(
    SpectrumAnalyzer::SIZE_1024,
    SpectrumAnalyzer::OVERLAP_1,
    SpectrumAnalyzer::WINDOW_HANNING,
    44100.0
  );
  float in[8] = { 0.0, 0.7, 1.0, 0.7, 0.0, -0.7, -1.0, -0.7 };
  int i = 0;
  for (auto _ : state) {
    sa.step(in[i]);
    ++i;
    i %= 8;
  }
}
BENCHMARK(BM_SpectrumAnalyzerStep);

static void BM_SpectrumAnalyzerProcess(benchmark::State& state) {
  SpectrumAnalyzer sa(
    SpectrumAnalyzer::SIZE_1024,
    SpectrumAnalyzer::OVERLAP_1,
    SpectrumAnalyzer::WINDOW_HANNING,
    44100.0
  );
  float in[8] = { 0.0, 0.7, 1.0, 0.7, 0.0, -0.7, -1.0, -0.7 };
  for (int i = 0; i < 1024; ++i) {
    sa.step(in[i % 8]);
  }

  for (auto _ : state) {
    sa.process(sa._samples);
  }
}
BENCHMARK(BM_SpectrumAnalyzerProcess);

static void BM_SpectrumAnalyzerGetMagnitudes(benchmark::State& state) {
  SpectrumAnalyzer sa(
    SpectrumAnalyzer::SIZE_1024,
    SpectrumAnalyzer::OVERLAP_1,
    SpectrumAnalyzer::WINDOW_HANNING,
    44100.0
  );
  const int nBins = 256;
  float bins[nBins];
  float in[8] = { 0.0, 0.7, 1.0, 0.7, 0.0, -0.7, -1.0, -0.7 };
  for (int i = 0; i < 1024; ++i) {
    sa.step(in[i % 8]);
  }

  for (auto _ : state) {
    sa.getMagnitudes(bins, nBins);
  }
}
BENCHMARK(BM_SpectrumAnalyzerGetMagnitudes);
