#include "1x8.hpp"

namespace rack_plugin_SynthKit {

M1x8Module::M1x8Module()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  // nothing to do here
}

void M1x8Module::step() {
  float in = inputs[TOP_INPUT].value;

  outputs[FIRST_OUTPUT].value = in;
  outputs[SECOND_OUTPUT].value = in;
  outputs[THIRD_OUTPUT].value = in;
  outputs[FOURTH_OUTPUT].value = in;
  outputs[FIFTH_OUTPUT].value = in;
  outputs[SIXTH_OUTPUT].value = in;
  outputs[SEVENTH_OUTPUT].value = in;
  outputs[EIGHTH_OUTPUT].value = in;
}

} // namespace rack_plugin_SynthKit
