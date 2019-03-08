#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct RABBIT : Module {
	enum ParamIds {
		BITOFF_PARAM,
    BITREV_PARAM = BITOFF_PARAM + 8,
		NUM_PARAMS = BITREV_PARAM + 8
	};
	enum InputIds {
		L_INPUT,
    R_INPUT,
		BITOFF_INPUT,
		BITREV_INPUT = BITOFF_INPUT + 8,
		NUM_INPUTS = BITREV_INPUT + 8
	};
	enum OutputIds {
		L_OUTPUT,
    R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
    BITOFF_LIGHTS,
    BITREV_LIGHTS = BITOFF_LIGHTS + 8,
		NUM_LIGHTS = BITREV_LIGHTS + 8
	};

  SchmittTrigger bitOffTrigger[8], bitRevTrigger[8];

  bool bitOff[8];
  bool bitRev[8];

	RABBIT() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    memset(&bitOff,0,8*sizeof(bool));
    memset(&bitRev,0,8*sizeof(bool));
	}

	~RABBIT() {
  }

	json_t *toJson() override {
		json_t *rootJ = json_object();
		for (int i=0; i<8; i++) {
			json_object_set_new(rootJ, ("bitOff" + to_string(i)).c_str(), json_boolean(bitOff[i]));
			json_object_set_new(rootJ, ("bitRev" + to_string(i)).c_str(), json_boolean(bitRev[i]));
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		for (int i=0; i<8; i++) {
			json_t *jbitOff = json_object_get(rootJ, ("bitOff" + to_string(i)).c_str());
			if (jbitOff) {
				bitOff[i] = json_is_true(jbitOff) ? 1 : 0;
			}
			json_t *jbitRev = json_object_get(rootJ, ("bitRev" + to_string(i)).c_str());
			if (jbitRev) {
				bitRev[i] = json_is_true(jbitRev) ? 1 : 0;
			}
		}
	}

	void step() override;
};


void RABBIT::step() {
  float in_L = clamp(inputs[L_INPUT].value,-10.0f,10.0f);
  float in_R = clamp(inputs[R_INPUT].value,-10.0f,10.0f);

  in_L = roundf(clamp(in_L / 20.0f + 0.5f, 0.0f, 1.0f) * 255);
  in_R = roundf(clamp(in_R / 20.0f + 0.5f, 0.0f, 1.0f) * 255);

  unsigned char red_L = in_L;
  unsigned char red_R = in_R;

  for (int i = 0 ; i < 8 ; i++ ) {

    if (bitOffTrigger[i].process(params[BITOFF_PARAM+i].value + inputs[BITOFF_INPUT+i].value))
    {
      bitOff[i] = !bitOff[i];
    }

    if (bitRevTrigger[i].process(params[BITREV_PARAM+i].value + inputs[BITREV_INPUT+i].value))
    {
      bitRev[i] = !bitRev[i];
    }

    if (bitOff[i]) {
      red_L &= ~(1 << i);
      red_R &= ~(1 << i);
    }
    else {
      if (bitRev[i]) {
        red_L ^= ~(1 << i);
        red_R ^= ~(1 << i);
      }
    }

    lights[BITOFF_LIGHTS + i].value = bitOff[i] ? 1.0f : 0.0f;
    lights[BITREV_LIGHTS + i].value = bitRev[i] ? 1.0f : 0.0f;
  }

  outputs[L_OUTPUT].value = clamp(((((float)red_L/255.0f))-0.5f)*20.0f,-10.0f,10.0f);
  outputs[R_OUTPUT].value = clamp(((((float)red_R/255.0f))-0.5f)*20.0f,-10.0f,10.0f);
}

template <typename BASE>
struct RabbitLight : BASE {
	RabbitLight() {
		this->box.size = mm2px(Vec(6.0f, 6.0f));
	}
};

struct RABBITWidget : ModuleWidget {
	RABBITWidget(RABBIT *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/RABBIT.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    for (int i = 0; i<8; i++) {
    	addParam(ParamWidget::create<LEDBezel>(Vec(27.0f, 50.0f + 32.0f * i), module, RABBIT::BITOFF_PARAM + i, 0.0f, 1.0f, 0.0f));
      addChild(ModuleLightWidget::create<RabbitLight<RedLight>>(Vec(29.0f, 52.0f + 32.0f * i), module, RABBIT::BITOFF_LIGHTS + i));

			addInput(Port::create<TinyPJ301MPort>(Vec(8.0f, 54.0f + 32.0f * i), Port::INPUT, module, RABBIT::BITOFF_INPUT + i));
			addInput(Port::create<TinyPJ301MPort>(Vec(83.0f, 54.0f + 32.0f * i), Port::INPUT, module, RABBIT::BITREV_INPUT + i));

      addParam(ParamWidget::create<LEDBezel>(Vec(57.0f, 50.0f + 32.0f * i), module, RABBIT::BITREV_PARAM + i, 0.0f, 1.0f, 0.0f));
			addChild(ModuleLightWidget::create<RabbitLight<BlueLight>>(Vec(59.0f, 52.0f + 32.0f * i), module, RABBIT::BITREV_LIGHTS + i));
    }

		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 319.0f), Port::INPUT, module, RABBIT::L_INPUT));
		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 339.0f), Port::INPUT, module, RABBIT::R_INPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 319.0f),Port::OUTPUT, module, RABBIT::L_OUTPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 339.0f),Port::OUTPUT, module, RABBIT::R_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, RABBIT) {
   Model *modelRABBIT = Model::create<RABBIT, RABBITWidget>("Bidoo", "rabBIT", "rabBIT bit crusher", EFFECT_TAG, DIGITAL_TAG, DISTORTION_TAG);
   return modelRABBIT;
}
