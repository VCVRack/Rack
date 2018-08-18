#include "1x8CV.hpp"

namespace rack_plugin_SynthKit {

M1x8CVModule::M1x8CVModule()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  for (int i = 0; i < 8; i++) {
    cv[i] = new SynthDevKit::CV(1.7f);
  }
}

void M1x8CVModule::step() {
  float in = inputs[TOP_INPUT].value;

  // this logic can be greatly simplified with a loop
  cv[0]->update(inputs[FIRST_CV].value);
  cv[1]->update(inputs[SECOND_CV].value);
  cv[2]->update(inputs[THIRD_CV].value);
  cv[3]->update(inputs[FOURTH_CV].value);
  cv[4]->update(inputs[FIFTH_CV].value);
  cv[5]->update(inputs[SIXTH_CV].value);
  cv[6]->update(inputs[SEVENTH_CV].value);
  cv[7]->update(inputs[EIGHTH_CV].value);

  if (cv[0]->isHigh()) {
    outputs[FIRST_OUTPUT].value = in;
    lights[FIRST_LED].value = 1;
  } else {
    outputs[FIRST_OUTPUT].value = 0;
    lights[FIRST_LED].value = 0;
  }

  if (cv[1]->isHigh()) {
    outputs[SECOND_OUTPUT].value = in;
    lights[SECOND_LED].value = 1;
  } else {
    outputs[SECOND_OUTPUT].value = 0;
    lights[SECOND_LED].value = 0;
  }

  if (cv[2]->isHigh()) {
    outputs[THIRD_OUTPUT].value = in;
    lights[THIRD_LED].value = 1;
  } else {
    outputs[THIRD_OUTPUT].value = 0;
    lights[THIRD_LED].value = 0;
  }

  if (cv[3]->isHigh()) {
    outputs[FOURTH_OUTPUT].value = in;
    lights[FOURTH_LED].value = 1;
  } else {
    outputs[FOURTH_OUTPUT].value = 0;
    lights[FOURTH_LED].value = 0;
  }

  if (cv[4]->isHigh()) {
    outputs[FIFTH_OUTPUT].value = in;
    lights[FIFTH_LED].value = 1;
  } else {
    outputs[FIFTH_OUTPUT].value = 0;
    lights[FIFTH_LED].value = 0;
  }

  if (cv[5]->isHigh()) {
    outputs[SIXTH_OUTPUT].value = in;
    lights[SIXTH_LED].value = 1;
  } else {
    outputs[SIXTH_OUTPUT].value = 0;
    lights[SIXTH_LED].value = 0;
  }

  if (cv[6]->isHigh()) {
    outputs[SEVENTH_OUTPUT].value = in;
    lights[SEVENTH_LED].value = 1;
  } else {
    outputs[SEVENTH_OUTPUT].value = 0;
    lights[SEVENTH_LED].value = 0;
  }

  if (cv[7]->isHigh()) {
    outputs[EIGHTH_OUTPUT].value = in;
    lights[EIGHTH_LED].value = 1;
  } else {
    outputs[EIGHTH_OUTPUT].value = 0;
    lights[EIGHTH_LED].value = 0;
  }
}

} // namespace rack_plugin_SynthKit
