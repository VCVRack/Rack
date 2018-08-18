#include <cstdint>

#include "../../deps/SynthDevKit/src/CV.hpp"
#include "../../deps/SynthDevKit/src/FibonacciClock.hpp"
#include "../SynthKit.hpp"

namespace rack_plugin_SynthKit {

struct FibonacciClockDividerModule : Module {
  enum ParamIds { NUM_PARAMS };
  enum InputIds { TOP_INPUT, RESET_INPUT, NUM_INPUTS };
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

  FibonacciClockDividerModule();
  void step() override;

  SynthDevKit::FibonacciClock *clock;
  SynthDevKit::CV *cv;
};

} // namespace rack_plugin_SynthKit
