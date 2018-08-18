#include "LFSR.hpp"
#include "dsp/digital.hpp"

#include "TriadexEngine.hpp"

namespace rack_plugin_alto777_LFSR {

struct Amuse : Module {
	enum ParamIds {

/* 8 40 step slide fader switches */
		ENUMS(SLIDEPOT_PARAM, 8),
		RATE_PARAM,

		RUN_HOLD_PARAM,
		STEP_PARAM,
		RESET_BUTTON_PARAM,

		TRIG_EDGES_PARAM,
		REST_NORMAL_PARAM,
		DELTA_ALL_PARAM,

		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_CV_INPUT,
		EXT_CLOCK_INPUT,

		RUN_HOLD_INPUT,
		RESET_INPUT,

		NUM_INPUTS
	};
	enum OutputIds {
		NOTE_OUTPUT,	/* as tempered CV 0-3 volts */
		TRIGGER_OUTPUT,	/* 1ms pulse on clock edges (now configurable) */
		CLOCK_OUTPUT,	/* from the synthetic clock */

		NUM_OUTPUTS
	};
	enum LightIds {
		RUN_HOLD_LAMP,
		RESET_LAMP,
		
		STEP_LAMP,
/* Triadex state machine lamps */
		ENUMS(TRIADEX_LAMP, 40),	
				
		NUM_LIGHTS
	};

/* clock edge detector and Triadex test state */
	bool clockState = false;	/* we watch the clock for edges, this is state for that */
	bool runnable = false;		/* don't until we see a reset or intializing */
//	bool wasRunning = false;
//	bool gateIn = false;

	PulseGenerator resetPulse;
	SchmittTrigger resetTrigger;

	bool running = true; 	
	SchmittTrigger runningTrigger;

	SchmittTrigger resetButtonTrigger;
	SchmittTrigger resetInputTrigger;

	PulseGenerator triggerPulse;

/* Triadex control vectors */
	int interval[4];
	int theme[4];

	float phase = 0.0;
	float blinkPhase = 0.0;
	SchmittTrigger clockTrigger;
	
	TriadexEngine theTriadex;
	
	float previousNote = 0.123456f;

	Amuse() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	void onReset() override {
		theTriadex.reset();
		theTriadex.setIntervalAndTheme(interval, theme);
	}

	void onCreate() override {
		theTriadex.reset();
		theTriadex.setIntervalAndTheme(interval, theme);
	}

	void myInit() {
//		theTriadex.reset();
		theTriadex.setIntervalAndTheme(interval, theme);

		runnable = true;
	}

/**/
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// state
		json_t *state = json_array();
		int temp[4];
		theTriadex.getTriadexState(temp);
		for (int i = 0; i < 4; i++) {
			json_array_insert_new(state, i, json_integer((int) temp[i]));
		}
		json_object_set_new(rootJ, "state", state);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// gates
		json_t *state = json_object_get(rootJ, "state");
		if (state) {
			int temp[4];
			for (int i = 0; i < 4; i++) {
				json_t *wha = json_array_get(state, i);
				if (wha)
					temp[i] = json_integer_value(wha);
			}
			theTriadex.setTriadexState(temp);
		}
	}
/**/
};

struct tinyLEDButton : SVGSwitch, MomentarySwitch {
	tinyLEDButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/tinyLEDButton.svg")));
	}
};

template <typename BASE>
struct bigLight : BASE {
	bigLight() {
		this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

struct my2Switch : SVGSwitch, ToggleSwitch {
	my2Switch() {
		addFrame(SVG::load(assetPlugin(plugin, "res/togSwitch0ff.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/togSwitch0n.svg")));
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

void Amuse::step() {
	if (!runnable) myInit();
	
//	bool running = (params[RUN_HOLD_PARAM].value > 0.0f) && (!(inputs[RUN_HOLD_INPUT].value > 0.0f));

	if (runningTrigger.process(params[RUN_HOLD_PARAM].value))
		running = !running;

	if (inputs[RUN_HOLD_INPUT].active)
		running = inputs[RUN_HOLD_INPUT].value <= 0.0f;

	lights[RUN_HOLD_LAMP].value = running ? 1.0f : 0.0;
	
/*
	if (!wasRunning && running) phase = 0.0;
	if (wasRunning && !running) clockState = 0;

	wasRunning = running;
*/	
	bool zeroEm = false;
	if (resetButtonTrigger.process(params[RESET_BUTTON_PARAM].value) || resetInputTrigger.process(inputs[RESET_INPUT].value))
		zeroEm = true;
	float deltaTime = 1.0 / engineGetSampleRate();

//	bool clockState persistent...
	bool risingEdge = 0;
	bool fallingEdge = 0;
	bool stepTime = 0;

	if (zeroEm) {
		theTriadex.reset();
		resetPulse.trigger(0.166);
	}

	lights[RESET_LAMP].value = resetPulse.process(deltaTime) ? 1.0f : 0.0;		

	bool syntheticClock = false;
	if (running) {
		if (inputs[EXT_CLOCK_INPUT].active) {
			// External clock
			if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value))
				phase = 0.0;
			syntheticClock = clockTrigger.isHigh();
		}
		else {
			// Internal clock
			float clockTime = powf(2.0f, params[RATE_PARAM].value + inputs[CLOCK_CV_INPUT].value);
			phase += clockTime * engineGetSampleTime();
			if (phase >= 1.0)
				phase -= 1.0;
			syntheticClock = (phase < 0.5f);
		}
	}
	else {	/* not running */
//		clockState = 0;
		syntheticClock = params[STEP_PARAM].value > 0.0;
		lights[STEP_LAMP].value = syntheticClock ? 1.0 : 0.0;
	}

	if (syntheticClock) {
		if (!clockState) {
			fallingEdge = 1;
			clockState = 1;
		}
	}
	else {
		if (clockState) {
			risingEdge = 1;
			clockState = 0;
		}	
	}

	stepTime = risingEdge || fallingEdge;

/* load the control vectors */
	for (int i = 0; i < 4; i++) {
		theme[i] = params[SLIDEPOT_PARAM + i].value + 0.05;
		interval[i] = params[SLIDEPOT_PARAM + 7 - i].value + 0.05;
	}

if (runnable) {		/* just my kludgy one-shot */

	if (stepTime) {
		theTriadex.halfStep();
		float theNote = theTriadex.getNote();
		bool doTrigger = 0;
		if (params[DELTA_ALL_PARAM].value > 0.0f) doTrigger = 1;	/* but still on edges */
		else doTrigger = fabs(theNote - previousNote) > 0.01f;
		
		if (doTrigger) {
			if (params[TRIG_EDGES_PARAM].value > 1.1f) doTrigger = fallingEdge;
			else if (params[TRIG_EDGES_PARAM].value < 0.9f) doTrigger = risingEdge; 
			else doTrigger = 1;
		}
		
		if ((theNote < 0.01) && params[REST_NORMAL_PARAM].value <= 0.0f) doTrigger = 0;
		else {		/* leave trigger unchanged but for sure update the note */
			outputs[NOTE_OUTPUT].value = theNote;
			previousNote = theNote;
		}

		if (doTrigger)
			triggerPulse.trigger(0.001);		/* std 1 mS */	
	}
}	
	outputs[TRIGGER_OUTPUT].value =  triggerPulse.process(deltaTime) ? 10.0f : 0.0;
	outputs[CLOCK_OUTPUT].value =  clockState ? 10.0f : 0.0;

/* curious */
	for (int i = 0; i < 39; i++) lights[TRIADEX_LAMP + i].value = theTriadex.bitValue(i);
}

struct rectangularLEDWidget : ModuleLightWidget {
	rectangularLEDWidget() {
		bgColor = nvgRGB(0x10, 0x10, 0x10);
		borderColor = nvgRGBA(0x20, 0x20, 0x20, 0xc0);
	}

	void drawLight(NVGcontext *) override;
};

/* the (now) easy part, draw the LED as specified */

void rectangularLEDWidget::drawLight(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);

	nvgFillColor(vg, bgColor);
	nvgFill(vg);

	nvgFillColor(vg, color);
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgRect(vg, 0.25, 0.25, box.size.x - 0.5, box.size.y - 0.5);
	nvgStrokeColor(vg, borderColor);
	nvgStrokeWidth(vg, 0.5);
	nvgStroke(vg);
}

template <typename BASE>
struct rectangularTriadexLight : BASE {
	rectangularTriadexLight() {
		this->box.size = mm2px(Vec(2.367, 1.463));		/* golden! */
	}
};

struct redRectangularLED : rectangularLEDWidget {
	redRectangularLED() {
		addBaseColor(nvgRGB(0xff, 0x0, 0x0));
	}
};

struct yellowRectangularLED : rectangularLEDWidget {
	yellowRectangularLED() {
		addBaseColor(nvgRGB(0xff, 0xff, 0x0));
	}
};

struct greenRectangularLED : rectangularLEDWidget {
	greenRectangularLED() {
		addBaseColor(nvgRGB(0x0, 0xff, 0x0));
	}
};

struct blueRectangularLED : rectangularLEDWidget {
	blueRectangularLED() {
		addBaseColor(nvgRGB(0x40, 0x40, 0xff));
	}
};


struct MyBSlidePot : SVGSlider {
	MyBSlidePot() {
		snap = true;
		maxHandlePos = mm2px(Vec(-0.25, 0.4));	/* was also -0.56 */
//		minHandlePos = mm2px(Vec(-0.56, 105.84));
		minHandlePos = mm2px(Vec(-0.25, 106.1));		/* just look */
		setSVGs(SVG::load(assetPlugin(plugin, "res/MyBSlidePot.svg")),
		  SVG::load(assetPlugin(plugin, "res/MyBSlidePotHandle.svg")));
	}
};

struct HexCapScrew : SVGScrew {
	HexCapScrew() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/HexCapScrew.svg")));
		box.size = sw->box.size;
	}
};

struct AmuseWidget : ModuleWidget {
  AmuseWidget(Amuse *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Amuse.svg")));

/* screw this! */
	addChild(Widget::create<HexCapScrew>(Vec(0, 0)));
	addChild(Widget::create<HexCapScrew>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<HexCapScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<HexCapScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

/* clock section */
	addParam(ParamWidget::create<RoundBlackKnob>(mm2px(Vec(1.934, 8.521)), module, Amuse::RATE_PARAM, -2.0f, 3.0f, 0.5f));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.756, 23.696)), Port::INPUT, module, Amuse::CLOCK_CV_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(17.47, 23.696)), Port::INPUT, module, Amuse::EXT_CLOCK_INPUT));

/* output */
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(2.756, 89.925 - 9.0)), Port::OUTPUT, module, Amuse::NOTE_OUTPUT));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(2.756, 102.103 - 4.5)), Port::OUTPUT, module, Amuse::TRIGGER_OUTPUT));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(2.756, 114.279 - 0.0)), Port::OUTPUT, module, Amuse::CLOCK_OUTPUT));

/* 40 lamps for Triadex state */
	float lampX = 96.71 - 27.0;
	float lampY = 117.68 + 0.25;
	float bulbSize = 2.7053;
	for (int i = 0; i < 31; i++) 
		addChild(ModuleLightWidget::create<rectangularTriadexLight<blueRectangularLED>>(mm2px(Vec(lampX, lampY - bulbSize * i)), module, Amuse::TRIADEX_LAMP + i));

	for (int i = 31; i < 33; i++)
		addChild(ModuleLightWidget::create<rectangularTriadexLight<yellowRectangularLED>>(mm2px(Vec(lampX, lampY - bulbSize * i)), module, Amuse::TRIADEX_LAMP + i));

	for (int i = 33; i < 38; i++)
		addChild(ModuleLightWidget::create<rectangularTriadexLight<greenRectangularLED>>(mm2px(Vec(lampX, lampY - bulbSize * i)), module, Amuse::TRIADEX_LAMP + i));

	for (int i = 38; i < 40; i++)
		addChild(ModuleLightWidget::create<rectangularTriadexLight<redRectangularLED>>(mm2px(Vec(lampX, lampY - bulbSize * i)), module, Amuse::TRIADEX_LAMP + i));

/* 8 40-step slide faders for interval and theme selection */
	float sliderX = 110.0 - 17.246 - 27.0; float sliderSpace = 4.06; float tween = 4.06;
	for (int i = 0; i < 4; i++)
		addParam(ParamWidget::create<MyBSlidePot>(mm2px(Vec(sliderX - sliderSpace * i, 11.84)), module, Amuse::SLIDEPOT_PARAM + i, 0.0, 39.0, 39.0));

	for (int i = 4; i < 8; i++)
		addParam(ParamWidget::create<MyBSlidePot>(mm2px(Vec(sliderX - sliderSpace * i - tween, 11.84)), module, Amuse::SLIDEPOT_PARAM + i, 0.0, 39.0, 39.0));

//	addParam(ParamWidget::create<my2Switch>(mm2px(Vec(3.231, 48.263)), module, Amuse::RUN_HOLD_PARAM, 0.0, 1.0, 0.0));
//	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.756, 59.888)), Port::INPUT, module, Amuse::RUN_HOLD_INPUT));
	
	
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(3.184, 48.69)), module, Amuse::RUN_HOLD_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<bigLight<RedLight>>(mm2px(Vec(3.934, 49.44)), module, Amuse::RUN_HOLD_LAMP));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.756, 59.888)), Port::INPUT, module, Amuse::RUN_HOLD_INPUT));

	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(17.898, 48.69)), module, Amuse::RESET_BUTTON_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<bigLight<RedLight>>(mm2px(Vec(18.648, 49.44)), module, Amuse::RESET_LAMP));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(17.47, 59.888)), Port::INPUT, module, Amuse::RESET_INPUT));

	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(17.898, 9.772)), module, Amuse::STEP_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<bigLight<RedLight>>(mm2px(Vec(18.648, 10.522)), module, Amuse::STEP_LAMP));

	addParam(ParamWidget::create<myOther2Switch>(mm2px(Vec(19.796, 91.372 - 9.0)), module, Amuse::DELTA_ALL_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<my3Switch>(mm2px(Vec(19.793, 102.374 - 4.5)), module, Amuse::TRIG_EDGES_PARAM, 0.0, 2.0, 1.0));
	addParam(ParamWidget::create<myOther2Switch>(mm2px(Vec(19.796, 115.727 - 0.0)), module, Amuse::REST_NORMAL_PARAM, 0.0, 1.0, 0.0));
  }
};

} // namespace rack_plugin_alto777_LFSR

using namespace rack_plugin_alto777_LFSR;

RACK_PLUGIN_MODEL_INIT(alto777_LFSR, Amuse) {
   Model *modelAmuse = Model::create<Amuse, AmuseWidget>("alto777_LFSR", "Amuse", "Amuse", SEQUENCER_TAG);
   return modelAmuse;
}
