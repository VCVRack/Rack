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


#include "Gratrix.hpp"

namespace rack_plugin_Gratrix {


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


//============================================================================================================

struct VCF : MicroModule {
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

	VCF() : MicroModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step();
	void onReset() {
		filter.reset();
	}
};


//============================================================================================================

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


//============================================================================================================

struct VCFBank : Module
{
	std::array<VCF, GTX__N> inst;

	VCFBank() : Module(VCF::NUM_PARAMS, (GTX__N+1) * VCF::NUM_INPUTS, GTX__N * VCF::NUM_OUTPUTS) {}

	static std::size_t imap(std::size_t port, std::size_t bank)
	{
		return port + bank * VCF::NUM_INPUTS;
	}

	static std::size_t omap(std::size_t port, std::size_t bank)
	{
		return port + bank * VCF::NUM_OUTPUTS;
	}

	void step() override
	{
		for (std::size_t i=0; i<GTX__N; ++i)
		{
			for (std::size_t p=0; p<VCF::NUM_PARAMS;  ++p) inst[i].params[p]  = params[p];
			for (std::size_t p=0; p<VCF::NUM_INPUTS;  ++p) inst[i].inputs[p]  = inputs[imap(p, i)].active ? inputs[imap(p, i)] : inputs[imap(p, GTX__N)];
			for (std::size_t p=0; p<VCF::NUM_OUTPUTS; ++p) inst[i].outputs[p] = outputs[omap(p, i)];

			inst[i].step();

			for (std::size_t p=0; p<VCF::NUM_OUTPUTS; ++p) outputs[omap(p, i)].value = inst[i].outputs[p].value;
		}
	}

	void onReset() override
	{
		for (std::size_t i=0; i<GTX__N; ++i)
		{
			inst[i].onReset();
		}
	}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget_VCF : ModuleWidget
{
	GtxWidget_VCF(VCFBank *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(18*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/VCF-F1.svg"), box.size, "VCF-F1");

			pg.nob_big(0, 0, "FREQ");

			pg.nob_med(1.1, -0.28, "FINE");     pg.nob_med(1.9, -0.28, "RES");
			pg.nob_med(1.1, +0.28, "FREQ  CV"); pg.nob_med(1.9, +0.28, "DRIVE");

			pg.bus_in(0, 1, "FREQ"); pg.bus_in(1, 1, "RES");   pg.bus_out(2, 1, "HPF");
			pg.bus_in(0, 2, "IN");   pg.bus_in(1, 2, "DRIVE"); pg.bus_out(2, 2, "LPF");
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/VCF-F1.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

		addParam(createParamGTX<KnobFreeHug>(Vec(fx(0.0), fy(+0.00)), module, VCF::FREQ_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(createParamGTX<KnobFreeMed>(Vec(fx(1.1), fy(-0.28)), module, VCF::FINE_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(createParamGTX<KnobFreeMed>(Vec(fx(1.9), fy(-0.28)), module, VCF::RES_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(createParamGTX<KnobFreeMed>(Vec(fx(1.1), fy(+0.28)), module, VCF::FREQ_CV_PARAM, -1.0f, 1.0f, 0.0f));
		addParam(createParamGTX<KnobFreeMed>(Vec(fx(1.9), fy(+0.28)), module, VCF::DRIVE_PARAM, 0.0f, 1.0f, 0.0f));

		for (std::size_t i=0; i<GTX__N; ++i)
		{
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(1, i)), module, VCFBank::imap(VCF::FREQ_INPUT,  i)));
			addInput(createInputGTX<PortInMed>(Vec(px(1, i), py(1, i)), module, VCFBank::imap(VCF::RES_INPUT,   i)));
			addInput(createInputGTX<PortInMed>(Vec(px(1, i), py(2, i)), module, VCFBank::imap(VCF::DRIVE_INPUT, i)));
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(2, i)), module, VCFBank::imap(VCF::IN_INPUT,    i)));

			addOutput(createOutputGTX<PortOutMed>(Vec(px(2, i), py(2, i)), module, VCFBank::omap(VCF::LPF_OUTPUT,  i)));
			addOutput(createOutputGTX<PortOutMed>(Vec(px(2, i), py(1, i)), module, VCFBank::omap(VCF::HPF_OUTPUT,  i)));
		}

		addInput(createInputGTX<PortInMed>(Vec(gx(0), gy(1)), module, VCFBank::imap(VCF::FREQ_INPUT,  GTX__N)));
		addInput(createInputGTX<PortInMed>(Vec(gx(1), gy(1)), module, VCFBank::imap(VCF::RES_INPUT,   GTX__N)));
		addInput(createInputGTX<PortInMed>(Vec(gx(1), gy(2)), module, VCFBank::imap(VCF::DRIVE_INPUT, GTX__N)));
		addInput(createInputGTX<PortInMed>(Vec(gx(0), gy(2)), module, VCFBank::imap(VCF::IN_INPUT,    GTX__N)));
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, VCF_F1) {
   Model *model = Model::create<VCFBank, GtxWidget_VCF>("Gratrix", "VCF-F1", "VCF-F1", FILTER_TAG);
   return model;
}
