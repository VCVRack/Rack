#pragma once

#include <cstdint>
#include "DrumKit.hpp"

namespace rack_plugin_DrumKit {

struct DrumContainer {
  float *sample;
  unsigned int length;
};

struct DrumModule : Module {
  enum ParamIds { DRUM1_PARAM, DRUM2_PARAM, NUM_PARAMS };
  enum InputIds { CLOCK1_INPUT, CLOCK2_INPUT, NUM_INPUTS };
  enum OutputIds { AUDIO1_OUTPUT, AUDIO2_OUTPUT, NUM_OUTPUTS };
  enum LightIds { NUM_LIGHTS };

  DrumModule( );
  void step( ) override;
  virtual void setupSamples( );
  struct DrumContainer *getSample(float);

  SynthDevKit::CV *cv1;
  uint32_t currentStep1;
  bool ready1;
  SynthDevKit::CV *cv2;
  uint32_t currentStep2;
  bool ready2;
  struct DrumContainer samples[32];
  uint8_t numSamples;
};

} // namespace rack_plugin_DrumKit
