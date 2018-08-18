#include "Nohmad.hpp"

#include <dsp/filter.hpp>

#include <random>
#include <cmath>

namespace rack_plugin_Nohmad {

struct NoiseGenerator {
	std::mt19937 rng;
	std::uniform_real_distribution<float> uniform;

	NoiseGenerator() : uniform(-1.0f, 1.0f) {
		rng.seed(std::random_device()());
	}

	float white() {
		return uniform(rng);
	}
};

struct PinkFilter {
	float b0, b1, b2, b3, b4, b5, b6; // Coefficients
	float y; // Out

	void process(float x) {
		b0 = 0.99886f * b0 + x * 0.0555179f;
		b1 = 0.99332f * b1 + x * 0.0750759f;
		b2 = 0.96900f * b2 + x * 0.1538520f;
		b3 = 0.86650f * b3 + x * 0.3104856f;
		b4 = 0.55000f * b4 + x * 0.5329522f;
		b5 = -0.7616f * b5 - x * 0.0168980f;
		y = b0 + b1 + b2 + b3 + b4 + b5 + b6 + x * 0.5362f;
		b6 = x * 0.115926f;
	}

	float pink() {
		return y;
	}
};

struct NotchFilter {
	float freq, bandwidth; // Params
	float a0, a1, a2, b1, b2; // Coefficients
	float x1, x2; // In
	float y1, y2; // out

	void setFreq(float value) {
		freq = value;
		computeCoefficients();
	}

	void setBandwidth(float value) {
		bandwidth = value;
		computeCoefficients();
	}

	void process(float x) {
		float y = a0 * x + a1 * x1 + a2 * x2 + b1 * y1 + b2 * y2;

		x2 = x1;
		x1 = x;
		y2 = y1;
		y1 = y;
	}

	float notch() {
		return y1;
	}

	void computeCoefficients() {
		float c2pf = cos(2.0f * M_PI * freq);
		float r = 1.0f - 3.0f * bandwidth;
		float r2 = r * r;
		float k = (1.0f - (2.0f * r * c2pf) + r2) / (2.0f - 2.0f * c2pf);

		a0 = k;
		a1 = -2.0f * k * c2pf;
		a2 = k;
		b1 = 2.0f * r * c2pf;
		b2 = -r2;
	}
};

struct Noise : Module {
	enum ParamIds {
		QUANTA_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		NUM_INPUTS
	};

	enum OutputIds {
		WHITE_OUTPUT,
		PINK_OUTPUT,
		RED_OUTPUT,
		GREY_OUTPUT,
		BLUE_OUTPUT,
		PURPLE_OUTPUT,
		QUANTA_OUTPUT,
		NUM_OUTPUTS
	};

	NoiseGenerator noise;

	PinkFilter pinkFilter;
	RCFilter redFilter;
	NotchFilter greyFilter;
	RCFilter blueFilter;
	RCFilter purpleFilter;

	Noise() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		redFilter.setCutoff(441.0f / engineGetSampleRate());
		purpleFilter.setCutoff(44100.0f / engineGetSampleRate());
		blueFilter.setCutoff(44100.0f / engineGetSampleRate());
		greyFilter.setFreq(1000.0f / engineGetSampleRate());
		greyFilter.setBandwidth(0.3f);
	}

	void step() override;
};

void Noise::step() {
	float white = noise.white();
	if (outputs[PINK_OUTPUT].active || outputs[BLUE_OUTPUT].active || outputs[GREY_OUTPUT].active) {
		pinkFilter.process(white);
	}

	if (outputs[WHITE_OUTPUT].active) {
		outputs[WHITE_OUTPUT].value = 5.0f * white;
	}

	if (outputs[RED_OUTPUT].active) {
		redFilter.process(white);
		outputs[RED_OUTPUT].value = 5.0f * clamp(7.8f * redFilter.lowpass(), -1.0f, 1.0f);
	}

	if (outputs[PINK_OUTPUT].active) {
		outputs[PINK_OUTPUT].value = 5.0f * clamp(0.18f * pinkFilter.pink(), -1.0f, 1.0f);
	}

	if (outputs[GREY_OUTPUT].active) {
		greyFilter.process(pinkFilter.pink() * 0.034);
		outputs[GREY_OUTPUT].value = 5.0f * clamp(0.23f * (pinkFilter.pink() * 0.5f + greyFilter.notch() * 0.5f), -1.0f, 1.0f);
	}

	if (outputs[BLUE_OUTPUT].active) {
		blueFilter.process(pinkFilter.pink());
		outputs[BLUE_OUTPUT].value = 5.0f * clamp(0.64f * blueFilter.highpass(), -1.0f, 1.0f);
	}

	if (outputs[PURPLE_OUTPUT].active) {
		purpleFilter.process(white);
		outputs[PURPLE_OUTPUT].value = 5.0f * clamp(0.82f * purpleFilter.highpass(), -1.0f, 1.0f);
	}

	if (outputs[QUANTA_OUTPUT].active) {
		outputs[QUANTA_OUTPUT].value = abs(white) <= params[QUANTA_PARAM].value ? 5.0f * sgn(white) : 0.0f;
	}
}

struct MiniTrimpot : Trimpot  {
	MiniTrimpot() {
		box.size = Vec(12, 12);
	}
};


struct NoiseWidget : ModuleWidget {
	NoiseWidget(Noise *module);
};

NoiseWidget::NoiseWidget(Noise *module) : ModuleWidget(module) {
	box.size = Vec(15 * 3, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Noise.svg")));
		addChild(panel);
	}

	addOutput(Port::create<PJ301MPort>(Vec(10.5, 55), Port::OUTPUT, module, Noise::WHITE_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10.5, 101), Port::OUTPUT, module, Noise::PINK_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10.5, 150), Port::OUTPUT, module, Noise::RED_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10.5, 199), Port::OUTPUT, module, Noise::GREY_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10.5, 247), Port::OUTPUT, module, Noise::BLUE_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10.5, 295), Port::OUTPUT, module, Noise::PURPLE_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10.5, 343), Port::OUTPUT, module, Noise::QUANTA_OUTPUT));

	addParam(ParamWidget::create<MiniTrimpot>(Vec(30, 365), module, Noise::QUANTA_PARAM, 0.0f, 1.0f, 0.066f));
}

} // namespace rack_plugin_Nohmad

using namespace rack_plugin_Nohmad;

RACK_PLUGIN_MODEL_INIT(Nohmad, Noise) {
   Model *modelNoise = Model::create<Noise, NoiseWidget>("Nohmad", "Noise", "Noise", NOISE_TAG);
   return modelNoise;
}
