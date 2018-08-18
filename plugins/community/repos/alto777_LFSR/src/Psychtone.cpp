#include "LFSR.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_alto777_LFSR {

/* recreation of Don Lancaster's Psychtone Music Composer-Synthesizer (note logic only) */

/*
a 6 bit LFSR with 4 pre-wired tap selections and a output bit weighting mechanism
takes a bit of doing but isn't always dreadful 
*/

#define NUM_CHANNELS	6

struct Psychtone : Module {
	enum ParamIds {
//		PITCH_PARAM,
		
		CLOCK_PARAM,
		RUN_PARAM,
		STEP_PARAM,

		ENUMS(TUNE_SEL_PARAMS, 3),
		ENUMS(WEIGHT_PARAMS, 3),
		ENUMS(PAUSE_SEL_PARAMS, 6),
		ENUMS(LFSR_PARAM, NUM_CHANNELS),
		
		FWD_REV_PARAM,
		UP_DOWN_PARAM,

		NUM_PARAMS
	};
	enum InputIds {
//		PITCH_INPUT,


		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,

		NUM_INPUTS
	};
	enum OutputIds {
//		SINE_OUTPUT,
		
		
		GATE_OUTPUT,
		OUTPUT_OUTPUT,
/*	addOutput(Port::create<PJ301MPort>(Vec(303, 228), Port::OUTPUT, module, Psychtone::GATE_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(303, 228), Port::OUTPUT, module, Psychtone::OUTPUT_OUTPUT));

	addInput(Port::create<PJ301MPort>(Vec(42, 228), Port::INPUT, module, Psychtone::CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(88, 228), Port::INPUT, module, Psychtone::EXT_CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(134, 228), Port::INPUT, module, Psychtone::RESET_INPUT));
*/		
		
		NUM_OUTPUTS
	};
	enum LightIds {
//		BLINK_LIGHT,
		CLOCK_LIGHT,
		
		RUNNING_LIGHT,
		RESET_LIGHT,
		STEP_LIGHT,

		ENUMS(LFSR_LIGHTS, 6),
	
		NUM_LIGHTS
	};

	float phase = 0.0;
//	float blinkPhase = 0.0;
	
/* hello world global primitive info window */
	int hwCounter;
	int printValue = 0;

/* running */
	SchmittTrigger runningTrigger;
	bool running = false;

/* clock */
	SchmittTrigger clockTrigger;
	bool nextStep = false;

/* step - reset internal clock phase and step the engine. GATE? */
	SchmittTrigger stepTrigger;
	SchmittTrigger resetTrigger;

/* shift register */
	bool bit;
	unsigned int lfsrBits;
	SchmittTrigger lfsrTrigger[6];

	Psychtone() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu

	void onReset() override {
//		gateBits = LFSR_MASK;		
//		tapBits = HIGH_BIT;
		lfsrBits = 0x0;
	}
};

/* OMG your brain is fried! */
static unsigned int tuneSelectBits[12] = {
	0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001,
	0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001,
};
	
void Psychtone::step() {
/* Blink light at 1Hz
	float deltaTime = engineGetSampleTime();

	blinkPhase += deltaTime;
	if (blinkPhase >= 1.0f) blinkPhase -= 1.0f;
	lights[BLINK_LIGHT].value = (blinkPhase < 0.5f) ? 1.0f : 0.0f;
*/	

/* running */
	if (runningTrigger.process(params[RUN_PARAM].value)) {
		running = !running;
	}
	lights[RUNNING_LIGHT].value = running ? 1.0f : 0.0f;
	
/* clock */
	bool nextStep = false;
	bool gateIn = false;
	
	if (resetTrigger.process(inputs[RESET_INPUT].value)) {
		lfsrBits = 0x0;
		phase = 0;		/* seems only fair? */
	}

	if (running) {
		if (inputs[EXT_CLOCK_INPUT].active) {
			// External clock
			if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) {
				phase = 0.0;
				nextStep = true;
			}
			gateIn = clockTrigger.isHigh();
		}
		else {
			// Internal clock
			float clockTime = powf(2.0f, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
			phase += clockTime * engineGetSampleTime();
			if (phase >= 1.0f) {
				phase -= 1.0f;
				nextStep = true;
			}
			gateIn = (phase < 0.5f);
		}
	}

/* for now, just feed our gateIn to the output */
//		outputs[GATE_OUTPUT].value = gateIn ? 10.0f : 0.0f;			
		lights[CLOCK_LIGHT].value = gateIn ? 1.0f : 0.0f;
/*
returns -5.0 to 6 just fine, but still stops at max angle
setting value d0es not set physical angle

		weightedOutput += (tune[params[TUNE_SEL_PARAMS + i]] & 

*/
	if (stepTrigger.process(params[STEP_PARAM].value)) {
		nextStep = true;
		phase = 0;
		gateIn = 1;
	}

	if (nextStep) {
		if (params[FWD_REV_PARAM].value <= 0.0f)
			bit = (lfsrBits ^ (lfsrBits >> 5)) & 1;
		else
			bit = (lfsrBits ^ (lfsrBits >> 1)) & 1;

		lfsrBits >>= 1;

		if (params[UP_DOWN_PARAM].value <= 0.0f) {
			if (bit) lfsrBits |= 0x20;
		}
		else {
			if (!bit) lfsrBits |= 0x20;
		}

		lfsrBits &= 0x3f;
	}
	
	int jj = 1;
	bool gateReason = false;
	for (int i = 0; i < 6; i++, jj <<= 1) {
/*		if (gateTriggers[i].process(params[GATE_PARAM + i].value)) {
			gateBits ^= jj;
		}

		if (tapTrigger[i].process(params[TAP_PARAM + i].value))
			tapBits ^= jj;
*/
		if (lfsrTrigger[i].process(params[LFSR_PARAM + i].value))
			lfsrBits ^= jj;

		bit = !!(lfsrBits & jj);	/* mightn't need !! normalization */
//		lights[GATE_LIGHTS + i].value = (gateBits & jj) ? 0.85 : 0.0;
//		lights[TAP_LIGHTS + i].value = (tapBits & jj) ? 0.85 : 0.0;
		lights[LFSR_LIGHTS + i].value = bit ? 1.0 : 0.0;
		
		if (!gateReason)
			gateReason = ((params[PAUSE_SEL_PARAMS + i].value < 1.0) && !bit) | ((params[PAUSE_SEL_PARAMS + i].value > 1.0) && bit);
	}
	
	outputs[GATE_OUTPUT].value = (gateIn & gateReason) ? 10.0f : 0.0f;			
	lights[CLOCK_LIGHT].value = (gateIn & gateReason) ? 1.0f : 0.0f;

/* calculate output */
	float theKnob;
	int myKnobIndex;

	float weightedOutput = 0;
//...	unsigned lfsrState = (lfsrBits << 8) | (lfsrBits ^ 0xff);

//	printValue = params[RESET_PARAM].value;

/* just one knob first */
	for (int i = 0; i < 3; i++) {
		theKnob = params[TUNE_SEL_PARAMS + i].value; 
		if (theKnob >= 7.0) theKnob -= 12.0;
		if (theKnob <= -6.0) theKnob += 12.0;	
		params[TUNE_SEL_PARAMS + i].value = theKnob;
	
		theKnob += 6; if (theKnob >= 12) theKnob -= 12;

		myKnobIndex = theKnob;
		if (myKnobIndex < 6)
			weightedOutput += (tuneSelectBits[myKnobIndex] & lfsrBits) ? params[WEIGHT_PARAMS + i].value : 0.0f;
		else
			weightedOutput -= (tuneSelectBits[myKnobIndex] & (lfsrBits ^ 0x3f)) ? params[WEIGHT_PARAMS + i].value : 0.0f;
	}
	outputs[OUTPUT_OUTPUT].value = weightedOutput / 12.0f;
	
	lights[STEP_LIGHT].setBrightnessSmooth(stepTrigger.isHigh());
}

/* hello world... */
struct MyModuleDisplay : TransparentWidget {
	Psychtone *module;
	int frame = 0;
	std::shared_ptr<Font> font;
	
	MyModuleDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/Sudo.ttf"));
	}

	void draw(NVGcontext *vg) override {
		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);
		
		nvgFillColor(vg, nvgRGBA(0xe0, 0xe0, 0xff, 0x80));
		char text[128];
		snprintf(text, sizeof(text), "= %x", module->printValue);
		nvgText(vg, 1, 1, text, NULL);
//		nvgText(vg, 25, 25, text, NULL);
//		nvgText(vg, 25, 125, text, NULL);
	}
};

template <typename BASE>
struct bigLight : BASE {
	bigLight() {
		this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

/* ...hello world */
struct myOwnKnob : SVGKnob {
	myOwnKnob() {
		box.size = Vec(40, 40);
		minAngle = -0.8 * M_PI;
		maxAngle = 0.8 * M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/myOwnKnob.svg")));
//
// z'all it takes to tune this thing		snap = true;
		shadow->opacity = -1.0;
	}
};

struct myBigKnob : SVGKnob {
	myBigKnob() {
//		minAngle = -0.25 * M_PI;
//		maxAngle = 0.25 * M_PI;

		minAngle = -6.0 * M_PI;
		maxAngle = 6.0 * M_PI;

//		snap - shows large mouse travel needed
		snap = true;
		smooth = false;

		setSVG(SVG::load(assetPlugin(plugin, "res/myBigKnob.svg")));
	}
};

/* test variations for infinite but snapping knobs */
struct xBigKnob : SVGKnob {
	xBigKnob() {
		minAngle = -M_PI / 6.0;
		maxAngle = M_PI / 6.0;

		snap = true;
		smooth = false;
		setSVG(SVG::load(assetPlugin(plugin, "res/myBigKnob.svg")));
	}
};

struct yBigKnob : SVGKnob {
	yBigKnob() {
		minAngle = -2.0 * M_PI;
		maxAngle = 2.0 * M_PI;

		snap = true;
		smooth = false;

		setSVG(SVG::load(assetPlugin(plugin, "res/myBigKnob.svg")));
	}
};
/*
struct BefacoBigSnapKnob : BefacoBigKnob {
	BefacoBigSnapKnob() {
		snap = true;
		smooth = false;
	}
};
*/

struct my2Switch : SVGSwitch, ToggleSwitch {
	my2Switch() {
		addFrame(SVG::load(assetPlugin(plugin, "res/togSwitch0ff.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/togSwitch0n.svg")));
	}
};



struct PsychtoneWidget : ModuleWidget {
	PsychtoneWidget(Psychtone *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Psychtone.svg")));

/*		addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
*/
		addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(0, box.size.y - 15)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, box.size.y - 15)));

/* hello world... */	
	{
		MyModuleDisplay *display = new MyModuleDisplay();
		display->module = module;
		display->box.pos = Vec(100, 170);
//		display->box.size = Vec(box.size.x, 140);
		display->box.size = Vec(50, 180);
		addChild(display);
	}
/* ...hello world */
/* TUNE SELECT KNOBS w/ trimmer *//* not quite... */
/* snap rotary encoder */
/* my */
	addParam(ParamWidget::create<myBigKnob>(Vec(37, 51), module, Psychtone::TUNE_SEL_PARAMS, -36, 36, 6.0));
	addParam(ParamWidget::create<myOwnKnob>(Vec(37 + 18, 51 + 18), module, Psychtone::WEIGHT_PARAMS, 0.0f, 12.0f, 0.0f));

	addParam(ParamWidget::create<myBigKnob>(Vec(148, 51), module, Psychtone::TUNE_SEL_PARAMS + 1, -36, 36, 6.0));
	addParam(ParamWidget::create<myOwnKnob>(Vec(148 + 18, 51 + 18), module, Psychtone::WEIGHT_PARAMS + 1, 0.0f, 12.0f, 0.0f));

	addParam(ParamWidget::create<myBigKnob>(Vec(262, 51), module, Psychtone::TUNE_SEL_PARAMS + 2, -36, 36, 6.0));
	addParam(ParamWidget::create<myOwnKnob>(Vec(262 + 18, 51 + 18), module, Psychtone::WEIGHT_PARAMS + 2, 0.0f, 12.0f, 0.0f));

/* x
	addParam(ParamWidget::create<xBigKnob>(Vec(148, 51), module, Psychtone::TUNE_SEL_PARAMS + 1, -INFINITY, INFINITY, 0.0));
	addParam(ParamWidget::create<myOwnKnob>(Vec(148 + 18, 51 + 18), module, Psychtone::WEIGHT_PARAMS + 1, -0.5, 0.5, 0.0));

 
	addParam(ParamWidget::create<yBigKnob>(Vec(262, 51), module, Psychtone::TUNE_SEL_PARAMS + 2, -INFINITY, INFINITY, 0.0));
	addParam(ParamWidget::create<myOwnKnob>(Vec(262 + 18, 51 + 18), module, Psychtone::WEIGHT_PARAMS + 2, -0.5, 0.5, 0.0));
*/


/* the lights and pause select */

/* slide switch 3 position parameter */
	float xa = 313.0; float dx = -52.0;
	for (int i = 0; i < 6; i++, xa += dx) {
		addParam(ParamWidget::create<CKSSThree>(Vec(xa, 309), module, Psychtone::PAUSE_SEL_PARAMS + i, 0.0f, 2.0f, 1.0f));
//		addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(xa - 1, 282), module, Psychtone::LFSR_LIGHTS + i));
//		addParam(ParamWidget::create<LEDBezel>(Vec(xa, 282), module, Psychtone::LFSR_PARAM + i, 0.0, 1.0, 0.0));

		addParam(ParamWidget::create<LEDBezel>(Vec(xa - 1 - 3, 282 - 2.5), module, Psychtone::LFSR_PARAM + i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<bigLight<BlueLight>>(Vec(xa + 1.5 - 3, 284 - 2.5), module, Psychtone::LFSR_LIGHTS + i));
	}

/* input/output */
		addInput(Port::create<PJ301MPort>(Vec(42, 228), Port::INPUT, module, Psychtone::CLOCK_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(88, 228), Port::INPUT, module, Psychtone::EXT_CLOCK_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(134, 228), Port::INPUT, module, Psychtone::RESET_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(303, 228), Port::OUTPUT, module, Psychtone::GATE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(303, 228 - 44), Port::OUTPUT, module, Psychtone::OUTPUT_OUTPUT));

/* shift register control switches */

		addParam(ParamWidget::create<my2Switch>(Vec(188, 198), module, Psychtone::FWD_REV_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<my2Switch>(Vec(238, 198), module, Psychtone::UP_DOWN_PARAM, 0.0, 1.0, 1.0));



		addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(34, 179), module, Psychtone::CLOCK_PARAM, -2.0, 6.0, 2.0));

//		addParam(ParamWidget::create<LEDButton>(Vec(93, 190), module, Psychtone::RUN_PARAM, 0.0, 1.0, 0.0));
//		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(93 + 4.4, 190 + 4.4), module, Psychtone::RUNNING_LIGHT));

		addParam(ParamWidget::create<LEDBezel>(Vec(89, 187.5), module, Psychtone::RUN_PARAM, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<bigLight<GreenLight>>(Vec(91, 189.5), module, Psychtone::RUNNING_LIGHT));

		addParam(ParamWidget::create<LEDBezel>(Vec(135, 187.5), module, Psychtone::STEP_PARAM, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<bigLight<GreenLight>>(Vec(137, 189.5), module, Psychtone::STEP_LIGHT));




//		

//		addInput(Port::create<PJ301MPort>(Vec(33, 186), Port::INPUT, module, Psychtone::PITCH_INPUT));

//		addOutput(Port::create<PJ301MPort>(Vec(33, 275), Port::OUTPUT, module, Psychtone::SINE_OUTPUT));

//		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(15, 35), module, Psychtone::BLINK_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(15, 15), module, Psychtone::CLOCK_LIGHT));
	}
};

} // namespace rack_plugin_alto777_LFSR

using namespace rack_plugin_alto777_LFSR;

RACK_PLUGIN_MODEL_INIT(alto777_LFSR, Psychtone) {
   Model *modelPsychtone = Model::create<Psychtone, PsychtoneWidget>("alto777_LFSR", "Psychtone", "Psych tone", SEQUENCER_TAG);
   return modelPsychtone;
}
