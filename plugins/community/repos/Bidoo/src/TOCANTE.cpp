#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct TOCANTE : Module {
	enum ParamIds {
		BPM_PARAM,
		BPMFINE_PARAM,
		BEATS_PARAM,
		REF_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_MEASURE,
		OUT_BEAT,
		OUT_TRIPLET,
		OUT_QUARTER,
		OUT_EIGHTH,
		OUT_SIXTEENTH,
		NUM_OUTPUTS
	};
	enum LightIds {
		RUNNING_LIGHT,
		NUM_LIGHTS
	};

	int ref = 2;
	int beats = 1;
	int currentStep = 0;
	int stepsPerMeasure = 1;
	int stepsPerBeat = 1;
	int stepsPerSixteenth = 1;
	int stepsPerEighth = 1;
	int stepsPerQuarter = 1;
	int stepsPerTriplet = 1;
	PulseGenerator gatePulse;
	PulseGenerator gatePulse_triplets;
	SchmittTrigger runningTrigger;
	SchmittTrigger resetTrigger;
	bool running = true;
	bool reset = false;
	float runningLight = 0.0f;
	bool pulseEven = false, pulseTriplets = false;
	float bpm = 0.0f;

	TOCANTE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	}

	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "running", json_boolean(running));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ){
			running = json_is_true(runningJ);
		}
}

};

void TOCANTE::step() {
	if (runningTrigger.process(params[RUN_PARAM].value)) {
		running = !running;
		if(running){
			currentStep = 0;
		}
	}

	if (resetTrigger.process(params[RESET_PARAM].value)){
		currentStep = 0;
	}

	ref = clamp(powf(2.0f,params[REF_PARAM].value),2.0f,16.0f);
	beats = clamp(params[BEATS_PARAM].value,1.0f,32.0f);
	bpm = clamp(round(params[BPM_PARAM].value) + round(100*params[BPMFINE_PARAM].value)/100, 1.0f, 350.0f);
	stepsPerSixteenth =  floor(engineGetSampleRate() / bpm * 60 * ref / 32) * 2;
	stepsPerEighth = stepsPerSixteenth * 2;
	stepsPerQuarter = stepsPerEighth * 2;
	stepsPerTriplet = floor(stepsPerQuarter/3);
	stepsPerBeat = stepsPerSixteenth * 16 / ref;
	stepsPerMeasure = beats*stepsPerBeat;

	if ((stepsPerSixteenth>0) && ((currentStep%stepsPerSixteenth) == 0)) {
		gatePulse.trigger(10 / engineGetSampleRate());
	}

	if ((stepsPerTriplet>0) && ((currentStep%stepsPerTriplet) == 0) && (currentStep <= (stepsPerMeasure-100))) {
		gatePulse_triplets.trigger(10 / engineGetSampleRate());
	}

	pulseEven = gatePulse.process(1 / engineGetSampleRate());
	pulseTriplets = gatePulse_triplets.process(1 / engineGetSampleRate());

	outputs[OUT_MEASURE].value = (currentStep == 0) ? 10.0f : 0.0f;
	outputs[OUT_BEAT].value = (pulseEven && (currentStep%stepsPerBeat == 0)) ? 10.0f : 0.0f;
	outputs[OUT_TRIPLET].value = (pulseTriplets && (currentStep%stepsPerTriplet == 0)) ? 10.0f : 0.0f;
	outputs[OUT_QUARTER].value = (pulseEven && (currentStep%stepsPerQuarter == 0)) ? 10.0f : 0.0f;
	outputs[OUT_EIGHTH].value = (pulseEven && (currentStep%stepsPerEighth == 0)) ? 10.0f : 0.0f;
	outputs[OUT_SIXTEENTH].value = (pulseEven && (currentStep%stepsPerSixteenth == 0)) ? 10.0f : 0.0f;
	if (running) {
		currentStep = floor((currentStep + 1)%stepsPerMeasure);
	}
	lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;
}

struct TOCANTEDisplay : TransparentWidget {
	TOCANTE *module;
	std::shared_ptr<Font> font;

	TOCANTEDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		char tBPM[128],tBeats[128];
		snprintf(tBPM, sizeof(tBPM), "%1.2f BPM", module->bpm);
		snprintf(tBeats, sizeof(tBeats), "%1i/%1i", module->beats, module->ref);
		nvgFontSize(vg, 16.0f);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2.0f);
		nvgFillColor(vg, YELLOW_BIDOO);
		nvgText(vg, 0.0f, 0.0f, tBPM, NULL);
		nvgText(vg, 0.0f, 15.0f, tBeats, NULL);
	}
};

struct BPMBlueKnob : BidooBlueKnob {
	void onChange (EventChange &e) override {
		BidooBlueKnob::onChange(e);
	}
};

struct TOCANTEWidget : ModuleWidget {
	TOCANTEWidget(TOCANTE *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/TOCANTE.svg")));

		TOCANTEDisplay *display = new TOCANTEDisplay();
		display->module = module;
		display->box.pos = Vec(16.0f, 55.0f);
		display->box.size = Vec(50.0f, 85.0f);
		addChild(display);

		addParam(ParamWidget::create<LEDButton>(Vec(44, 140), module, TOCANTE::RUN_PARAM, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(50, 145), module, TOCANTE::RUNNING_LIGHT));
		addParam(ParamWidget::create<BlueCKD6>(Vec(39, 180), module, TOCANTE::RESET_PARAM, 0.0, 1.0, 0.0));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<BPMBlueKnob>(Vec(5.0f,90.0f), module, TOCANTE::BPM_PARAM, 1.0f, 350.0f, 60.0f));
		addParam(ParamWidget::create<BPMBlueKnob>(Vec(5.0f,125.0f), module, TOCANTE::BPMFINE_PARAM, 0.0f, 0.99f, 0.0f));
		addParam(ParamWidget::create<BidooBlueSnapKnob>(Vec(38.0f,90.0f), module, TOCANTE::BEATS_PARAM, 1.0f, 32.0f, 4.0f));
		addParam(ParamWidget::create<BidooBlueSnapKnob>(Vec(72.0f,90.0f), module, TOCANTE::REF_PARAM, 1.0f, 4.0f, 2.0f));

		addOutput(Port::create<PJ301MPort>(Vec(18.0f, 230.0f), Port::OUTPUT, module, TOCANTE::OUT_MEASURE));
		addOutput(Port::create<PJ301MPort>(Vec(62.0f, 230.0f), Port::OUTPUT, module, TOCANTE::OUT_BEAT));
		addOutput(Port::create<PJ301MPort>(Vec(18.0f, 280.0f), Port::OUTPUT, module, TOCANTE::OUT_QUARTER));
		addOutput(Port::create<PJ301MPort>(Vec(62.0f, 280.0f), Port::OUTPUT, module, TOCANTE::OUT_TRIPLET));
		addOutput(Port::create<PJ301MPort>(Vec(18.0f, 331.0f), Port::OUTPUT, module, TOCANTE::OUT_EIGHTH));
		addOutput(Port::create<PJ301MPort>(Vec(62.0f, 331.0f), Port::OUTPUT, module, TOCANTE::OUT_SIXTEENTH));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, TOCANTE) {
   Model *modelTOCANTE = Model::create<TOCANTE, TOCANTEWidget>("Bidoo", "tOCAnTe", "tOCAnTe clock", CLOCK_TAG);
   return modelTOCANTE;
}
