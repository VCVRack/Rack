#include "SubmarineFree.hpp"
#include "dsp/functions.hpp"

namespace rack_plugin_SubmarineFree {

struct PO_Util {
	static constexpr float deg0   = 0.0f;
	static constexpr float deg30  =         M_PI / 6.0f;
	static constexpr float deg45  =         M_PI / 4.0f;
	static constexpr float deg60  =         M_PI / 3.0f;
	static constexpr float deg90  =         M_PI / 2.0f;
	static constexpr float deg120 = 2.0f  * M_PI / 3.0f;
	static constexpr float deg135 = 3.0f  * M_PI / 4.0f;
	static constexpr float deg150 = 5.0f  * M_PI / 6.0f;
	static constexpr float ph0 = 0.0f;
	static constexpr float ph30 = 1.0f / 12.0f;
	static constexpr float ph45 = 0.125f;
	static constexpr float ph60 = 1.0f / 6.0f;
	static constexpr float ph90 = 0.25f;
	static constexpr float ph120 = 1.0f / 3.0f;
	static constexpr float ph135 = 0.375f;
	static constexpr float ph150 = 5.0f / 12.0f;
	static constexpr float ph180 = 0.5f;
	static constexpr float ph210 = 7.0f / 12.0f;
	static constexpr float ph225 = 0.625;
	static constexpr float ph240 = 2.0f / 3.0f;
	static constexpr float ph270 = 0.75f;
	static constexpr float ph300 = 5.0f / 6.0f;
	static constexpr float ph315 = 0.875f;
	static constexpr float ph330 = 11.0f / 12.0f;

	float sin(float phase);
	float tri(float phase);
	float saw(float phase);
	float sqr(float phase);
	float rsn(float phase);
};

float PO_Util::sin(float phase) {
	return 5.0f * sinf(phase);
}

float PO_Util::tri(float phase) {
	phase -= floor(phase);
	if (phase < 0.25f)
		return 20.0f * phase;
	if (phase < 0.75f)
		return 20.0f * (0.5f - phase);
	return 20.0f * (phase - 1.0f);
}

float PO_Util::saw(float phase) {
	phase -= floor(phase);
	if (phase < 0.5f)
		return 10.0f * phase;
	return 10.0f * (phase - 1.0f);
}

float PO_Util::sqr(float phase) {
	phase -= floor(phase);
	return (phase < 0.5f)?5.0f:-5.0f;
}

float PO_Util::rsn(float phase) {
	return 10.0f * fabs(sinf(phase)) - 5.0f; 
}

struct PO_101 : Module, PO_Util {
	
	enum ParamIds {
		PARAM_TUNE,
		PARAM_FINE,
		PARAM_WAVE,
		PARAM_PHASE_1,
		PARAM_PHASE_2,
		PARAM_PHASE_3,
		PARAM_PHASE_4,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_NOTE_CV,
		INPUT_PHASE_1,
		INPUT_PHASE_2,
		INPUT_PHASE_3,
		INPUT_PHASE_4,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
		OUTPUT_6,
		OUTPUT_7,
		OUTPUT_8,
		OUTPUT_9,
		OUTPUT_10,
		OUTPUT_11,
		OUTPUT_12,
		OUTPUT_13,
		OUTPUT_14,
		OUTPUT_15,
		OUTPUT_16,
		OUTPUT_17,
		OUTPUT_18,
		OUTPUT_19,
		OUTPUT_20,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	PO_101() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
	void sin(float phase);
	void tri(float phase);
	void saw(float phase);
	void sqr(float phase);
	void rsn(float phase);
	float phase = 0.0f;
	float baseFreq = 261.626f;
};

void PO_101::sin(float phase) {
	phase *= (2 * M_PI);
	outputs[OUTPUT_9].value = -(outputs[OUTPUT_1].value = PO_Util::sin(phase + deg0));
	outputs[OUTPUT_10].value = -(outputs[OUTPUT_2].value = PO_Util::sin(phase + deg30)); 
	outputs[OUTPUT_11].value = -(outputs[OUTPUT_3].value = PO_Util::sin(phase + deg45)); 
	outputs[OUTPUT_12].value = -(outputs[OUTPUT_4].value = PO_Util::sin(phase + deg60)); 
	outputs[OUTPUT_13].value = -(outputs[OUTPUT_5].value = PO_Util::sin(phase + deg90)); 
	outputs[OUTPUT_14].value = -(outputs[OUTPUT_6].value = PO_Util::sin(phase + deg120)); 
	outputs[OUTPUT_15].value = -(outputs[OUTPUT_7].value = PO_Util::sin(phase + deg135)); 
	outputs[OUTPUT_16].value = -(outputs[OUTPUT_8].value = PO_Util::sin(phase + deg150)); 
	for (int i = 0; i < 4; i++) {
		if (outputs[OUTPUT_17 + i].active) {
			float offset = params[PARAM_PHASE_1 + i].value;
			if (inputs[INPUT_PHASE_1 + i].active)
				offset += inputs[INPUT_PHASE_1 + i].value * 0.4f;
			offset *= 2 * M_PI;
			outputs[OUTPUT_17 + i].value = PO_Util::sin(phase + offset);
		}	
	}
}

void PO_101::tri(float phase) {
	outputs[OUTPUT_9].value = -(outputs[OUTPUT_1].value = PO_Util::tri(phase + ph0));
	outputs[OUTPUT_10].value = -(outputs[OUTPUT_2].value = PO_Util::tri(phase + ph30)); 
	outputs[OUTPUT_11].value = -(outputs[OUTPUT_3].value = PO_Util::tri(phase + ph45)); 
	outputs[OUTPUT_12].value = -(outputs[OUTPUT_4].value = PO_Util::tri(phase + ph60)); 
	outputs[OUTPUT_13].value = -(outputs[OUTPUT_5].value = PO_Util::tri(phase + ph90)); 
	outputs[OUTPUT_14].value = -(outputs[OUTPUT_6].value = PO_Util::tri(phase + ph120)); 
	outputs[OUTPUT_15].value = -(outputs[OUTPUT_7].value = PO_Util::tri(phase + ph135)); 
	outputs[OUTPUT_16].value = -(outputs[OUTPUT_8].value = PO_Util::tri(phase + ph150)); 
	for (int i = 0; i < 4; i++) {
		if (outputs[OUTPUT_17 + i].active) {
			float offset = params[PARAM_PHASE_1 + i].value;
			if (inputs[INPUT_PHASE_1 + i].active)
				offset += inputs[INPUT_PHASE_1 + i].value * 0.4f;
			outputs[OUTPUT_17 + i].value = PO_Util::tri(phase + offset);
		}	
	}
}

void PO_101::saw(float phase) {
	outputs[OUTPUT_1].value = PO_Util::saw(phase + ph0);
	outputs[OUTPUT_2].value = PO_Util::saw(phase + ph30); 
	outputs[OUTPUT_3].value = PO_Util::saw(phase + ph45); 
	outputs[OUTPUT_4].value = PO_Util::saw(phase + ph60); 
	outputs[OUTPUT_5].value = PO_Util::saw(phase + ph90); 
	outputs[OUTPUT_6].value = PO_Util::saw(phase + ph120); 
	outputs[OUTPUT_7].value = PO_Util::saw(phase + ph135); 
	outputs[OUTPUT_8].value = PO_Util::saw(phase + ph150); 
	outputs[OUTPUT_9].value = PO_Util::saw(phase + ph180);
	outputs[OUTPUT_10].value = PO_Util::saw(phase + ph210); 
	outputs[OUTPUT_11].value = PO_Util::saw(phase + ph225); 
	outputs[OUTPUT_12].value = PO_Util::saw(phase + ph240); 
	outputs[OUTPUT_13].value = PO_Util::saw(phase + ph270); 
	outputs[OUTPUT_14].value = PO_Util::saw(phase + ph300); 
	outputs[OUTPUT_15].value = PO_Util::saw(phase + ph315); 
	outputs[OUTPUT_16].value = PO_Util::saw(phase + ph330); 
	for (int i = 0; i < 4; i++) {
		if (outputs[OUTPUT_17 + i].active) {
			float offset = params[PARAM_PHASE_1 + i].value;
			if (inputs[INPUT_PHASE_1 + i].active)
				offset += inputs[INPUT_PHASE_1 + i].value * 0.4f;
			outputs[OUTPUT_17 + i].value = PO_Util::saw(phase + offset);
		}	
	}
}

void PO_101::sqr(float phase) {
	outputs[OUTPUT_9].value = -(outputs[OUTPUT_1].value = PO_Util::sqr(phase + ph0));
	outputs[OUTPUT_10].value = -(outputs[OUTPUT_2].value = PO_Util::sqr(phase + ph30)); 
	outputs[OUTPUT_11].value = -(outputs[OUTPUT_3].value = PO_Util::sqr(phase + ph45)); 
	outputs[OUTPUT_12].value = -(outputs[OUTPUT_4].value = PO_Util::sqr(phase + ph60)); 
	outputs[OUTPUT_13].value = -(outputs[OUTPUT_5].value = PO_Util::sqr(phase + ph90)); 
	outputs[OUTPUT_14].value = -(outputs[OUTPUT_6].value = PO_Util::sqr(phase + ph120)); 
	outputs[OUTPUT_15].value = -(outputs[OUTPUT_7].value = PO_Util::sqr(phase + ph135)); 
	outputs[OUTPUT_16].value = -(outputs[OUTPUT_8].value = PO_Util::sqr(phase + ph150)); 
	for (int i = 0; i < 4; i++) {
		if (outputs[OUTPUT_17 + i].active) {
			float offset = params[PARAM_PHASE_1 + i].value;
			if (inputs[INPUT_PHASE_1 + i].active)
				offset += inputs[INPUT_PHASE_1 + i].value * 0.4f;
			outputs[OUTPUT_17 + i].value = PO_Util::sqr(phase + offset);
		}	
	}
}
	
void PO_101::rsn(float phase) {
	phase *= (2 * M_PI);
	outputs[OUTPUT_9].value = (outputs[OUTPUT_1].value = PO_Util::rsn(phase + deg0));
	outputs[OUTPUT_10].value = (outputs[OUTPUT_2].value = PO_Util::rsn(phase + deg30)); 
	outputs[OUTPUT_11].value = (outputs[OUTPUT_3].value = PO_Util::rsn(phase + deg45)); 
	outputs[OUTPUT_12].value = (outputs[OUTPUT_4].value = PO_Util::rsn(phase + deg60)); 
	outputs[OUTPUT_13].value = (outputs[OUTPUT_5].value = PO_Util::rsn(phase + deg90)); 
	outputs[OUTPUT_14].value = (outputs[OUTPUT_6].value = PO_Util::rsn(phase + deg120)); 
	outputs[OUTPUT_15].value = (outputs[OUTPUT_7].value = PO_Util::rsn(phase + deg135)); 
	outputs[OUTPUT_16].value = (outputs[OUTPUT_8].value = PO_Util::rsn(phase + deg150)); 
	for (int i = 0; i < 4; i++) {
		if (outputs[OUTPUT_17 + i].active) {
			float offset = params[PARAM_PHASE_1 + i].value;
			if (inputs[INPUT_PHASE_1 + i].active)
				offset += inputs[INPUT_PHASE_1 + i].value * 0.4f;
			offset *= 2 * M_PI;
			outputs[OUTPUT_17 + i].value = PO_Util::rsn(phase + offset);
		}	
	}
}

void PO_101::step() {
	float freq = baseFreq * powf(2.0f, (params[PARAM_TUNE].value + 3.0f * quadraticBipolar(params[PARAM_FINE].value)) / 12.0f + (inputs[INPUT_NOTE_CV].active?inputs[INPUT_NOTE_CV].value:0.0f));
	float deltaTime = freq / engineGetSampleRate();
	phase += deltaTime;
	double intPart;
	phase = modf(phase, &intPart); 
	
	{
		float waveShape = clamp(params[PARAM_WAVE].value, 0.0f, 4.0f);
		if (waveShape < 0.5f)
			sin(phase);
		else if (waveShape < 1.5f)
			tri(phase);
		else if (waveShape < 2.5f)
			saw(phase);
		else if (waveShape < 3.5f)
			sqr(phase);
		else
			rsn(phase);
	}

}

struct PO_204 : Module, PO_Util {
	
	enum ParamIds {
		PARAM_TUNE,
		PARAM_FINE,
		PARAM_WAVE_1,
		PARAM_WAVE_2,
		PARAM_WAVE_3,
		PARAM_WAVE_4,
		PARAM_PHASE_1,
		PARAM_PHASE_2,
		PARAM_PHASE_3,
		PARAM_PHASE_4,
		PARAM_MULT_1,
		PARAM_MULT_2,
		PARAM_MULT_3,
		PARAM_MULT_4,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_TUNE,
		INPUT_WAVE_1,
		INPUT_WAVE_2,
		INPUT_WAVE_3,
		INPUT_WAVE_4,
		INPUT_PHASE_1,
		INPUT_PHASE_2,
		INPUT_PHASE_3,
		INPUT_PHASE_4,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	PO_204() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
	float phase = 0.0f;
	float baseFreq = 261.626f;
};

void PO_204::step() {
	float freq = baseFreq * powf(2.0f, (params[PARAM_TUNE].value + 3.0f * quadraticBipolar(params[PARAM_FINE].value)) / 12.0f + (inputs[INPUT_TUNE].active?inputs[INPUT_TUNE].value:0.0f));
	float deltaTime = freq / engineGetSampleRate();
	phase += deltaTime;
	double intPart;
	phase = modf(phase, &intPart); 
	for (int i = 0; i < 4; i++) {
		if (outputs[OUTPUT_1 + i].active) {
			float offset = phase + params[PARAM_PHASE_1 + i].value;
			if (inputs[INPUT_PHASE_1 + i].active)
				offset += inputs[INPUT_PHASE_1 + i].value * 0.4f;
			offset *= params[PARAM_MULT_1 + i].value;
			float wave = params[PARAM_WAVE_1 + i].value + (inputs[INPUT_WAVE_1 + i].active?inputs[INPUT_WAVE_1 + i].value:0.0f);
			double waveSection;
			wave = modf(clamp(wave, 0.0f, 10.0f), &waveSection);		
			float w1 = 0.0f;
			float w2 = 0.0f;
			switch ((int)waveSection) {
				case 0:
					w1 = PO_Util::sin(offset * 2 * M_PI);
					w2 = PO_Util::saw(offset);
					break;
				case 1:
					w1 = PO_Util::saw(offset);
					w2 = PO_Util::rsn(offset * 2 * M_PI);
					break;
				case 2:
					w1 = PO_Util::rsn(offset * 2 * M_PI);
					w2 = PO_Util::tri(offset);
					break;
				case 3:
					w1 = PO_Util::tri(offset);
					w2 = PO_Util::sqr(offset);
					break;
				case 4:
					w1 = PO_Util::sqr(offset);
					w2 = PO_Util::sin(offset * 2 * M_PI);
					break;
				case 5:
					w1 = PO_Util::sin(offset * 2 * M_PI);
					w2 = PO_Util::tri(offset);
					break;
				case 6:
					w1 = PO_Util::tri(offset);
					w2 = PO_Util::saw(offset);
					break;
				case 7:
					w1 = PO_Util::saw(offset);
					w2 = PO_Util::sqr(offset);
					break;
				case 8:
					w1 = PO_Util::sqr(offset);
					w2 = PO_Util::rsn(offset * 2 * M_PI);
					break;
				case 9:
					w1 = PO_Util::rsn(offset * 2 * M_PI);
					w2 = PO_Util::sin(offset * 2 * M_PI);
					break;
				default:
					w2 = w1 = PO_Util::sin(offset * 2 * M_PI);
					break;
			}
			outputs[OUTPUT_1 + i].value = w1 * (1.0f - wave) + w2 * wave;
		}	
	}
}

struct PO_Layout : ModuleWidget {
	PO_Layout(PO_101 *module) : ModuleWidget(module) {}
	void Layout() {
		addParam(ParamWidget::create<sub_knob_med>(Vec(66, 39), module, PO_101::PARAM_FINE, -1.0f, +1.0f, 0.0f));
		addParam(ParamWidget::create<sub_knob_med_snap_narrow>(Vec(121, 39), module, PO_101::PARAM_WAVE, 0.0f, +4.0f, 0.0f));

		addInput(Port::create<sub_port>(Vec(45,19), Port::INPUT, module, PO_101::INPUT_NOTE_CV));

		addOutput(Port::create<sub_port>(Vec(77.5,100), Port::OUTPUT, module, PO_101::OUTPUT_1));
		addOutput(Port::create<sub_port>(Vec(110,109), Port::OUTPUT, module, PO_101::OUTPUT_2));
		addOutput(Port::create<sub_port>(Vec(142.5,100), Port::OUTPUT, module, PO_101::OUTPUT_3));
		addOutput(Port::create<sub_port>(Vec(133.5,132.5), Port::OUTPUT, module, PO_101::OUTPUT_4));
		addOutput(Port::create<sub_port>(Vec(142.5,165), Port::OUTPUT, module, PO_101::OUTPUT_5));
		addOutput(Port::create<sub_port>(Vec(133.5,197.5), Port::OUTPUT, module, PO_101::OUTPUT_6));
		addOutput(Port::create<sub_port>(Vec(142.5,230), Port::OUTPUT, module, PO_101::OUTPUT_7));
		addOutput(Port::create<sub_port>(Vec(110,221), Port::OUTPUT, module, PO_101::OUTPUT_8));
		addOutput(Port::create<sub_port>(Vec(77.5,230), Port::OUTPUT, module, PO_101::OUTPUT_9));
		addOutput(Port::create<sub_port>(Vec(45,221), Port::OUTPUT, module, PO_101::OUTPUT_10));
		addOutput(Port::create<sub_port>(Vec(12.5,230), Port::OUTPUT, module, PO_101::OUTPUT_11));
		addOutput(Port::create<sub_port>(Vec(21.5,197.5), Port::OUTPUT, module, PO_101::OUTPUT_12));
		addOutput(Port::create<sub_port>(Vec(12.5,165), Port::OUTPUT, module, PO_101::OUTPUT_13));
		addOutput(Port::create<sub_port>(Vec(21.5,132.5), Port::OUTPUT, module, PO_101::OUTPUT_14));
		addOutput(Port::create<sub_port>(Vec(12.5,100), Port::OUTPUT, module, PO_101::OUTPUT_15));
		addOutput(Port::create<sub_port>(Vec(45,109), Port::OUTPUT, module, PO_101::OUTPUT_16));

		for (int i = 0; i < 4; i++) {
			addInput(Port::create<sub_port>(Vec(10 + 45 * i,260), Port::INPUT, module, PO_101::INPUT_PHASE_1 + i));
			addParam(ParamWidget::create<sub_knob_med>(Vec(3.5 + 45 * i, 290), module, PO_101::PARAM_PHASE_1 + i, -1.0f, +1.0f, 0.0f));
			addOutput(Port::create<sub_port>(Vec(10 + 45 * i,333), Port::OUTPUT, module, PO_101::OUTPUT_17 + i));
		}
	}
};

struct PO101 : PO_Layout {
	PO101(PO_101 *module) : PO_Layout(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/PO-101.svg")));
		addParam(ParamWidget::create<sub_knob_med>(Vec(11, 39), module, PO_101::PARAM_TUNE, -54.0f, +54.0f, 0.0f));
		Layout();
	}
};

struct PO102 : PO_Layout {
	PO102(PO_101 *module) : PO_Layout(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/PO-102.svg")));
		addParam(ParamWidget::create<sub_knob_med>(Vec(11, 39), module, PO_101::PARAM_TUNE, -96.0f, 72.0f, -12.0f));
		module->baseFreq = 1.0f;
		Layout();
	}
};

struct PO204 : ModuleWidget {
	PO204(PO_204 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/PO-204.svg")));
		addParam(ParamWidget::create<sub_knob_med>(Vec(60, 19), module, PO_204::PARAM_TUNE, -54.0f, +54.0f, 0.0f));
		addParam(ParamWidget::create<sub_knob_med>(Vec(105, 19), module, PO_204::PARAM_FINE, -1.0f, +1.0f, 0.0f));
		addInput(Port::create<sub_port>(Vec(17.5, 25.5), Port::INPUT, module, PO_204::INPUT_TUNE));

		for (int i = 0; i < 4; i++) {
			addParam(ParamWidget::create<sub_knob_small>(Vec(5, 89 + 70 * i), module, PO_204::PARAM_WAVE_1 + i, 0.0f, 10.0f, 5.0f));
			addParam(ParamWidget::create<sub_knob_small>(Vec(45, 89 + 70 * i), module, PO_204::PARAM_PHASE_1 + i, -1.0f, +1.0f, 0.0f));
			addParam(ParamWidget::create<sub_knob_small_snap>(Vec(85, 89 + 70 * i), module, PO_204::PARAM_MULT_1 + i, 1.0f, 16.0f, 1.0f));
			addInput(Port::create<sub_port>(Vec(4.5, 125 + 70 * i), Port::INPUT, module, PO_204::INPUT_WAVE_1 + i));
			addInput(Port::create<sub_port>(Vec(44.5, 125 + 70 * i), Port::INPUT, module, PO_204::INPUT_PHASE_1 + i));
			addOutput(Port::create<sub_port>(Vec(120.5, 125 + 70 * i), Port::OUTPUT, module, PO_204::OUTPUT_1 + i));
		}
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, PO101) {
   Model *modelPO101 = Model::create<PO_101, PO101>("SubmarineFree", "PO-101", "PO-101 Phased VCO", OSCILLATOR_TAG, MULTIPLE_TAG, DIGITAL_TAG);
   return modelPO101;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, PO102) {
   Model *modelPO102 = Model::create<PO_101, PO102>("SubmarineFree", "PO-102", "PO-102 Phased LFO", OSCILLATOR_TAG, MULTIPLE_TAG, DIGITAL_TAG);
   return modelPO102;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, PO204) {
   Model *modelPO204 = Model::create<PO_204, PO204>("SubmarineFree", "PO-204", "PO-204 Phase Modulation Engine", OSCILLATOR_TAG, QUAD_TAG, DIGITAL_TAG);
   return modelPO204;
}
