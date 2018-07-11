#include <stdint.h>

#include "DrumKit.hpp"
#include "dmx.h"

namespace rack_plugin_DrumKit {

struct DMXContainer {
  float *sample;
  unsigned int length;
  int current;
};

struct DMXContainer dmxsamples[ 12 ] = {
  { (float *)dmx1, dmx1_len, 0 },
  { (float *)dmx2, dmx2_len, 1 },
  { (float *)dmx3, dmx3_len, 2 },
  { (float *)dmx4, dmx4_len, 3 },
  { (float *)dmx5, dmx5_len, 4 },
  { (float *)dmx6, dmx6_len, 5 },
  { (float *)dmx7, dmx7_len, 6 },
  { (float *)dmx8, dmx8_len, 7 },
  { (float *)dmx9, dmx9_len, 8 },
  { (float *)dmx10, dmx10_len, 9 },
  { (float *)dmx11, dmx11_len, 10 },
  { (float *)dmx12, dmx12_len, 11 },
};

float dmxnotes[12] = { 0.08, 0.17, 0.25, 0.33, 0.42, 0.5, 0.58, 0.67, 0.75, 0.83, 0.92, 1.0 };


struct DMXContainer *getNote(float current) {
  for (int i = 0; i < 12; i++) {
    if ((dmxnotes[i] - 0.02) <= current && (dmxnotes[i] + 0.02) >= current) {
      return &dmxsamples[i];
    }
  }

  return NULL;
}

struct DMXModule : Module {
  enum ParamIds { NUM_PARAMS };
  enum InputIds { NOTE1_INPUT, NUM_INPUTS };
  enum OutputIds { AUDIO1_OUTPUT, NUM_OUTPUTS };
  enum LightIds { NUM_LIGHTS };

  DMXModule( ) : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    currentStep = 0;
    last = -1;
  }

  void step( ) override;

  uint32_t currentStep;
  int last;
};

void DMXModule::step( ) {
  float in1 = inputs[ NOTE1_INPUT ].value;
  struct DMXContainer *note;

  // check the first note
  note = getNote(in1);

  if (note == NULL) {
    currentStep = 0;
    outputs[ AUDIO1_OUTPUT ].value = 0;
    last = -1;
  } else {
    if (last != note->current) {
      last = note->current;
      currentStep = 0;
    }

    if (currentStep >= note->length) {
      outputs[ AUDIO1_OUTPUT ].value = 0;
    } else {
      outputs[ AUDIO1_OUTPUT ].value = note->sample[currentStep];
      currentStep++;
    }
  }
}

struct DMXWidget : ModuleWidget {
  DMXWidget(DMXModule *module);
};

DMXWidget::DMXWidget(DMXModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel( );
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/DMX.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 45), Port::INPUT, module,
                                   DMXModule::NOTE1_INPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 92), Port::OUTPUT, module,
                                     DMXModule::AUDIO1_OUTPUT));

}

} // namespace rack_plugin_DrumKit

using namespace rack_plugin_DrumKit;

RACK_PLUGIN_MODEL_INIT(DrumKit, DMX) {
   Model *modelDMX = Model::create<DMXModule, DMXWidget>("DrumKit", "DMX", "DMX", DRUM_TAG);
   return modelDMX;
}
