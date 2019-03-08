#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct BISTROT : Module {
	enum ParamIds {
		LINK_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT,
    ADCCLOCK_INPUT,
    DACCLOCK_INPUT,
		BIT_INPUT,
		NUM_INPUTS = BIT_INPUT + 8
	};
	enum OutputIds {
		OUTPUT,
    BIT_OUTPUT,
		NUM_OUTPUTS  = BIT_OUTPUT + 8
	};
	enum LightIds {
    LINK_LIGHT,
    BIT_INPUT_LIGHTS,
    BIT_OUTPUT_LIGHTS = BIT_INPUT_LIGHTS + 8,
		NUM_LIGHTS = BIT_OUTPUT_LIGHTS + 8
	};

  SchmittTrigger linkTrigger, acdClockTrigger, dacClockTrigger;

  unsigned char in = 0;
  unsigned char out = 0;

	BISTROT() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

	}

	~BISTROT() {
  }

	void step() override;
};


void BISTROT::step() {
    if ((!inputs[ADCCLOCK_INPUT].active) || (acdClockTrigger.process(inputs[ADCCLOCK_INPUT].value)))
    {
      in = roundf(clamp(clamp(inputs[INPUT].value,-10.0f,10.0f) / 20.0f + 0.5f, 0.0f, 1.0f) * 255);
    }

    for (int i = 0 ; i != 8 ; i++)
    {
      int bitValue = ((in & (1U << i)) != 0);
      lights[BIT_INPUT_LIGHTS+i].value = 1-bitValue;
      outputs[BIT_OUTPUT+i].value = (1-bitValue) * 10;
    }

    if ((!inputs[DACCLOCK_INPUT].active) || (dacClockTrigger.process(inputs[DACCLOCK_INPUT].value)))
    {
      for (int i = 0 ; i != 8 ; i++)
      {
        if ((inputs[BIT_INPUT+i].active) && (inputs[BIT_INPUT+i].value != 0)) {
          out |= 1U << i;
        }
				else {
					out &= ~(1U << i);
				}
        lights[BIT_OUTPUT_LIGHTS+i].value = (out >> i) & 1U;
      }
    }
		outputs[OUTPUT].value = -1.0f * clamp(((((float)out/255.0f))-0.5f)*10.0f,-10.0f,10.0f);
}

struct BISTROTWidget : ModuleWidget {
	BISTROTWidget(BISTROT *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BISTROT.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(Port::create<PJ301MPort>(Vec(29.0f, 46.0f), Port::INPUT, module, BISTROT::ADCCLOCK_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(67.0f, 46.0f), Port::INPUT, module, BISTROT::DACCLOCK_INPUT));

    for (int i = 0; i<8; i++) {
      addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(18.0f, 97.5f + 26.0f * i), module, BISTROT::BIT_INPUT_LIGHTS + i));
			addOutput(Port::create<TinyPJ301MPort>(Vec(34.0f, 93.0f + 26.0f * i), Port::OUTPUT, module, BISTROT::BIT_OUTPUT + i));
			addInput(Port::create<TinyPJ301MPort>(Vec(72.0f, 93.0f + 26.0f * i), Port::INPUT, module, BISTROT::BIT_INPUT + i));
			addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(95.0f, 97.5f + 26.0f * i), module, BISTROT::BIT_OUTPUT_LIGHTS + i));
    }

		addInput(Port::create<PJ301MPort>(Vec(29.0f, 320.0f), Port::INPUT, module, BISTROT::INPUT));
		// addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 339.0f), Port::INPUT, module, BISTROT::R_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(67.0f, 320.0f),Port::OUTPUT, module, BISTROT::OUTPUT));
		// addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 339.0f),Port::OUTPUT, module, BISTROT::R_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, BISTROT) {
   Model *modelBISTROT = Model::create<BISTROT, BISTROTWidget>("Bidoo", "BISTROT", "BISTROT bit crusher", EFFECT_TAG, DIGITAL_TAG, DISTORTION_TAG);
   return modelBISTROT;
}
