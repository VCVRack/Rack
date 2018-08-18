#include "ClockDivider.hpp"

namespace rack_plugin_SynthKit {

ClockDividerModule::ClockDividerModule()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  clock = new SynthDevKit::Clock(8, 1.7f);
  cv = new SynthDevKit::CV(1.7f);
}

void ClockDividerModule::step() {
  float reset_in = inputs[RESET_INPUT].value;

  cv->update(reset_in);

  if (cv->newTrigger()) {
    clock->reset();
  }

  float in = inputs[TOP_INPUT].value;
  bool *states = clock->update(in);

  for (int i = 0; i < 8; i++) {
    if (states[i] == true) {
      outputs[i].value = in;
      lights[i].value = 1.0f;
    } else {
      outputs[i].value = 0;
      lights[i].value = 0;
    }
  }
}

} // namespace rack_plugin_SynthKit
