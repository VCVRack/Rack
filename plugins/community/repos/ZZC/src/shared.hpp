
#ifndef ZZC_SHARED_H
#define ZZC_SHARED_H

#include "dsp/digital.hpp"
#include "util/math.hpp"
#include "ZZC.hpp"

using namespace rack;

namespace rack_plugin_ZZC {

struct LowFrequencyOscillator {
  float phase = 0.0f;
  float lastPhase = 0.0f;
  float freq = 1.0f;

  struct NormalizationResult {
    float value;
    bool normalized;
  };

  LowFrequencyOscillator() {}

  void setPitch(float pitch) {
    freq = pitch;
  }

  bool reset(float value) {
    NormalizationResult result = normalize(value);
    phase = result.value;
    return result.normalized;
  }

  NormalizationResult normalize(float value) {
    float output = eucmod(value, 1.0f);
    return NormalizationResult { output, output != value };
  }

  bool step(float dt) {
    float deltaPhase = freq * dt;
    NormalizationResult result = normalize(phase + deltaPhase);
    lastPhase = phase;
    phase = result.value;
    return result.normalized;
  }
};

struct ClockTracker {
  int triggersPassed;
  float period;
  float freq;
  bool freqDetected;

  SchmittTrigger clockTrigger;

  void init() {
    triggersPassed = 0;
    period = 0.0f;
    freq = 0.0f;
    freqDetected = false;
  }

  void process(float dt, float clock) {
    period += dt;
    if (clockTrigger.process(clock)) {
      if (triggersPassed < 3) {
        triggersPassed += 1;
      }
      if (triggersPassed > 2) {
        freqDetected = true;
        freq = 1.0f / period;
      }
      period = 0.0f;
    }
  }
};

} // namespace rack_plugin_ZZC

#endif // ZZC_SHARED_H
