#pragma once

#include "../SynthKit.hpp"

namespace rack_plugin_SynthKit {

struct AndModule : Module {
  enum ParamIds { NUM_PARAMS };
  enum InputIds {
    TOP1_INPUT,
    TOP2_INPUT,
    BOTTOM1_INPUT,
    BOTTOM2_INPUT,
    NUM_INPUTS
  };
  enum OutputIds { TOP_OUTPUT, BOTTOM_OUTPUT, NUM_OUTPUTS };
  enum LightIds { NUM_LIGHTS };

  AndModule();
  void step() override;
};

} // namespace rack_plugin_SynthKit
