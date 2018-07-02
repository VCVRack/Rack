
#include <benchmark/benchmark.h>

#include "dsp/oscillator.hpp"

using namespace bogaudio::dsp;

static void BM_Phasor(benchmark::State& state) {
  Phasor p(44100.0, 440.0);
  for (auto _ : state) {
    p.next();
  }
}
BENCHMARK(BM_Phasor);

static void BM_SineOscillator(benchmark::State& state) {
  SineOscillator o(44100.0, 440.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SineOscillator);

static void BM_SineTableOscillator(benchmark::State& state) {
  SineTableOscillator o(44100.0, 440.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SineTableOscillator);

static void BM_SineOscillatorFM(benchmark::State& state) {
  const float baseHz = 440.0;
  SineOscillator m(44100.0, baseHz);
  SineOscillator c(44100.0, baseHz);
  for (auto _ : state) {
    c.setFrequency(baseHz + m.next());
    c.next();
  }
}
BENCHMARK(BM_SineOscillatorFM);

static void BM_SineTableOscillatorPM(benchmark::State& state) {
  SineTableOscillator m(44100.0, 440.0);
  SineTableOscillator c(44100.0, 440.0);
  for (auto _ : state) {
    c.advancePhase();
    c.nextFromPhasor(c, Phasor::radiansToPhase(m.next()));
  }
}
BENCHMARK(BM_SineTableOscillatorPM);

static void BM_SawOscillator(benchmark::State& state) {
  SawOscillator o(44100.0, 440.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SawOscillator);

static void BM_SaturatingSawOscillator(benchmark::State& state) {
  SaturatingSawOscillator o(44100.0, 440.0);
  o.setSaturation(0.9);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SaturatingSawOscillator);

static void BM_BandLimitedSawOscillator(benchmark::State& state) {
  BandLimitedSawOscillator o(44100.0, 440.0);
  o.setQuality(12);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_BandLimitedSawOscillator);

static void BM_SquareOscillator(benchmark::State& state) {
  SquareOscillator o(44100.0, 440.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SquareOscillator);

static void BM_BandLimitedSquareOscillator(benchmark::State& state) {
  BandLimitedSquareOscillator o(44100.0, 440.0);
  o.setQuality(12);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_BandLimitedSquareOscillator);

static void BM_TriangleOscillator(benchmark::State& state) {
  TriangleOscillator o(44100.0, 440.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_TriangleOscillator);

static void BM_SampledTriangleOscillator(benchmark::State& state) {
  TriangleOscillator o(44100.0, 440.0);
  o.setSampleWidth(Phasor::maxSampleWidth / 2.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SampledTriangleOscillator);

static void BM_SineBankOscillator100(benchmark::State& state) {
  SineBankOscillator o(44100.0, 100.0, 100);
  for (int i = 1, n = o.partialCount(); i <= n; ++i) {
    o.setPartial(i, i, 1.0 / (float)i);
  }

  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SineBankOscillator100);

static void BM_SineBankOscillator500(benchmark::State& state) {
  SineBankOscillator o(44100.0, 500.0, 100);
  for (int i = 1, n = o.partialCount(); i <= n; ++i) {
    o.setPartial(i, i, 1.0 / (float)i);
  }

  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SineBankOscillator500);

static void BM_SineBankOscillator5000(benchmark::State& state) {
  SineBankOscillator o(44100.0, 5000.0, 100);
  for (int i = 1, n = o.partialCount(); i <= n; ++i) {
    o.setPartial(i, i, 1.0 / (float)i);
  }

  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SineBankOscillator5000);

static void BM_SineBankOscillator15000(benchmark::State& state) {
  SineBankOscillator o(44100.0, 15000.0, 100);
  for (int i = 1, n = o.partialCount(); i <= n; ++i) {
    o.setPartial(i, i, 1.0 / (float)i);
  }

  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SineBankOscillator15000);
