#include "dsp/digital.hpp"
#include "ZZC.hpp"

namespace rack_plugin_ZZC {

const int NUM_CHANNELS = 8;

struct SH8 : Module {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    NOISE_INPUT,
    ENUMS(TRIG_INPUT, NUM_CHANNELS),
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(HOLD_OUTPUT, NUM_CHANNELS),
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  float last_trig_inputs[NUM_CHANNELS];
  SchmittTrigger triggers[NUM_CHANNELS];

  SH8() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;
};


void SH8::step() {
  bool previousIsTriggered = false;
  for (int i = 0; i < NUM_CHANNELS; i++) {
    if (inputs[TRIG_INPUT + i].active) {
      if (triggers[i].process(inputs[TRIG_INPUT + i].value)) {
        outputs[HOLD_OUTPUT + i].value = inputs[NOISE_INPUT].active ? inputs[NOISE_INPUT].value : randomNormal() * 2.0;
        previousIsTriggered = true;
      } else {
        previousIsTriggered = false;
      }
    } else {
      if (i > 0 && previousIsTriggered) {
        outputs[HOLD_OUTPUT + i].value = inputs[NOISE_INPUT].active ? inputs[NOISE_INPUT].value : randomNormal() * 2.0;
      }
    }
  }
}


struct SH8Widget : ModuleWidget {
  SH8Widget(SH8 *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/panels/SH-8.svg")));

    addInput(Port::create<ZZC_PJ_Port>(Vec(25, 53), Port::INPUT, module, SH8::NOISE_INPUT));

    for (int i = 0; i < NUM_CHANNELS; i++) {
      addInput(Port::create<ZZC_PJ_Port>(Vec(7.25f, 109 + 30 * i), Port::INPUT, module, SH8::TRIG_INPUT + i));
    }

    for (int i = 0; i < NUM_CHANNELS; i++) {
      addOutput(Port::create<ZZC_PJ_Port>(Vec(42.25f, 109 + 30 * i), Port::OUTPUT, module, SH8::HOLD_OUTPUT + i));
    }

    addChild(Widget::create<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};

} // namespace rack_plugin_ZZC

using namespace rack_plugin_ZZC;

RACK_PLUGIN_MODEL_INIT(ZZC, SH8) {
   Model *modelSH8 = Model::create<SH8, SH8Widget>("ZZC", "SH-8", "SH-8 Sample & Hold", NOISE_TAG, SAMPLE_AND_HOLD_TAG);
   return modelSH8;
}
