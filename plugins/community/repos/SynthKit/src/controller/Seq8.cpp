#include "Seq8.hpp"

namespace rack_plugin_SynthKit {

Seq8Module::Seq8Module()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  cv = new SynthDevKit::CV(1.7f);
  currentStep = 0;
}

void Seq8Module::step() {
  float in = inputs[CLOCK_INPUT].value;
  cv->update(in);

  if (cv->newTrigger()) {
    float note_param = params[SEQ1_PARAM + currentStep].value;
    float oct_param = params[OCTAVE1_PARAM + currentStep].value;

    outputs[GATE_OUTPUT].value = notes[(int)note_param] + octives[(int)oct_param];

    for (int i = 0; i < 8; i++) {
      if (i == currentStep) {
        lights[i].value = 1.0f;
      } else {
        lights[i].value = 0.0f;
      }
    }

    currentStep++;

    if (currentStep == 8) {
      currentStep = 0;
    }
  }
}

} // namespace rack_plugin_SynthKit
