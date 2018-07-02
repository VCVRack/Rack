#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

struct Dust : Module
{
	enum ParamIds
	{
		RATE_PARAM,
		BIPOLAR_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		RATE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		DUST_OUTPUT,
		NUM_OUTPUTS
	};

	float lastDensity = 0.0;
	float densityScaled = 0.0;
	float threshold = 0.0;

	Dust() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{

	}

	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Dust::step()
{
	float densityInput = params[RATE_PARAM].value + inputs[RATE_INPUT].value;

	if(lastDensity != densityInput)
	{
		densityScaled = clampf(densityInput, 0.0f, 4.0f) / 4.0f;
		densityScaled = engineGetSampleRate() * powf(densityScaled, 3.0f);
		lastDensity = densityInput;
		threshold = (1.0/engineGetSampleRate()) * densityScaled;
	}

	const float noiseValue = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	if (noiseValue < threshold)
	{
		const bool bipolar = (params[BIPOLAR_PARAM].value == 0.0);

		if(bipolar)
		{
			const float scale = (threshold > 0.0f) ? 2.0f/threshold : 0.0f;
			outputs[DUST_OUTPUT].value = clampf((noiseValue * scale - 1.0f) * 5.0f, -5.0f, 5.0f);
		}
		else
		{
			const float scale = (threshold > 0.0f) ? 1.0f/threshold : 0.0f;
			outputs[DUST_OUTPUT].value = clampf(noiseValue * scale * 5.0f, 5.0f, 5.0f);
		}
	}
	else
	{
		outputs[DUST_OUTPUT].value = 0.0;
	}
}

struct DustWidget : ModuleWidget { DustWidget(Dust *module); };

DustWidget::DustWidget(Dust *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Dust.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	//////PARAMS//////
	addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(28, 87), module, Dust::RATE_PARAM, 0, 4.0, 0.0));
	addParam(ParamWidget::create<CKSS>(Vec(37, 220), module, Dust::BIPOLAR_PARAM, 0.0, 1.0, 0.0));

	//////INPUTS//////
	addInput(Port::create<PJ301MPort>(Vec(33, 146), Port::INPUT, module, Dust::RATE_INPUT));

	//////OUTPUTS//////
	addOutput(Port::create<PJ301MPort>(Vec(33, 285), Port::OUTPUT, module, Dust::DUST_OUTPUT));
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, Dust) {
   Model *modelDust = Model::create<Dust, DustWidget>("HetrickCV", "Dust", "Dust", NOISE_TAG, GRANULAR_TAG);
   return modelDust;
}
