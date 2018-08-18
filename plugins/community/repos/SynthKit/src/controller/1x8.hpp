#pragma once

#include "../SynthKit.hpp"

namespace rack_plugin_SynthKit {

struct M1x8Module : Module {
  enum ParamIds { NUM_PARAMS };
  enum InputIds { TOP_INPUT, NUM_INPUTS };
  enum OutputIds {
    FIRST_OUTPUT,
    SECOND_OUTPUT,
    THIRD_OUTPUT,
    FOURTH_OUTPUT,
    FIFTH_OUTPUT,
    SIXTH_OUTPUT,
    SEVENTH_OUTPUT,
    EIGHTH_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds { NUM_LIGHTS };

  M1x8Module();
  void step() override;
};

} // namespace rack_plugin_SynthKit
