#include "ESeries.hpp"
#include "dsp/filter.hpp"
#include "dsp/minblep.hpp"


struct E340 : Module {
	enum ParamIds {
		COARSE_PARAM,
		FINE_PARAM,
		FM_PARAM,
		SPREAD_PARAM,
		CHAOS_PARAM,
		CHAOS_BW_PARAM,
		DENSITY_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
		FM_INPUT,
		SYNC_INPUT,
		SPREAD_INPUT,
		CHAOS_INPUT,
		CHAOS_BW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SAW_OUTPUT,
		SINE_OUTPUT,
		NUM_OUTPUTS
	};

	float phases[8] = {};
	RCFilter noiseFilters[8];
	float sync = 0.0;

	MinBLEP<16> sineMinBLEP;
	MinBLEP<16> sawMinBLEP;

	// For removing DC
	RCFilter sineFilter;
	RCFilter sawFilter;

	E340();
	void step() override;
};


E340::E340() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
	sineMinBLEP.minblep = minblep_16_32;
	sineMinBLEP.oversample = 32;
	sawMinBLEP.minblep = minblep_16_32;
	sawMinBLEP.oversample = 32;

	// Randomize initial phases
	for (int i = 0; i < 8; i++) {
		phases[i] = randomUniform();
	}
}

void E340::step() {
	// Base pitch
	float basePitch = params[COARSE_PARAM].value + 12.0 * inputs[PITCH_INPUT].value;
	if (inputs[FM_INPUT].active) {
		basePitch += 12.0 / 4.0 * params[FM_PARAM].value * inputs[FM_INPUT].value;
	}
	basePitch += params[FINE_PARAM].value;

	// Spread
	float spread = params[SPREAD_PARAM].value + inputs[SPREAD_INPUT].value / 10.0;
	spread = clamp(spread, 0.0f, 1.0f);
	const float spreadPower = 50.0;
	spread = (powf(spreadPower + 1.0, spread) - 1.0) / spreadPower;

	// Chaos
	float chaos = params[CHAOS_PARAM].value + inputs[CHAOS_INPUT].value / 10.0;
	chaos = clamp(chaos, 0.0f, 1.0f);
	const float chaosPower = 50.0;
	chaos = 8.0 * (powf(chaosPower + 1.0, chaos) - 1.0) / chaosPower;

	// Chaos BW
	float chaosBW = params[CHAOS_BW_PARAM].value + inputs[CHAOS_BW_INPUT].value / 10.0;
	chaosBW = clamp(chaosBW, 0.0f, 1.0f);
	chaosBW = 6.0 * powf(100.0, chaosBW);
	// This shouldn't scale with the global sample rate, because of reasons.
	float filterCutoff = chaosBW / 44100.0;

	// Check sync input
	float newSync = inputs[SYNC_INPUT].value - 0.25;
	float syncCrossing = INFINITY;
	if (sync < 0.0 && newSync >= 0.0) {
		float deltaSync = newSync - sync;
		syncCrossing = -newSync / deltaSync;
	}
	sync = newSync;

	// Density
	int density;
	switch ((int)roundf(params[DENSITY_PARAM].value)) {
		case 0: density = 2; break;
		case 1: density = 4; break;
		default: density = 8; break;
	}

	// Detuning amounts, in note value
	const static float detunings[8] = {-21, 21, -9, 9, -3, 3, -15, 15}; // Perfect fourths
	// const static float detunings[8] = {-24, 24, -12, 12, -5, 7, -17, 19}; // Fifths

	// Oscillator block
	float sines = 0.0;
	float saws = 0.0;
	for (int i = 0; i < density; i++) {
		// Noise
		float noise = 0.0;
		if (chaos > 0.0) {
			noise = randomNormal();
			noiseFilters[i].setCutoff(filterCutoff);
			noiseFilters[i].process(noise);
			noise = noiseFilters[i].lowpass();
			noise *= chaos;
		}

		// Frequency
		float pitch = basePitch + spread * detunings[i] + 12.0 * noise;
		pitch = clamp(pitch, -72.0f, 72.0f);
		float freq = 261.626 * powf(2.0, pitch / 12.0);

		// Advance phase
		float deltaPhase = freq * engineGetSampleTime();
		float phase = phases[i] + deltaPhase;

		// Reset phase
		if (phase >= 1.0) {
			phase -= 1.0;
			float crossing = -phase / deltaPhase;
			sawMinBLEP.jump(crossing, -2.0);
		}

		// Compute output
		float sine = -cosf(2*M_PI * phase);
		float saw = 2.0*phase - 1.0;

		// Sync
		if (syncCrossing <= 0.0) {
			phase = deltaPhase * -syncCrossing;
			float newSine = -cosf(2*M_PI * phase);
			float newSaw = 2.0*phase - 1.0;
			sineMinBLEP.jump(syncCrossing, newSine - sine);
			sawMinBLEP.jump(syncCrossing, newSaw - saw);
			sine = newSine;
			saw = newSaw;
		}

		phases[i] = phase;
		sines += sine;
		saws += saw;
	}

	sines += sineMinBLEP.shift();
	saws += sawMinBLEP.shift();

	sines /= density;
	saws /= density;

	// Apply HP filter at 20Hz
	float r = 20.0 * engineGetSampleTime();
	sineFilter.setCutoff(r);
	sawFilter.setCutoff(r);

	sineFilter.process(sines);
	sawFilter.process(saws);

	outputs[SINE_OUTPUT].value = 5.0 * sineFilter.highpass();
	outputs[SAW_OUTPUT].value = 5.0 * sawFilter.highpass();
}


struct E340Widget : ModuleWidget {
	E340Widget(E340 *module);
};


E340Widget::E340Widget(E340 *module) : ModuleWidget(module) {
#ifdef USE_VST2
	setPanel(SVG::load(assetStaticPlugin("ESeries", "res/E340.svg")));
#else
	setPanel(SVG::load(assetPlugin(plugin, "res/E340.svg")));
#endif // USE_VST2

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<SynthTechAlco>(Vec(26, 43), module, E340::COARSE_PARAM, -48.0, 48.0, 0.0));
	addParam(ParamWidget::create<SynthTechAlco>(Vec(137, 43), module, E340::FINE_PARAM, -1.0, 1.0, 0.0));

	addParam(ParamWidget::create<SynthTechAlco>(Vec(26, 109), module, E340::FM_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<SynthTechAlco>(Vec(137, 109), module, E340::SPREAD_PARAM, 0.0, 1.0, 0.0));

	addParam(ParamWidget::create<SynthTechAlco>(Vec(26, 175), module, E340::CHAOS_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<SynthTechAlco>(Vec(137, 175), module, E340::CHAOS_BW_PARAM, 0.0, 1.0, 0.5));

	addParam(ParamWidget::create<NKK>(Vec(89, 140), module, E340::DENSITY_PARAM, 0.0, 2.0, 2.0));

	addInput(Port::create<CL1362Port>(Vec(13, 243), Port::INPUT, module, E340::PITCH_INPUT));
	addInput(Port::create<CL1362Port>(Vec(63, 243), Port::INPUT, module, E340::FM_INPUT));
	addInput(Port::create<CL1362Port>(Vec(113, 243), Port::INPUT, module, E340::SYNC_INPUT));
	addInput(Port::create<CL1362Port>(Vec(163, 243), Port::INPUT, module, E340::SPREAD_INPUT));

	addInput(Port::create<CL1362Port>(Vec(13, 301), Port::INPUT, module, E340::CHAOS_INPUT));
	addInput(Port::create<CL1362Port>(Vec(63, 301), Port::INPUT, module, E340::CHAOS_BW_INPUT));
	addOutput(Port::create<CL1362Port>(Vec(113, 301), Port::OUTPUT, module, E340::SAW_OUTPUT));
	addOutput(Port::create<CL1362Port>(Vec(163, 301), Port::OUTPUT, module, E340::SINE_OUTPUT));
}


RACK_PLUGIN_MODEL_INIT(ESeries, E340) {
   Model *modelE340 = Model::create<E340, E340Widget>("E-Series", "E340", "E340 Cloud Generator", OSCILLATOR_TAG);
   return modelE340;
}
