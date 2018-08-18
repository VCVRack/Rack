#include <cstdio>

#include "../../deps/SynthDevKit/src/CV.hpp"
#include "../SynthKit.hpp"

namespace rack_plugin_SynthKit {

struct DevKitModule : Module {
  enum ParamIds { NUM_PARAMS };
  enum InputIds { DEV_INPUT, NUM_INPUTS };
  enum OutputIds { NUM_OUTPUTS };
  enum LightIds { BLINK_LIGHT, NUM_LIGHTS };

  DevKitModule();
  void step() override;
  float min = 0;
  float max = 0;
  uint32_t cvcount = 0;
  uint32_t cvinterval = 0;
  SynthDevKit::CV *cv = new SynthDevKit::CV(1.3f);
};

} // namespace rack_plugin_SynthKit
