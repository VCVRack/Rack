#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

#ifdef USE_VST2
#define plugin "HetrickCV"
#endif // USE_VST2

struct Waveshape : Module
{
	enum ParamIds
	{
		AMOUNT_PARAM,
        SCALE_PARAM,
        RANGE_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
        MAIN_INPUT,
        AMOUNT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};

	Waveshape() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{

	}

	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Waveshape::step()
{
	float input = inputs[MAIN_INPUT].value;

    bool mode5V = (params[RANGE_PARAM].value == 0.0f);
    if(mode5V) input = clampf(input, -5.0f, 5.0f) * 0.2f;
	else input = clampf(input, -10.0f, 10.0f) * 0.1f;

	float shape = params[AMOUNT_PARAM].value + (inputs[AMOUNT_INPUT].value * params[SCALE_PARAM].value);
	shape = clampf(shape, -5.0f, 5.0f) * 0.2f;
	shape *= 0.99f;

	const float shapeB = (1.0 - shape) / (1.0 + shape);
	const float shapeA = (4.0 * shape) / ((1.0 - shape) * (1.0 + shape));

	float output = input * (shapeA + shapeB);
	output = output / ((std::abs(input) * shapeA) + shapeB);

    if(mode5V) output *= 5.0f;
    else output *= 10.0f;

    outputs[MAIN_OUTPUT].value = output;
}

struct CKSSRot : SVGSwitch, ToggleSwitch {
	CKSSRot() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_rot_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_rot_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};


struct WaveshapeWidget : ModuleWidget { WaveshapeWidget(Waveshape *module); };

WaveshapeWidget::WaveshapeWidget(Waveshape *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Waveshape.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	//////PARAMS//////
	addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(27, 62), module, Waveshape::AMOUNT_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Trimpot>(Vec(36, 112), module, Waveshape::SCALE_PARAM, -1.0, 1.0, 1.0));
    addParam(ParamWidget::create<CKSSRot>(Vec(35, 200), module, Waveshape::RANGE_PARAM, 0.0, 1.0, 0.0));

	//////INPUTS//////
    addInput(Port::create<PJ301MPort>(Vec(33, 235), Port::INPUT, module, Waveshape::MAIN_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(33, 145), Port::INPUT, module, Waveshape::AMOUNT_INPUT));

	//////OUTPUTS//////
	addOutput(Port::create<PJ301MPort>(Vec(33, 285), Port::OUTPUT, module, Waveshape::MAIN_OUTPUT));
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, Waveshape) {
   Model *modelWaveshape = Model::create<Waveshape, WaveshapeWidget>("HetrickCV", "Waveshaper", "Waveshaper", WAVESHAPER_TAG, DISTORTION_TAG, EFFECT_TAG);
   return modelWaveshape;
}
