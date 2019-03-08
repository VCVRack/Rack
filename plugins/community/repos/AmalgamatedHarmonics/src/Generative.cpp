#include "util/common.hpp"
#include "dsp/digital.hpp"

#include "dsp/noise.hpp"

#include "AH.hpp"
#include "Core.hpp"
#include "UI.hpp"
#include "VCO.hpp"

namespace rack_plugin_AmalgamatedHarmonics {

struct Generative : AHModule {

	enum ParamIds {
		FREQ_PARAM,
		WAVE_PARAM,
		FM_PARAM,
		AM_PARAM,
		NOISE_PARAM,
		CLOCK_PARAM,
		PROB_PARAM,
		DELAYL_PARAM,
		DELAYS_PARAM,
		GATEL_PARAM,
		GATES_PARAM,
		SLOPE_PARAM,
		SPEED_PARAM,
		ATTN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		WAVE_INPUT,
		FM_INPUT,
		AM_INPUT,
		NOISE_INPUT,
		SAMPLE_INPUT,
		CLOCK_INPUT,
		PROB_INPUT,
		HOLD_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LFO_OUTPUT,
		MIXED_OUTPUT,
		NOISE_OUTPUT,
		OUT_OUTPUT,
		GATE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(GATE_LIGHT,2),
		NUM_LIGHTS
	};

	Generative() : AHModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }
	
	void step() override;

		json_t *toJson() override {
		json_t *rootJ = json_object();

		// quantise
		json_t *quantiseJ = json_boolean(quantise);
		json_object_set_new(rootJ, "quantise", quantiseJ);

		// offset
		json_t *offsetJ = json_boolean(offset);
		json_object_set_new(rootJ, "offset", offsetJ);


		return rootJ;
	}
	
	void fromJson(json_t *rootJ) override {
		// quantise
		json_t *quantiseJ = json_object_get(rootJ, "quantise");
		
		if (quantiseJ) {
			quantise = json_boolean_value(quantiseJ);
		}

		// offset
		json_t *offsetJ = json_object_get(rootJ, "offset");
		
		if (offsetJ) {
			offset = json_boolean_value(offsetJ);
		}

	}


	Core core;

	SchmittTrigger sampleTrigger;
	SchmittTrigger holdTrigger;
	SchmittTrigger clockTrigger;
	bogaudio_dsp::PinkNoiseGenerator pink;
	LowFrequencyOscillator oscillator;
	LowFrequencyOscillator clock;
	AHPulseGenerator delayPhase;
	AHPulseGenerator gatePhase;

	float target = 0.0f;
	float current = 0.0f;
	bool quantise = false;
	bool offset = false;
	bool delayState = false;
	bool gateState = false;
	
	// minimum and maximum slopes in volts per second
	const float slewMin = 0.1;
	const float slewMax = 10000.0;
	const float slewRatio = slewMin / slewMax;
	
	// Amount of extra slew per voltage difference
	const float shapeScale = 1.0 / 10.0;

	float delayTime;
	float gateTime;

};

void Generative::step() {
	
	AHModule::step();

	oscillator.setPitch(params[FREQ_PARAM].value + params[FM_PARAM].value * inputs[FM_INPUT].value);
	oscillator.offset = offset;
	oscillator.step(delta);

	clock.setPitch(clamp(params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value, -2.0f, 6.0f));
	clock.step(delta);

	float wavem = fabs(fmodf(params[WAVE_PARAM].value + inputs[WAVE_INPUT].value, 4.0f));

	float interp = 0.0f;
	bool toss = false;

	if (wavem < 1.0f)
		interp = crossfade(oscillator.sin(), oscillator.tri(), wavem) * 5.0; 
	else if (wavem < 2.0f)
		interp = crossfade(oscillator.tri(), oscillator.saw(), wavem - 1.0f) * 5.0;
	else if (wavem < 3.0f)
		interp = crossfade(oscillator.saw(), oscillator.sqr(), wavem - 2.0f) * 5.0;
	else 
		interp = crossfade(oscillator.sqr(), oscillator.sin(), wavem - 3.0f) * 5.0;


	// Capture (pink) noise
	float noise = clamp(pink.next() * 7.5f, -5.0f, 5.0f); // -5V to 5V
	float range = params[ATTN_PARAM].value;

	// Shift the noise floor
	if (offset) {
		noise += 5.0f;
	} 

	float noiseLevel = clamp(params[NOISE_PARAM].value + inputs[NOISE_INPUT].value, 0.0f, 1.0f);

	// Mixed the input AM signal or noise
	if (inputs[AM_INPUT].active) {
		interp = crossfade(interp, inputs[AM_INPUT].value, params[AM_PARAM].value) * range;
	} else {
		interp *= range;
	}

	// Mix noise
	float mixedSignal = (noise * noiseLevel + interp * (1.0f - noiseLevel));

	// Process gate
	bool sampleActive = inputs[SAMPLE_INPUT].active;
	float sample = inputs[SAMPLE_INPUT].value;
	bool hold = inputs[HOLD_INPUT].value > 0.000001f;

	bool isClocked = false;

	if (!sampleActive) {
		if (clockTrigger.process(clock.sqr())) {
			isClocked = true;
		}
	} else {
		if (sampleTrigger.process(sample)) {
			isClocked = true;
		}
	}

	// If we have no input or we have been a trigger on the sample input
	if (isClocked) {

		// If we are not in a delay or gate state process the tick, otherwise eat it
		if (!delayPhase.ishigh() && !gatePhase.ishigh()) {

			// Check against prob control
			float threshold = clamp(params[PROB_PARAM].value + inputs[PROB_INPUT].value / 10.f, 0.f, 1.f);
			toss = (randomUniform() < threshold);

			// Tick is valid
			if (toss) {

				// Determine delay time
				float dlyLen = log2(params[DELAYL_PARAM].value);
				float dlySpr = log2(params[DELAYS_PARAM].value);

				double rndD = clamp(core.gaussrand(), -2.0f, 2.0f);
				delayTime = clamp(dlyLen + dlySpr * rndD, 0.0f, 100.0f);
				
				// Trigger the respective delay pulse generator
				delayState = true;
				delayPhase.trigger(delayTime);
			}
		} 
	}

	// In delay state and finished waiting
	if (delayState && !delayPhase.process(delta)) {

		// set the target voltage
		target = mixedSignal;

		// Determine gate time
		float gateLen = log2(params[GATEL_PARAM].value);
		float gateSpr = log2(params[GATES_PARAM].value);

		double rndG = clamp(core.gaussrand(), -2.0f, 2.0f);
		gateTime = clamp(gateLen + gateSpr * rndG, Core::TRIGGER, 100.0f);

		// Open the gate and set flags
		gatePhase.trigger(gateTime);
		gateState = true;
		delayState = false;			
	}

	// If not held slew voltages
	if (!hold) {

		// Curve calc
		float shape = params[SLOPE_PARAM].value;
		float speed = params[SPEED_PARAM].value;	
		float slew = slewMax * powf(slewRatio, speed);

		// Rise
		if (target > current) {
			current += slew * crossfade(1.0f, shapeScale * (target - current), shape) * delta;
			if (current > target) // Trap overshoot
				current = target;
		}
		// Fall
		else if (target < current) {
			current -= slew * crossfade(1.0f, shapeScale * (current - target), shape) * delta;
			if (current < target) // Trap overshoot
				current = target;
		}
	}

	// Quantise or not
	float out;
	if (quantise) {
		int i;
		int d;
		out = CoreUtil().getPitchFromVolts(current, Core::NOTE_C, Core::SCALE_CHROMATIC, &i, &d);
	} else {
		out = current;
	}

	// If the gate is open, set output to high
	if (gatePhase.process(delta)) {
		outputs[GATE_OUTPUT].value = 10.0f;

		lights[GATE_LIGHT].setBrightnessSmooth(1.0f);
		lights[GATE_LIGHT + 1].setBrightnessSmooth(0.0f);

	} else {
		outputs[GATE_OUTPUT].value = 0.0f;
		gateState = false;

		if (delayState) {
			lights[GATE_LIGHT].setBrightnessSmooth(0.0f);
			lights[GATE_LIGHT + 1].setBrightnessSmooth(1.0f);
		} else {
			lights[GATE_LIGHT].setBrightnessSmooth(0.0f);
			lights[GATE_LIGHT + 1].setBrightnessSmooth(0.0f);
		}

	}

	outputs[OUT_OUTPUT].value = out;
	outputs[NOISE_OUTPUT].value = noise * 2.0;
	outputs[LFO_OUTPUT].value = interp;
	outputs[MIXED_OUTPUT].value = mixedSignal;
	
}

struct GenerativeWidget : ModuleWidget {
	
	GenerativeWidget(Generative *module) : ModuleWidget(module) {
		
		UI ui;
		
		box.size = Vec(240, 380);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Generative.svg")));
			addChild(panel);
		}

		// LFO section
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 0, 0, false, false), module, Generative::FREQ_PARAM, -8.0f, 10.0f, 1.0f));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 1, 0, false, false), module, Generative::WAVE_PARAM, 0.0f, 4.0f, 1.5f));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 2, 0, false, false), module, Generative::FM_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 3, 0, false, false), module, Generative::AM_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 4, 0, false, false), module, Generative::NOISE_PARAM, 0.0f, 1.0f, 0.5f));
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 1, 1, false, false), Port::INPUT, module, Generative::WAVE_INPUT));
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 2, 1, false, false), Port::INPUT, module, Generative::FM_INPUT));
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 3, 1, false, false), Port::INPUT, module, Generative::AM_INPUT));
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 4, 1, false, false), Port::INPUT, module, Generative::NOISE_INPUT));

		// Gate Section
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, 2, false, false), Port::INPUT, module, Generative::SAMPLE_INPUT));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 1, 2, false, false), module, Generative::CLOCK_PARAM, -2.0, 6.0, 1.0));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 2, 2, false, false), module, Generative::PROB_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 3, 2, false, false), module, Generative::DELAYL_PARAM, 1.0f, 2.0f, 1.0f));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 4, 2, false, false), module, Generative::GATEL_PARAM, 1.0f, 2.0f, 1.0f));

		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 1, 3, false, false), Port::INPUT, module, Generative::CLOCK_INPUT));
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 2, 3, false, false), Port::INPUT, module, Generative::PROB_INPUT));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 3, 3, false, false), module, Generative::DELAYS_PARAM, 1.0f, 2.0f, 1.0f));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 4, 3, false, false), module, Generative::GATES_PARAM, 1.0f, 2.0f, 1.0f));

		// Curve Section
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 0, 4, false, false), module, Generative::SLOPE_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 1, 4, false, false), module, Generative::SPEED_PARAM, 0.0, 1.0, 0.0));
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 2, 4, false, false), Port::INPUT, module, Generative::HOLD_INPUT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(ui.getPosition(UI::LIGHT, 3, 4, false, false), module, Generative::GATE_LIGHT));
		addParam(ParamWidget::create<AHKnobNoSnap>(ui.getPosition(UI::KNOB, 4, 4, false, false), module, Generative::ATTN_PARAM, 0.0, 1.0, 1.0)); 

		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, 5, false, false), Port::OUTPUT, module, Generative::LFO_OUTPUT));
		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 1, 5, false, false), Port::OUTPUT, module, Generative::MIXED_OUTPUT));
		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 2, 5, false, false), Port::OUTPUT, module, Generative::NOISE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 3, 5, false, false), Port::OUTPUT, module, Generative::GATE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 4, 5, false, false), Port::OUTPUT, module, Generative::OUT_OUTPUT));




	}

	void appendContextMenu(Menu *menu) override {
			Generative *gen = dynamic_cast<Generative*>(module);
			assert(gen);

			struct GenModeItem : MenuItem {
				Generative *gen;
				void onAction(EventAction &e) override {
					gen->quantise ^= 1;
				}
				void step() override {
					rightText = gen->quantise ? "Quantised" : "Unquantised";
					MenuItem::step();
				}
			};

			struct GenOffsetItem : MenuItem {
				Generative *gen;
				void onAction(EventAction &e) override {
					gen->offset ^= 1;
				}
				void step() override {
					rightText = gen->offset ? "0V - 10V" : "-5V to 5V";
					MenuItem::step();
				}
			};

			menu->addChild(construct<MenuLabel>());
			menu->addChild(construct<GenModeItem>(&MenuItem::text, "Quantise", &GenModeItem::gen, gen));
			menu->addChild(construct<GenOffsetItem>(&MenuItem::text, "CV Offset", &GenOffsetItem::gen, gen));
	}
};

} // namespace rack_plugin_AmalgamatedHarmonics

using namespace rack_plugin_AmalgamatedHarmonics;

RACK_PLUGIN_MODEL_INIT(AmalgamatedHarmonics, Generative) {
   Model *modelGenerative = Model::create<Generative, GenerativeWidget>( "Amalgamated Harmonics", "Generative", "Generative", NOISE_TAG, SAMPLE_AND_HOLD_TAG, LFO_TAG, RANDOM_TAG);
   return modelGenerative;
}
