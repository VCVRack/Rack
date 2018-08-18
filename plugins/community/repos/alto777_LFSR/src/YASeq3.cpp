#include "LFSR.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_alto777_LFSR {

struct YASeq3 : Module {
	enum ParamIds {
		CLOCK_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		STEPS_PARAM,
		ENUMS(ROW1_PARAM, 8),
		ENUMS(ROW2_PARAM, 8),
		ENUMS(ROW3_PARAM, 8),
		ENUMS(GATE_PARAM, 8),
		
		ENUMS(MODE_PARAM, 3),
		ENUMS(RANGE_PARAM, 3),

		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		STEPS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATES_OUTPUT,
		ROW1_OUTPUT,
		ROW2_OUTPUT,
		ROW3_OUTPUT,
		ENUMS(GATE_OUTPUT, 8),
		NUM_OUTPUTS
	};
	enum LightIds {
		RUNNING_LIGHT,
		RESET_LIGHT,
//		GATES_LIGHT,
//		ENUMS(ROW_LIGHTS, 3),
		ENUMS(GATE_LIGHTS, 8),
		NUM_LIGHTS
	};

	bool running = true;
	SchmittTrigger clockTrigger;
	SchmittTrigger runningTrigger;
	SchmittTrigger resetTrigger;
	SchmittTrigger gateTriggers[8];
	/** Phase of internal LFO */
	float phase = 0.f;
	int index = 0;
	bool gates[8] = {};
	
	const int chromaticScale[13] = {0, 1, 2, 3, 4, 5,  6,  7,  8,  9, 10, 11, 12};
	const int diatonicScale[13] =  {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21};

	YASeq3() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override {
		for (int i = 0; i < 8; i++) {
			gates[i] = true;
		}
	}

	void onRandomize() override {
		for (int i = 0; i < 8; i++) {
			gates[i] = (randomUniform() > 0.5f);
		}
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// gates
		json_t *gatesJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_array_insert_new(gatesJ, i, json_integer((int) gates[i]));
		}
		json_object_set_new(rootJ, "gates", gatesJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// gates
		json_t *gatesJ = json_object_get(rootJ, "gates");
		if (gatesJ) {
			for (int i = 0; i < 8; i++) {
				json_t *gateJ = json_array_get(gatesJ, i);
				if (gateJ)
					gates[i] = !!json_integer_value(gateJ);
			}
		}
	}

	void setIndex(int index) {
		int numSteps = (int) clamp(roundf(params[STEPS_PARAM].value + inputs[STEPS_INPUT].value), 1.0f, 8.0f);
		phase = 0.f;
		this->index = index;
		if (this->index >= numSteps)
			this->index = 0;
	}

	void step() override {
		// Run
		if (runningTrigger.process(params[RUN_PARAM].value)) {
			running = !running;
		}

		bool gateIn = false;
		if (running) {
			if (inputs[EXT_CLOCK_INPUT].active) {
				// External clock
				if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) {
					setIndex(index + 1);
				}
				gateIn = clockTrigger.isHigh();
			}
			else {
				// Internal clock
				float clockTime = powf(2.0f, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
				phase += clockTime * engineGetSampleTime();
				if (phase >= 1.0f) {
					setIndex(index + 1);
				}
				gateIn = (phase < 0.5f);
			}
		}

		// Reset
		if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) {
			setIndex(0);
		}

		// Gate buttons
		for (int i = 0; i < 8; i++) {
			if (gateTriggers[i].process(params[GATE_PARAM + i].value)) {
				gates[i] = !gates[i];
			}
			outputs[GATE_OUTPUT + i].value = (running && gateIn && i == index && gates[i]) ? 10.0f : 0.0f;
			lights[GATE_LIGHTS + i].setBrightnessSmooth((gateIn && i == index) ? (gates[i] ? 1.f : 0.33) : (gates[i] ? 0.66 : 0.0));
		}

		// Outputs
/* figure out how to loopy loop enumerated enums */

		int rowSetting = round(params[ROW1_PARAM + index].value + 0.1);
		if (params[MODE_PARAM + 0].value > 1.1f) {		/* infinite */
			outputs[ROW1_OUTPUT].value = params[ROW1_PARAM + index].value * ((params[RANGE_PARAM + 0].value > 0.5f) ? 1.0f : 0.1f);
		}
		else if (params[MODE_PARAM + 0].value < 0.9f) {	/* diatonic */
			outputs[ROW1_OUTPUT].value = chromaticScale[rowSetting] / 12.0f;
		}
		else {											/* chromatic */
			outputs[ROW1_OUTPUT].value = diatonicScale[rowSetting] / 12.0f;
		}

		rowSetting = round(params[ROW2_PARAM + index].value + 0.1);
		if (params[MODE_PARAM + 1].value > 1.1f) {		/* infinite */
			outputs[ROW2_OUTPUT].value = params[ROW2_PARAM + index].value * ((params[RANGE_PARAM + 1].value > 0.5f) ? 1.0f : 0.1f);
		}
		else if (params[MODE_PARAM + 1].value < 0.9f) {	/* diatonic */
			outputs[ROW2_OUTPUT].value = chromaticScale[rowSetting] / 12.0f;
		}
		else {											/* chromatic */
			outputs[ROW2_OUTPUT].value = diatonicScale[rowSetting] / 12.0f;
		}

		rowSetting = round(params[ROW3_PARAM + index].value + 0.1);
		if (params[MODE_PARAM + 2].value > 1.1f) {		/* infinite */
			outputs[ROW3_OUTPUT].value = params[ROW3_PARAM + index].value * ((params[RANGE_PARAM + 2].value > 0.5f) ? 1.0f : 0.1f);
		}
		else if (params[MODE_PARAM + 2].value < 0.9f) {	/* diatonic */
			outputs[ROW3_OUTPUT].value = chromaticScale[rowSetting] / 12.0f;
		}
		else {											/* chromatic */
			outputs[ROW3_OUTPUT].value = diatonicScale[rowSetting] / 12.0f;
		}

		outputs[GATES_OUTPUT].value = (gateIn && gates[index]) ? 10.0f : 0.0f;
		lights[RUNNING_LIGHT].value = (running);
		lights[RESET_LIGHT].setBrightnessSmooth(resetTrigger.isHigh());
//		lights[GATES_LIGHT].setBrightnessSmooth(gateIn);
//		lights[ROW_LIGHTS].value = outputs[ROW1_OUTPUT].value / 10.0f;
//		lights[ROW_LIGHTS + 1].value = outputs[ROW2_OUTPUT].value / 10.0f;
//		lights[ROW_LIGHTS + 2].value = outputs[ROW3_OUTPUT].value / 10.0f;
	}
};

/* from Playground MyModule */
struct x13SlidePot : SVGSlider {	/* 13 steps 6 px per = 78 tall, so */
	x13SlidePot() {
		maxHandlePos = Vec(-1.65, 0);// .plus(margin);
		minHandlePos = Vec(-1.65, 72);// .plus(margin);
		setSVGs(SVG::load(assetPlugin(plugin, "res/x13SlidePot.svg")), SVG::load(assetPlugin(plugin, "res/x13SlidePotHandle.svg")));
	}
};

template <typename BASE>
struct bigLight : BASE {
	bigLight() {
		this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

struct myOther2Switch : SVGSwitch, ToggleSwitch {
	myOther2Switch() {
		addFrame(SVG::load(assetPlugin(plugin, "res/myCKSS_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/myCKSS_0.svg")));
	}
};

struct my3Switch : SVGSwitch, ToggleSwitch {
	my3Switch() {
		addFrame(SVG::load(assetPlugin(plugin, "res/myCKSSThree_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/myCKSSThree_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/myCKSSThree_2.svg")));
	}
};

//	addParam(ParamWidget::create<myOther2Switch>(Vec(330, 87), module, YASeq3::RANGE + 0, 0.0, 1.0, 0.0));
//	addParam(ParamWidget::create<my3Switch>(Vec(352, 92), module, YASeq3::MODE + 0, 0.0, 2.0, 0.0));

struct YASeq3Widget : ModuleWidget {
	YASeq3Widget(YASeq3 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/YASeq3.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(0, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));

		addParam(ParamWidget::create<RoundBlackKnob>(Vec(20, 98 - 50 - 15 - 4), module, YASeq3::CLOCK_PARAM, -2.0f, 6.0f, 2.0f));

addParam(ParamWidget::create<LEDBezel>(Vec(154, 98 - 50 - 15), module, YASeq3::RUN_PARAM, 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<bigLight<GreenLight>>(Vec(154 + 2, 98 - 50 - 15 + 2), module, YASeq3::RUNNING_LIGHT));

addParam(ParamWidget::create<LEDBezel>(Vec(192, 98 - 50 - 15), module, YASeq3::RESET_PARAM, 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<bigLight<YellowLight>>(Vec(192 + 2, 98 - 50 - 15 + 2), module, YASeq3::RESET_LIGHT));


addInput(Port::create<PJ301MPort>(Vec(230, 98 - 50 - 15), Port::INPUT, module, YASeq3::RESET_INPUT));

addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(268, 98 - 50 - 15 - 4), module, YASeq3::STEPS_PARAM, 1.0f, 8.0f, 8.0f));

addInput(Port::create<PJ301MPort>(Vec(306, 98 - 50 - 15), Port::INPUT, module, YASeq3::STEPS_INPUT));

//		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(179.4f, 64.4f), module, YASeq3::GATES_LIGHT));
//		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(218.4f, 64.4f), module, YASeq3::ROW_LIGHTS));
//		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(256.4f, 64.4f), module, YASeq3::ROW_LIGHTS + 1));
//		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(295.4f, 64.4f), module, YASeq3::ROW_LIGHTS + 2));

		static const float portX[8] = {20, 58, 96, 135, 173, 212, 250, 289};
		addInput(Port::create<PJ301MPort>(Vec(64.4f - 6,  98 - 50 - 15), Port::INPUT, module, YASeq3::CLOCK_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(103.4 - 12, 98 - 50 - 15), Port::INPUT, module, YASeq3::EXT_CLOCK_INPUT));



		addOutput(Port::create<PJ301MPort>(Vec(332,  307 + 50 - 13), Port::OUTPUT, module, YASeq3::GATES_OUTPUT));

		addOutput(Port::create<PJ301MPort>(Vec(332, 132), Port::OUTPUT, module, YASeq3::ROW1_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(332, 212), Port::OUTPUT, module, YASeq3::ROW2_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(332, 292), Port::OUTPUT, module, YASeq3::ROW3_OUTPUT));
		
		addParam(ParamWidget::create<myOther2Switch>(Vec(330, 97), module, YASeq3::RANGE_PARAM + 0, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<my3Switch>(Vec(352, 102), module, YASeq3::MODE_PARAM + 0, 0.0, 2.0, 0.0));

		addParam(ParamWidget::create<myOther2Switch>(Vec(330, 97 + 80), module, YASeq3::RANGE_PARAM + 1, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<my3Switch>(Vec(352, 102 + 80), module, YASeq3::MODE_PARAM + 1, 0.0, 2.0, 0.0));

		addParam(ParamWidget::create<myOther2Switch>(Vec(330, 97 + 160), module, YASeq3::RANGE_PARAM + 2, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<my3Switch>(Vec(352, 102 + 160), module, YASeq3::MODE_PARAM + 2, 0.0, 2.0, 0.0));

		for (int i = 0; i < 8; i++) {
			addParam(ParamWidget::create<x13SlidePot>(Vec(portX[i]-2 + 10, 157 - 80), module, YASeq3::ROW1_PARAM + i, 0.0f, 12.0f, 0.0f));
			addParam(ParamWidget::create<x13SlidePot>(Vec(portX[i]-2 + 10, 198 - 40), module, YASeq3::ROW2_PARAM + i, 0.0f, 12.0f, 0.0f));
			addParam(ParamWidget::create<x13SlidePot>(Vec(portX[i]-2 + 10, 240 - 0), module, YASeq3::ROW3_PARAM + i, 0.0f, 12.0f, 0.0f));

/**/			addParam(ParamWidget::create<LEDButton>(Vec(portX[i]+2, 278-1 + 50 - 7), module, YASeq3::GATE_PARAM + i, 0.0f, 1.0f, 0.0f));
/**/			addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(portX[i]+6.4f, 281.4f + 50 - 7), module, YASeq3::GATE_LIGHTS + i));
/**/			addOutput(Port::create<PJ301MPort>(Vec(portX[i]-1, 307 + 50 - 13), Port::OUTPUT, module, YASeq3::GATE_OUTPUT + i));
		}
	}
};

} // namespace rack_plugin_alto777_LFSR

using namespace rack_plugin_alto777_LFSR;

RACK_PLUGIN_MODEL_INIT(alto777_LFSR, YASeq3) {
   Model *modelYASeq3 = Model::create<YASeq3, YASeq3Widget>("alto777_LFSR", "YASeq3", "Yet Another SEQ3", SEQUENCER_TAG);
   return modelYASeq3;
}
