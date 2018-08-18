#include "LFSR.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_alto777_LFSR {

/* 8 bit LFSR sequencer */

#define NUM_CHANNELS 8
#define LFSR_MASK	0xff
#define HIGH_BIT	0x80

struct FG8 : Module {
	enum ParamIds {
		CLOCK_PARAM,
		RUN_PARAM,
		RESET_PARAM,
//		STEPS_PARAM,
		LFSR_MODE_PARAM,
		SCALE_MODE_PARAM,		
		GATE_MODE_PARAM,

		ENUMS(TAP_PARAM, NUM_CHANNELS),
		ENUMS(ROW1_PARAM, NUM_CHANNELS),
		ENUMS(ROW2_PARAM, NUM_CHANNELS),
		ENUMS(ROW3_PARAM, NUM_CHANNELS),
		ENUMS(GATE_PARAM, NUM_CHANNELS),
		ENUMS(LFSR_PARAM, NUM_CHANNELS),
		NUM_PARAMS
	};

	enum InputIds {
		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
//		STEPS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATES_OUTPUT,
		ROW1_OUTPUT,
		ROW2_OUTPUT,
		ROW3_OUTPUT,
//		GATE_OUTPUT,
//		NUM_OUTPUTS = GATE_OUTPUT + 8
		NUM_OUTPUTS
	};

	enum LightIds {
		RUNNING_LIGHT,
		RESET_LIGHT,

		ENUMS(GATE_LIGHTS, NUM_CHANNELS),
		ENUMS(TAP_LIGHTS, NUM_CHANNELS),
		ENUMS(LFSR_LIGHTS, NUM_CHANNELS),
		NUM_LIGHTS
	};
	
	unsigned int tapBits;
	unsigned int gateBits;
	unsigned int lfsrBits;
		
	SchmittTrigger tapTrigger[NUM_CHANNELS];
	SchmittTrigger lfsrTrigger[NUM_CHANNELS];

	bool running = true;
	// for external clock
	SchmittTrigger clockTrigger;

	// For buttons
	SchmittTrigger runningTrigger;
	SchmittTrigger resetTrigger;
	SchmittTrigger gateTriggers[NUM_CHANNELS];
	
	float lastOutput[3] = {0.0, 0.0, 0.0,};

	float phase = 0.0;
// we don't index	int index = 0;
	float resetLight = 0.0;
	bool bit;

	const int chromaticScale[13] = {-6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6};
//	const int diatonicScale[13] = {-10, -8, -7, -5, -3, -1, 0, 2, 4, 5, 7, 9, 11};

	const int diatonicScale[7] = {0, 2, 4, 5, 7, 9, 11,};

//	PulseGenerator gatePulse;

	FG8() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// gates, taps, lfsr
		json_object_set_new(rootJ, "gates", json_integer(gateBits));
		json_object_set_new(rootJ, "taps", json_integer(tapBits));
		json_object_set_new(rootJ, "lfsr", json_integer(lfsrBits));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// gates, taps, lfsr
		json_t *gtlJ;

		gtlJ = json_object_get(rootJ, "gates");
		if (gtlJ) gateBits = json_integer_value(gtlJ);

		gtlJ = json_object_get(rootJ, "taps");
		if (gtlJ) tapBits = json_integer_value(gtlJ);

		gtlJ = json_object_get(rootJ, "lfsr");
		if (gtlJ) lfsrBits = json_integer_value(gtlJ);

	}

	void onReset() override {
		gateBits = LFSR_MASK;		
		tapBits = HIGH_BIT;
		lfsrBits = HIGH_BIT;
	}

	void randomize() override {
//		for (int i = 0; i < 8; i++) {
//			gateState[i] = (randomf() > 0.5);
//		}
	}
};


void FG8::step() {
	const float lightLambda = 0.075;
	// Run
	if (runningTrigger.process(params[RUN_PARAM].value)) {
		running = !running;
	}
	lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;
	
	bool galoisType = params[LFSR_MODE_PARAM].value > 0.0f;	
	bool gateType = params[GATE_MODE_PARAM].value > 0.0f;
	bool scaleType = params[SCALE_MODE_PARAM].value <= 0.0f;

	bool nextStep = false;
	bool gateIn = false;
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
			if (phase >= 1.0) {
				phase -= 1.0;
				nextStep = true;
			}
			gateIn = (phase < 0.5f);
		}
	}

	// Reset
	if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) {
		phase = 0.0;
//		index = 8;
		nextStep = true;
		resetLight = 1.0;
	}

/* maximal taps for 8 bit LFSR:
 8E 95 96 A6 AF B1 B2 B4 B8 C3 C6 D4 E1 E7 F3 FA
*/

	if (nextStep) {
/* LFSR logic, thank you Wikipedia: */
		if (galoisType) {	/* Galois LFSR calculation */
			bit = lfsrBits & 1;		/* Get LSB (i.e., the output bit). */
			lfsrBits >>= 1;			/* shift */
			if (bit)				/* apply toggle mask. */
				lfsrBits ^= tapBits;
		} else {			/* Fibonacci LFSR calculation */
			int count;				/* calculate parity <-> XOR */
			unsigned int lfsrTemp = lfsrBits & tapBits & LFSR_MASK;
			for (count = 0; lfsrTemp; count++)
				lfsrTemp &= lfsrTemp - 1;

			lfsrBits >>= 1;			/* shift */
			if (count & 1)
				lfsrBits |= HIGH_BIT;
		}
/* gatePulse.trigger(1e-3); &c. no more */
	}

	resetLight -= resetLight / lightLambda / engineGetSampleRate();

//	bool pulse = gatePulse.process(1.0 / engineGetSampleRate());

	// Gate buttons &c

	int jj = 1;
	for (int i = 0; i < 8; i++, jj <<= 1) {
		if (gateTriggers[i].process(params[GATE_PARAM + i].value)) {
			gateBits ^= jj;
		}

		if (tapTrigger[i].process(params[TAP_PARAM + i].value))
			tapBits ^= jj;

		if (lfsrTrigger[i].process(params[LFSR_PARAM + i].value))
			lfsrBits ^= jj;

		lights[GATE_LIGHTS + i].value = (gateBits & jj) ? 0.85 : 0.0;
		lights[TAP_LIGHTS + i].value = (tapBits & jj) ? 0.85 : 0.0;
//		lights[LFSR_LIGHTS + i].value = lfsrState[i] ? 1.0 : 0.0;
	}
	
	jj = 1;
	for (int i = 0; i < 8; i++, jj <<= 1)
		lights[LFSR_LIGHTS + i].value = (lfsrBits & jj) ? 1.0 : 0.0;

	// Rows become dot products	/* volt/octave! */
	
	jj = 1;
	float row1 = 0.0;
	float row2 = 0.0;
	float row3 = 0.0;
	int scaleIndex;

	for (int i = 0; i < 8; i++, jj <<= 1) {
		row1 += (lfsrBits & jj) ? params[ROW1_PARAM + i].value : 0.0;
		row2 += (lfsrBits & jj) ? params[ROW2_PARAM + i].value : 0.0;
		row3 += (lfsrBits & jj) ? params[ROW3_PARAM + i].value : 0.0;
	}
	
	if (scaleType) {
		row1 /= 12.0;
		row2 /= 12.0;
		row3 /= 12.0;
	}
	else {
		scaleIndex = row1 + 0.05;
		row1 = diatonicScale[scaleIndex % 7] / 12.0 + scaleIndex / 7;

		scaleIndex = row2 + 0.05;
		row2 = diatonicScale[scaleIndex % 7] / 12.0 + scaleIndex / 7;

		scaleIndex = row3 + 0.05;
		row3 = diatonicScale[scaleIndex % 7] / 12.0 + scaleIndex / 7;
	}	
	//...	outputs[GATES_OUTPUT].value = gatesOn ? 10.0 : 0.0;
	// we'll add a mode switch for this, perhaps by channel? 
	if (gateIn && ((lfsrBits & LFSR_MASK) & (gateBits & LFSR_MASK))) {	/* only if we generate a trigger */
		outputs[GATES_OUTPUT].value = 10.0f;			
		outputs[ROW1_OUTPUT].value = row1;		// do we update the outputs
		outputs[ROW2_OUTPUT].value = row2;
		outputs[ROW3_OUTPUT].value = row3;
	} else {
		outputs[GATES_OUTPUT].value = 0.0f;
		if (gateType) {
			outputs[ROW1_OUTPUT].value = row1;	// or if he's switched on gateType
			outputs[ROW2_OUTPUT].value = row2;
			outputs[ROW3_OUTPUT].value = row3;
		}
	}

	if (!running) {
		outputs[GATES_OUTPUT].value = 0.0f;
		outputs[ROW1_OUTPUT].value = row1;		// or it's not running, hmmm.
		outputs[ROW2_OUTPUT].value = row2;
		outputs[ROW3_OUTPUT].value = row3;
	}

	lights[RESET_LIGHT].value = resetLight;
}

template <typename BASE>
struct bigLight : BASE {
	bigLight() {
		this->box.size = mm2px(Vec(6.0, 6.0));
	}
};


struct mySmallSnapKnob : RoundSmallBlackKnob {
	mySmallSnapKnob() {
		snap = true;
		smooth = false;
	}
};


struct FG8Widget : ModuleWidget {
	FG8Widget(FG8 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/FG8.svg")));

/* old
FG8Widget::FG8Widget() {
	FG8 *module = new FG8();
	setModule(module);
	box.size = Vec(15*22, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/FG8.svg")));
		addChild(panel);
	}
*/	
	static const float portX0[8] = {20, 58, 96, 135, 173, 212, 250, 289};
	static const float line98 = 91;

	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(0, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 365)));

	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(18, 56), module, FG8::CLOCK_PARAM, -2.0, 6.0, 2.0));
	addParam(ParamWidget::create<LEDButton>(Vec(60, 61-1), module, FG8::RUN_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(64.4, 64.4), module, FG8::RUNNING_LIGHT));
	addParam(ParamWidget::create<LEDButton>(Vec(99, 61-1), module, FG8::RESET_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(103.4, 64.4), module, FG8::RESET_LIGHT));
	
//	addParam(createParam<RoundSmallBlackSnapKnob>(Vec(132, 56), module, FG8::STEPS_PARAM, 1.0, 8.0, 8.0));

	addParam(ParamWidget::create<CKSS>(Vec(139, 74 - 19), module, FG8::LFSR_MODE_PARAM, 0.0, 1.0, 1.0));
	addParam(ParamWidget::create<CKSS>(Vec(179 - 2, 34 + 21), module, FG8::GATE_MODE_PARAM, 0.0, 1.0, 1.0));
	addParam(ParamWidget::create<CKSS>(Vec(179 - 2 + 70 + 12, 34 + 21 + 32 + 5), module, FG8::SCALE_MODE_PARAM, 0.0, 1.0, 1.0));
//	addParam(createParam<NKK>(Vec(139, 74), module, FG8::MODE_PARAM, 0.0, 1.0, 1.0));

//	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(179.4, 64.4), module, FG8::GATES_LIGHT));
//	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(218.4, 64.4), module, FG8::ROW_LIGHTS));
//	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(256.4, 64.4), module, FG8::ROW_LIGHTS + 1));
//	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(295.4, 64.4), module, FG8::ROW_LIGHTS + 2));

	addInput(Port::create<PJ301MPort>(Vec(portX0[0]-1, line98), Port::INPUT, module, FG8::CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(portX0[1]-1, line98), Port::INPUT, module, FG8::EXT_CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(portX0[2]-1, line98), Port::INPUT, module, FG8::RESET_INPUT));
	
	
//	addInput(createInput<PJ301MPort>(Vec(portX0[3]-1, line98), module, FG8::STEPS_INPUT));
	
	addOutput(Port::create<PJ301MPort>(Vec(portX0[5]-1, line98), Port::OUTPUT, module, FG8::GATES_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(portX0[5]-1 + 44, line98 - 39), Port::OUTPUT, module, FG8::ROW1_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(portX0[6]-1 + 41, line98 - 39), Port::OUTPUT, module, FG8::ROW2_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(portX0[6]-1 + 41, line98), Port::OUTPUT, module, FG8::ROW3_OUTPUT));

/* OOPS */

	static const float portX1[NUM_CHANNELS] = {289, 250, 212, 173, 135, 96, 58, 20};
	for (int i = 0; i < NUM_CHANNELS; i++) {
//		addParam(createParam<RoundSmallBlackKnob>(Vec(portX[i]-2, 157), module, FG8::ROW1_PARAM + i, 0.0, 10.0, 0.0));

/* change row knobs to snap octave 12 step knobs  ...removed snap? */
		addParam(ParamWidget::create<LEDBezel>(Vec(portX1[i], 126.5), module, FG8::LFSR_PARAM + i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<bigLight<RedLight>>(Vec(portX1[i] + 2, 128.5), module, FG8::LFSR_LIGHTS + i));

		addParam(ParamWidget::create<mySmallSnapKnob>(Vec(portX1[i]-2, 157), module, FG8::ROW1_PARAM + i, 0.0, 7.0, 0.0));
		addParam(ParamWidget::create<mySmallSnapKnob>(Vec(portX1[i]-2, 198), module, FG8::ROW2_PARAM + i, 0.0, 7.0, 0.0));
		addParam(ParamWidget::create<mySmallSnapKnob>(Vec(portX1[i]-2, 240), module, FG8::ROW3_PARAM + i, 0.0, 7.0, 0.0));
		
		addParam(ParamWidget::create<LEDBezel>(Vec(portX1[i], 282), module, FG8::GATE_PARAM + i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<bigLight<GreenLight>>(Vec(portX1[i] + 2, 284), module, FG8::GATE_LIGHTS + i));

		addParam(ParamWidget::create<LEDBezel>(Vec(portX1[i], 322), module, FG8::TAP_PARAM + i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<bigLight<BlueLight>>(Vec(portX1[i] + 2, 324), module, FG8::TAP_LIGHTS + i));

	}
}
};

} // namespace rack_plugin_alto777_LFSR

using namespace rack_plugin_alto777_LFSR;

RACK_PLUGIN_MODEL_INIT(alto777_LFSR, FG8) {
   Model *modelFG8 = Model::create<FG8, FG8Widget>("alto777_LFSR", "FG8", "FG-8", SEQUENCER_TAG);
   return modelFG8;
}
//		addOutput(createOutput<PJ301MPort>(Vec(portX[i]-1, 307), module, FG8::GATE_OUTPUT + i));
