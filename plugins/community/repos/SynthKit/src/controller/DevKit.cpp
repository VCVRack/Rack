#include "DevKit.hpp"

namespace rack_plugin_SynthKit {

DevKitModule::DevKitModule()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  cv = new SynthDevKit::CV(1.5f);
}

void DevKitModule::step() {
  float in = inputs[DEV_INPUT].value;

  if (min > in) {
    min = in;
  }

  if (max < in) {
    max = in;
  }

  cv->update(in);
  lights[BLINK_LIGHT].value = cv->isHigh() ? 1.0 : 0.0;

  if (cv->newTrigger()) {
    cvcount++;
    cvinterval = cv->triggerInterval();
  }
}

} // namespace rack_plugin_SynthKit
