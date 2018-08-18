#pragma once

#include "../../deps/SynthDevKit/src/CV.hpp"
#include "../SynthKit.hpp"

namespace rack_plugin_SynthKit {

struct M1x8CVModule : Module {
  enum ParamIds { NUM_PARAMS };
  enum InputIds {
    TOP_INPUT,
    FIRST_CV,
    SECOND_CV,
    THIRD_CV,
    FOURTH_CV,
    FIFTH_CV,
    SIXTH_CV,
    SEVENTH_CV,
    EIGHTH_CV,
    NUM_INPUTS
  };
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
  enum LightIds {
    FIRST_LED,
    SECOND_LED,
    THIRD_LED,
    FOURTH_LED,
    FIFTH_LED,
    SIXTH_LED,
    SEVENTH_LED,
    EIGHTH_LED,
    NUM_LIGHTS
  };

  M1x8CVModule();
  void step() override;

  SynthDevKit::CV *cv[8];
};

} // namespace rack_plugin_SynthKit
