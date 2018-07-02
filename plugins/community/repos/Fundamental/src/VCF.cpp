/*
The filter DSP code has been derived from
Miller Puckette's code hosted at
https://github.com/ddiakopoulos/MoogLadders/blob/master/src/RKSimulationModel.h
which is licensed for use under the following terms (MIT license):


Copyright (c) 2015, Miller Puckette. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "Fundamental.hpp"


// The clipping function of a transistor pair is approximately tanh(x)
// TODO: Put this in a lookup table. 5th order approx doesn't seem to cut it
inline float clip(float x) {
	return tanhf(x);
}

struct LadderFilter {
	float cutoff = 1000.0f;
	float resonance = 1.0f;
	float state[4] = {};

	void calculateDerivatives(float input, float *dstate, const float *state) {
		float cutoff2Pi = 2*M_PI * cutoff;

		float satstate0 = clip(state[0]);
		float satstate1 = clip(state[1]);
		float satstate2 = clip(state[2]);

		dstate[0] = cutoff2Pi * (clip(input - resonance * state[3]) - satstate0);
		dstate[1] = cutoff2Pi * (satstate0 - satstate1);
		dstate[2] = cutoff2Pi * (satstate1 - satstate2);
		dstate[3] = cutoff2Pi * (satstate2 - clip(state[3]));
	}

	void process(float input, float dt) {
		float deriv1[4], deriv2[4], deriv3[4], deriv4[4], tempState[4];

		calculateDerivatives(input, deriv1, state);
		for (int i = 0; i < 4; i++)
			tempState[i] = state[i] + 0.5f * dt * deriv1[i];

		calculateDerivatives(input, deriv2, tempState);
		for (int i = 0; i < 4; i++)
			tempState[i] = state[i] + 0.5f * dt * deriv2[i];

		calculateDerivatives(input, deriv3, tempState);
		for (int i = 0; i < 4; i++)
			tempState[i] = state[i] + dt * deriv3[i];

		calculateDerivatives(input, deriv4, tempState);
		for (int i = 0; i < 4; i++)
			state[i] += (1.0f / 6.0f) * dt * (deriv1[i] + 2.0f * deriv2[i] + 2.0f * deriv3[i] + deriv4[i]);
	}
	void reset() {
		for (int i = 0; i < 4; i++) {
			state[i] = 0.0f;
		}
	}
};


struct VCF : Module {
	enum ParamIds {
		FREQ_PARAM,
		FINE_PARAM,
		RES_PARAM,
		FREQ_CV_PARAM,
		DRIVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_INPUT,
		RES_INPUT,
		DRIVE_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LPF_OUTPUT,
		HPF_OUTPUT,
		NUM_OUTPUTS
	};

	LadderFilter filter;

	VCF() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
	void onReset() override {
		filter.reset();
	}
};


void VCF::step() {
	float input = inputs[IN_INPUT].value / 5.0f;
	float drive = params[DRIVE_PARAM].value + inputs[DRIVE_INPUT].value / 10.0f;
	float gain = powf(100.0f, drive);
	input *= gain;
	// Add -60dB noise to bootstrap self-oscillation
	input += 1e-6f * (2.0f*randomUniform() - 1.0f);

	// Set resonance
	float res = params[RES_PARAM].value + inputs[RES_INPUT].value / 5.0f;
	res = 5.5f * clamp(res, 0.0f, 1.0f);
	filter.resonance = res;

	// Set cutoff frequency
	float cutoffExp = params[FREQ_PARAM].value + params[FREQ_CV_PARAM].value * inputs[FREQ_INPUT].value / 5.0f;
	cutoffExp = clamp(cutoffExp, 0.0f, 1.0f);
	const float minCutoff = 15.0f;
	const float maxCutoff = 8400.0f;
	filter.cutoff = minCutoff * powf(maxCutoff / minCutoff, cutoffExp);

	// Push a sample to the state filter
	filter.process(input, 1.0f/engineGetSampleRate());

	// Set outputs
	outputs[LPF_OUTPUT].value = 5.0f * filter.state[3];
	outputs[HPF_OUTPUT].value = 5.0f * (input - filter.state[3]);
}


struct VCFWidget : ModuleWidget {
	VCFWidget(VCF *module);
};

VCFWidget::VCFWidget(VCF *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/VCF.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(33, 61), module, VCF::FREQ_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(12, 143), module, VCF::FINE_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(71, 143), module, VCF::RES_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(12, 208), module, VCF::FREQ_CV_PARAM, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(71, 208), module, VCF::DRIVE_PARAM, 0.0f, 1.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(Vec(10, 276), Port::INPUT, module, VCF::FREQ_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(48, 276), Port::INPUT, module, VCF::RES_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(85, 276), Port::INPUT, module, VCF::DRIVE_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10, 320), Port::INPUT, module, VCF::IN_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(48, 320), Port::OUTPUT, module, VCF::LPF_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(85, 320), Port::OUTPUT, module, VCF::HPF_OUTPUT));
}


RACK_PLUGIN_MODEL_INIT(Fundamental, VCF) {
   Model *modelVCF = Model::create<VCF, VCFWidget>("Fundamental", "VCF", "VCF", FILTER_TAG);
   return modelVCF;
}
