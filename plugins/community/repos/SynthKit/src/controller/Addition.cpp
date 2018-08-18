#include "Addition.hpp"

namespace rack_plugin_SynthKit {

AdditionModule::AdditionModule()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  // nothing to do here
}

void AdditionModule::step() {
  float top1 = inputs[TOP1_INPUT].value;
  float top2 = inputs[TOP2_INPUT].value;
  float bottom1 = inputs[BOTTOM1_INPUT].value;
  float bottom2 = inputs[BOTTOM2_INPUT].value;

  float val1 = top1 + top2;
  float val2 = bottom1 + bottom2;
  outputs[TOP_OUTPUT].value = val1;
  outputs[BOTTOM_OUTPUT].value = val2;
}

} // namespace rack_plugin_SynthKit
