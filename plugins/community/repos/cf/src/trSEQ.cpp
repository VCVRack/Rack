#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct trSEQ : Module {
	enum ParamIds {
		CLOCK_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		NOTESIN_PARAM,
		CLEAR_PARAM,
		STEPS_PARAM,
		GATE_PARAM = STEPS_PARAM + 16,
		NUM_PARAMS = GATE_PARAM + 16
	};
	enum InputIds {
		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		NOTESIN_INPUT,
		CLEAR_INPUT,
		STEPS_INPUT,
		GATE_INPUT,
		NUM_INPUTS = GATE_INPUT+16
	};
	enum OutputIds {
		GATES_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		RUNNING_LIGHT,
		RESET_LIGHT,
		GATES_LIGHT,
		GATE_LIGHTS,
		NUM_LIGHTS = GATE_LIGHTS + 16
	};

	bool running = true;
	SchmittTrigger clockTrigger; // for external clock
	// For buttons
	SchmittTrigger runningTrigger;
	SchmittTrigger resetTrigger;
	SchmittTrigger gateTriggers[32];

	float phase = 0.0;
	int index = 0;
	bool gateState[16] = {};
	float resetLight = 0.0;
	float stepLights[16] = {};

	enum GateMode {
		TRIGGER,
		RETRIGGER,
		CONTINUOUS,
	};
	GateMode gateMode = TRIGGER;
	PulseGenerator gatePulse;

	trSEQ() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {onReset();}
	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// gates
		json_t *gatesJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_t *gateJ = json_integer((int) gateState[i]);
			json_array_append_new(gatesJ, gateJ);
		}
		json_object_set_new(rootJ, "gates", gatesJ);

		// gateMode
		json_t *gateModeJ = json_integer((int) gateMode);
		json_object_set_new(rootJ, "gateMode", gateModeJ);

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
			for (int i = 0; i < 16; i++) {
				json_t *gateJ = json_array_get(gatesJ, i);
				if (gateJ)
					gateState[i] = !!json_integer_value(gateJ);
			}
		}

		// gateMode
		json_t *gateModeJ = json_object_get(rootJ, "gateMode");
		if (gateModeJ)
			gateMode = (GateMode)json_integer_value(gateModeJ);
	}

	void onReset() override {
		for (int i = 0; i < 16; i++) {
			gateState[i] = false;
		}
	}

	void onRandomize() override {
		for (int i = 0; i < 16; i++) {
			gateState[i] = (randomUniform() > 0.5);
		}
	}
};


void trSEQ::step() {
	const float lightLambda = 0.075;
	// Run
	if (runningTrigger.process(params[RUN_PARAM].value)) {
		running = !running;
	}
	lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;

	bool nextStep = false;


	if (running) {
		if (inputs[EXT_CLOCK_INPUT].active) {
			// External clock
			if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) {
				phase = 0.0;
				nextStep = true;
			}
		}
		else {
			// Internal clock
			float clockTime = powf(2.0, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
			phase += clockTime / engineGetSampleRate();
			if (phase >= 1.0) {
				phase -= 1.0;
				nextStep = true;
			}
		}
	}

	// Reset
	if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) {
		phase = 0.0;
		index = 16;
		nextStep = true;
		resetLight = 1.0;
	}

	if (nextStep) {
		// Advance step
		int numSteps = clamp(static_cast<int>(round(params[STEPS_PARAM].value + inputs[STEPS_INPUT].value)), 1, 16);

		index += 1;

		if (index >= numSteps) {
			index = 0;
		}
      
      if (!inputs[NOTESIN_INPUT].active) inputs[NOTESIN_INPUT].value = 0;
      if (!inputs[CLEAR_INPUT].active) inputs[CLEAR_INPUT].value = 0;
#define or ||
      if (params[NOTESIN_PARAM].value or inputs[NOTESIN_INPUT].value>0) gateState[index] = true;
      if (params[CLEAR_PARAM].value or inputs[CLEAR_INPUT].value>0) gateState[index] = false;
#undef or

		stepLights[index] = 1.0;
		gatePulse.trigger(1e-3);
	}

	resetLight -= resetLight / lightLambda / engineGetSampleRate();

	bool pulse = gatePulse.process(1.0 / engineGetSampleRate());

	// Gate buttons
	for (int i = 0; i < 16; i++) {


		if (gateTriggers[i+16].process(inputs[GATE_INPUT + i].value)) {
			gateState[i] = !gateState[i];
		}

		if (gateTriggers[i].process(params[GATE_PARAM + i].value)) {
			gateState[i] = !gateState[i];
		}

		bool gateOn = (running && i == index && gateState[i]);
		if (gateMode == TRIGGER)
			gateOn = gateOn && pulse;
		else if (gateMode == RETRIGGER)
			gateOn = gateOn && !pulse;

		//outputs[GATE_OUTPUT + i].value = gateOn ? 10.0 : 0.0;
		stepLights[i] -= stepLights[i] / lightLambda / engineGetSampleRate();
		lights[GATE_LIGHTS + i].value = gateState[i] ? 1.0 - stepLights[i] : stepLights[i];
	}

	// Rows

	bool gatesOn = (running && gateState[index]);
	if (gateMode == TRIGGER)
		gatesOn = gatesOn && pulse;
	else if (gateMode == RETRIGGER)
		gatesOn = gatesOn && !pulse;

	// Outputs

	outputs[GATES_OUTPUT].value = gatesOn ? 10.0 : 0.0;
	lights[RESET_LIGHT].value = resetLight;
	lights[GATES_LIGHT].value = gatesOn ? 1.0 : 0.0;
	}

struct PadButton : SVGSwitch, MomentarySwitch {
	PadButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/PadButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/PadButtonDown.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};


struct trSEQWidget : ModuleWidget {
	trSEQWidget(trSEQ *module);
	Menu *createContextMenu() override;
};

trSEQWidget::trSEQWidget(trSEQ *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/trSEQ.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(18, 56), module, trSEQ::CLOCK_PARAM, -2.0f, 6.0f, 2.0f));
	addParam(ParamWidget::create<LEDButton>(Vec(60, 61-1), module, trSEQ::RUN_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(64.4, 64.4), module, trSEQ::RUNNING_LIGHT));
	addParam(ParamWidget::create<LEDButton>(Vec(99, 61-1), module, trSEQ::RESET_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(103.4, 64.4), module, trSEQ::RESET_LIGHT));
	addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(132, 56), module, trSEQ::STEPS_PARAM, 1.0f, 16.0f, 16.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(289.4, 64.4), module, trSEQ::GATES_LIGHT));

	static const float portX[8] = {20, 58, 96, 135, 173, 212, 250, 289};


	addParam(ParamWidget::create<PadButton>(Vec(portX[5]-26, 56), module, trSEQ::NOTESIN_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<PadButton>(Vec(portX[6]-26, 56), module, trSEQ::CLEAR_PARAM, 0.0f, 1.0f, 0.0f));
	addInput(Port::create<PJ301MPort>(Vec(portX[5]-24, 98), Port::INPUT, module, trSEQ::NOTESIN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(portX[6]-24, 98), Port::INPUT, module, trSEQ::CLEAR_INPUT));


	addInput(Port::create<PJ301MPort>(Vec(portX[0]-1, 98), Port::INPUT, module, trSEQ::CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(portX[1]-1, 98), Port::INPUT, module, trSEQ::EXT_CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(portX[2]-1, 98), Port::INPUT, module, trSEQ::RESET_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(portX[3]-1, 98), Port::INPUT, module, trSEQ::STEPS_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(portX[7]-6.5, 98), Port::OUTPUT, module, trSEQ::GATES_OUTPUT));


	for (int i = 0; i < 16; i++) {

		addParam(ParamWidget::create<LEDButton>(Vec(i*19+12, 203-1), module, trSEQ::GATE_PARAM + i, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(i*19+16.4, 206.4), module, trSEQ::GATE_LIGHTS + i));
		addInput(Port::create<PJ301MPort>(Vec(i*19+9, 247+ 40*(i%2)), Port::INPUT, module, trSEQ::GATE_INPUT + i));
	}

}



struct trSEQGateModeItem : MenuItem {
	trSEQ *trseq;
	trSEQ::GateMode gateMode;
	void onAction(EventAction &e) override {
		trseq->gateMode = gateMode;
	}
	void step() override {
		rightText = (trseq->gateMode == gateMode) ? "✔" : "";
	}
};

Menu *trSEQWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	trSEQ *trseq = dynamic_cast<trSEQ*>(module);
	assert(trseq);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Gate Mode";
	menu->addChild(modeLabel);

	trSEQGateModeItem *triggerItem = new trSEQGateModeItem();
	triggerItem->text = "Trigger";
	triggerItem->trseq = trseq;
	triggerItem->gateMode = trSEQ::TRIGGER;
	menu->addChild(triggerItem);

	trSEQGateModeItem *retriggerItem = new trSEQGateModeItem();
	retriggerItem->text = "Retrigger";
	retriggerItem->trseq = trseq;
	retriggerItem->gateMode = trSEQ::RETRIGGER;
	menu->addChild(retriggerItem);

	trSEQGateModeItem *continuousItem = new trSEQGateModeItem();
	continuousItem->text = "Continuous";
	continuousItem->trseq = trseq;
	continuousItem->gateMode = trSEQ::CONTINUOUS;
	menu->addChild(continuousItem);

	return menu;
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, trSEQ) {
   Model *modeltrSEQ = Model::create<trSEQ, trSEQWidget>("cf", "trSEQ", "trSEQ", SEQUENCER_TAG);
   return modeltrSEQ;
}
