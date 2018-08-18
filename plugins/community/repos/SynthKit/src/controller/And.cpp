#include "And.hpp"

namespace rack_plugin_SynthKit {

AndModule::AndModule()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  // nothing to do here
}

void AndModule::step() {
  float top1 = inputs[TOP1_INPUT].value;
  float top2 = inputs[TOP2_INPUT].value;
  float bottom1 = inputs[BOTTOM1_INPUT].value;
  float bottom2 = inputs[BOTTOM2_INPUT].value;

  double val1 =
      (double)((long long)(top1 * 10000) & (long long)(top2 * 10000)) / 10000;
  double val2 =
      (double)((long long)(bottom1 * 10000) & (long long)(bottom2 * 10000)) /
      10000;

  outputs[TOP_OUTPUT].value = (float)val1;
  outputs[BOTTOM_OUTPUT].value = (float)val2;
}

} // namespace rack_plugin_SynthKit
