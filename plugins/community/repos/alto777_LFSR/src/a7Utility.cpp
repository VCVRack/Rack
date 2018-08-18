#include "LFSR.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_alto777_LFSR {

struct a7Utility : Module {
	enum ParamIds {
		ENUMS(MBUTTON_PARAM, 2),
		ENUMS(MMODE_PARAM, 2),

		ENUMS(CCTRL_PARAM, 2),
		ENUMS(CAMP_PARAM, 2),
		ENUMS(CRANGE_PARAM, 2),
		
		ENUMS(WIDTH_PARAM, 2),

		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(CCTRL_INPUT, 2),

		ENUMS(EDGE_INPUT, 2),

		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(MGATE_OUTPUT, 2),
		ENUMS(MTRIG_OUTPUT, 2),

		ENUMS(COUT_OUTPUT, 2),

		ENUMS(RISE_OUTPUT, 2),
		ENUMS(FALL_OUTPUT, 2),

		ENUMS(CLOCK_INV_OUTPUT, 2),

		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(M_LIGHT, 2),

		ENUMS(RISE_LIGHT, 2),
		ENUMS(FALL_LIGHT, 2),

		NUM_LIGHTS
	};

/* buttons */
	SchmittTrigger manualButton[2];
	PulseGenerator manualTriggerPulse[2];

	bool mState[2] = {0, 0};

/* constants - no variables */

/* edge detector */
	bool signalState[2] = {0, 0};
	PulseGenerator risingEdgePulse[2];
	PulseGenerator fallingEdgePulse[2];

	PulseGenerator risingEdgeLampStab[2];
	PulseGenerator fallingEdgeLampStab[2];

	a7Utility() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
	
	void onReset() override {
		mState[0] = 0;
	};
/**/
	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_t *mstate = json_array();

		for (int i = 0; i < 2; i++)
			json_array_insert_new(mstate, i, json_integer((int) mState[i]));

		json_object_set_new(rootJ, "mstate", mstate);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *mstate = json_object_get(rootJ, "mstate");

		if (mstate) {
			for (int i = 0; i < 2; i++) {
				json_t *wha = json_array_get(mstate, i);
				if (wha)
					mState[i] = json_integer_value(wha); // warning C4800: 'json_int_t': forcing value to bool 'true' or 'false' (performance warning)
			}
		}
	}
/**/
};

void a7Utility::step() {
	float deltaTime = 1.0 / engineGetSampleRate();





/* manual button to gate / trigger section */
	for (int i = 0; i < 2; i++) {
		// mode 0 = momentary 1 = toggle 
		bool mode = params[MMODE_PARAM + i].value > 0.0f;
		if (!mode)
			mState[i] = 0;

		if (manualButton[i].process(params[MBUTTON_PARAM + i].value)) {
			manualTriggerPulse[i].trigger(0.001);
			if (mode) mState[i] ^= 1;
		}
	
		bool bState = manualButton[i].isHigh();	
		if (mState[i] || (!mode && bState)) {
			outputs[MGATE_OUTPUT + i].value = 10.0f;
			lights[M_LIGHT + i].value = 1.0f;
		} 
		else {
			outputs[MGATE_OUTPUT + i].value = 0.0f;
			lights[M_LIGHT + i].value = 0.0;
		}

		outputs[MTRIG_OUTPUT + i].value =  manualTriggerPulse[i].process(deltaTime) ? 10.0f : 0.0;
	}





/* constant from knob switched by control */
	for (int i = 0; i < 2; i++) {
		bool isOn = params[CCTRL_PARAM + i].value <= 0.0f;
		bool rangeX10 = params[CRANGE_PARAM + i].value <= 0.0f;
		bool extControl = inputs[CCTRL_INPUT + i].active;
		bool nonZero = 0;

		if (isOn) nonZero = 1;
		else {
			if (extControl) {
				if (inputs[CCTRL_INPUT + i].value > 0.0f) nonZero = 1;
			}
			else nonZero = 1;
		}

		outputs[COUT_OUTPUT + i].value = nonZero ? (params[CAMP_PARAM + i].value * (rangeX10 ? 10.0f : 1.0f)) : 0;
	}

/* generate pulse on clock edges */
	for (int i = 0; i < 2; i++) {
		if (signalState[i]) {
			if (inputs[EDGE_INPUT + i].value < 0.3f) {
				signalState[i] = 0;
				fallingEdgePulse[i].trigger(0.001); // (params[WIDTH_PARAM + i].value <= 0.0f ? 0.005f : 0.0005f);
				fallingEdgeLampStab[i].trigger(0.25); // (params[WIDTH_PARAM + i].value <= 0.0f ? 0.005f : 0.0005f);
			}	
		}
		else {
			if (inputs[EDGE_INPUT + i].value > 0.7f) {
				signalState[i] = 1;
				risingEdgePulse[i].trigger(0.001); // (params[WIDTH_PARAM + i].value <= 0.0f ? 0.005f : 0.0005f);
				risingEdgeLampStab[i].trigger(0.25); // params[WIDTH_PARAM + i].value <= 0.0f ? 0.33f : 0.1f);
			}	
		}

		outputs[RISE_OUTPUT + i].value = risingEdgePulse[i].process(deltaTime) ? 10.0f : 0.0;
		outputs[FALL_OUTPUT + i].value = fallingEdgePulse[i].process(deltaTime) ? 10.0f : 0.0;

		outputs[CLOCK_INV_OUTPUT + i].value = signalState[i] ? 0.0f : 10.0f;

//!		outputs[CLOCK_INV_OUTPUT + i].value = (inputs[EDGE_INPUT + i].value < 0.3f) ? 10.0f : 0.0f;

		lights[RISE_LIGHT + i].value = risingEdgeLampStab[i].process(deltaTime) ? 1.0f : 0.0;
		lights[FALL_LIGHT + i].value = fallingEdgeLampStab[i].process(deltaTime) ? 1.0f : 0.0;
	}
}

/* why oh */
struct myCKSS : SVGSwitch, ToggleSwitch {
	myCKSS() {
		addFrame(SVG::load(assetPlugin(plugin, "res/myCKSS_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/myCKSS_1.svg")));
	}
};

struct myHCKSS : SVGSwitch, ToggleSwitch {
	myHCKSS() {
		addFrame(SVG::load(assetPlugin(plugin, "res/myHCKSS_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/myHCKSS_1.svg")));
	}
};

struct HexCapScrew0 : SVGScrew {
	HexCapScrew0() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/HexCapScrewSilver.svg")));
		box.size = sw->box.size;
	}
};

struct HexCapScrew1 : SVGScrew {
	HexCapScrew1() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/HexCapScrewSilver9.svg")));
		box.size = sw->box.size;
	}
};

struct HexCapScrew2 : SVGScrew {
	HexCapScrew2() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/HexCapScrewSilver13.svg")));
		box.size = sw->box.size;
	}
};

struct HexCapScrew3 : SVGScrew {
	HexCapScrew3() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/HexCapScrewSilver21.svg")));
		box.size = sw->box.size;
	}
};


struct a7UtilityWidget : ModuleWidget {
	a7UtilityWidget(a7Utility *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/a7Utility.svg")));

/*
	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
*/
	addChild(Widget::create<HexCapScrew3>(Vec(0, 0)));
	addChild(Widget::create<HexCapScrew2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<HexCapScrew0>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<HexCapScrew1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(0.798, 13.042)), module, a7Utility::MBUTTON_PARAM + 0, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(25.851, 13.042)), module, a7Utility::MBUTTON_PARAM + 1, 0.0, 1.0, 0.0));

	addParam(ParamWidget::create<myCKSS>(mm2px(Vec(13.949, 14.059)), module, a7Utility::MMODE_PARAM + 0, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<myCKSS>(mm2px(Vec(39.003, 14.059)), module, a7Utility::MMODE_PARAM + 1, 0.0, 1.0, 0.0));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(0.37, 23.495)), Port::OUTPUT, module, a7Utility::MTRIG_OUTPUT + 0));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(11.623, 23.495)), Port::OUTPUT, module, a7Utility::MGATE_OUTPUT + 0));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(25.424, 23.495)), Port::OUTPUT, module, a7Utility::MTRIG_OUTPUT + 1));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(36.677, 23.495)), Port::OUTPUT, module, a7Utility::MGATE_OUTPUT + 1));

	addChild(ModuleLightWidget::create<LargeLight<RedLight>>(mm2px(Vec(1.958, 14.2)), module, a7Utility::M_LIGHT + 0 ));
	addChild(ModuleLightWidget::create<LargeLight<RedLight>>(mm2px(Vec(27.012, 14.2)), module, a7Utility::M_LIGHT + 1));




	addParam(ParamWidget::create<myHCKSS>(mm2px(Vec(23.806, 51.25)), module, a7Utility::CCTRL_PARAM + 0, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(mm2px(Vec(9.682, 53.452)), module, a7Utility::CAMP_PARAM + 0, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<myCKSS>(mm2px(Vec(2.208, 55.72)), module, a7Utility::CRANGE_PARAM + 0, 0.0, 1.0, 0.0));

	addParam(ParamWidget::create<myHCKSS>(mm2px(Vec(23.806, 70.872)), module, a7Utility::CCTRL_PARAM + 1, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(mm2px(Vec(9.682, 73.073)), module, a7Utility::CAMP_PARAM + 1, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<myCKSS>(mm2px(Vec(2.208, 75.341)), module, a7Utility::CRANGE_PARAM + 1, 0.0, 1.0, 0.0));

	addInput(Port::create<PJ301MPort>(mm2px(Vec(22.359, 57.296)), Port::INPUT, module, a7Utility::CCTRL_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(22.359, 76.917)), Port::INPUT, module, a7Utility::CCTRL_INPUT + 1));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(34.118, 54.272)), Port::OUTPUT, module, a7Utility::COUT_OUTPUT + 0));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(34.118, 73.894)), Port::OUTPUT, module, a7Utility::COUT_OUTPUT + 1));



//	addParam(ParamWidget::create<myCKSS>(mm2px(Vec(36.288, 103.218)), module, a7Utility::WIDTH_PARAM + 0, 0.0, 1.0, 0.0));
//	addParam(ParamWidget::create<myCKSS>(mm2px(Vec(36.288, 115.388)), module, a7Utility::WIDTH_PARAM + 1, 0.0, 1.0, 0.0));

	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.113, 101.77)), Port::INPUT, module, a7Utility::EDGE_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.113, 113.7)), Port::INPUT, module, a7Utility::EDGE_INPUT + 1));



	addOutput(Port::create<PJ301MPort>(mm2px(Vec(14.184, 101.77)), Port::OUTPUT, module, a7Utility::RISE_OUTPUT + 0));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(23.701, 101.77)), Port::OUTPUT, module, a7Utility::FALL_OUTPUT + 0));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(34.118, 101.77)), Port::OUTPUT, module, a7Utility::CLOCK_INV_OUTPUT + 0));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(14.184, 113.7)), Port::OUTPUT, module, a7Utility::RISE_OUTPUT + 1));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(23.701, 113.7)), Port::OUTPUT, module, a7Utility::FALL_OUTPUT + 1));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(34.118, 113.7)), Port::OUTPUT, module, a7Utility::CLOCK_INV_OUTPUT + 1));

	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(mm2px(Vec(16.773, 97.812)), module, a7Utility::RISE_LIGHT + 0));
//	addChild(ModuleLightWidget::create<MediumLight<YellowLight>>(mm2px(Vec(16.773, 97.812)), module, a7Utility::RISE_LIGHT + 0));
	addChild(ModuleLightWidget::create<MediumLight<YellowLight>>(mm2px(Vec(26.291, 97.812)), module, a7Utility::FALL_LIGHT + 0));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(mm2px(Vec(16.773, 122.818)), module, a7Utility::RISE_LIGHT + 1));
//	addChild(ModuleLightWidget::create<MediumLight<YellowLight>>(mm2px(Vec(16.773, 122.818)), module, a7Utility::RISE_LIGHT + 1));
	addChild(ModuleLightWidget::create<MediumLight<YellowLight>>(mm2px(Vec(26.291, 122.818)), module, a7Utility::FALL_LIGHT + 1));



	}
};

} // namespace rack_plugin_alto777_LFSR

using namespace rack_plugin_alto777_LFSR;

RACK_PLUGIN_MODEL_INIT(alto777_LFSR, a7Utility) {
   Model *modela7Utility = Model::create<a7Utility, a7UtilityWidget>("alto777_LFSR", "a7Utility", "a7Utility VIS", UTILITY_TAG);
   return modela7Utility;
}
