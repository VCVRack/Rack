#include <stdint.h>

#include "../deps/SynthDevKit/src/CV.hpp"
#include "DrumKit.hpp"
#include "DrumModule.hpp"

namespace rack_plugin_DrumKit {

DrumModule::DrumModule( ) : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    cv1          = new SynthDevKit::CV(1.7f);
    cv2          = new SynthDevKit::CV(1.7f);
    currentStep1 = 0;
    currentStep2 = 0;
    ready1       = false;
    ready2       = false;
    numSamples = 0;
  }


struct DrumContainer *DrumModule::getSample(float current) {
  if (numSamples == 0) {
    setupSamples();
  }
  if (current < 1 || current >= numSamples) {
    return &samples[ 0 ];
  }
  return &samples[ (int)current - 1 ];
}

void DrumModule::step( ) {
  float in1 = inputs[ CLOCK1_INPUT ].value;
  cv1->update(in1);

  if (cv1->newTrigger( )) {
    if (!ready1) {
      ready1 = true;
    }

    currentStep1 = 0;
  }

  float current1           = params[ DRUM1_PARAM ].value;
  struct DrumContainer *c = getSample(current1);

  if (currentStep1 >= c->length) {
    outputs[ AUDIO1_OUTPUT ].value = 0;
  } else {
    outputs[ AUDIO1_OUTPUT ].value = c->sample[ currentStep1 ];
    currentStep1++;
  }

  float in2 = inputs[ CLOCK2_INPUT ].value;
  cv2->update(in2);

  if (cv2->newTrigger( )) {
    if (!ready2) {
      ready2 = true;
    }

    currentStep2 = 0;
  }

  float current2 = params[ DRUM2_PARAM ].value;
  c              = getSample(current2);

  if (currentStep2 >= c->length) {
    outputs[ AUDIO2_OUTPUT ].value = 0;
  } else {
    outputs[ AUDIO2_OUTPUT ].value = c->sample[ currentStep2 ];
    currentStep2++;
  }
}

void DrumModule::setupSamples( ) { }

} // namespace rack_plugin_DrumKit


