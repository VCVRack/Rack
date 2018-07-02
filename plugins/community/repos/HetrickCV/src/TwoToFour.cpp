#include "HetrickCV.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_HetrickCV {

#ifdef USE_VST2
#define plugin "HetrickCV"
#endif // USE_VST2


struct TwoToFour : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
        INA_INPUT,
        INB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        OUT3_OUTPUT,
        OUT4_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds
    {
        OUT1_POS_LIGHT, OUT1_NEG_LIGHT,
		OUT2_POS_LIGHT, OUT2_NEG_LIGHT,
		OUT3_POS_LIGHT, OUT3_NEG_LIGHT,
		OUT4_POS_LIGHT, OUT4_NEG_LIGHT,
        NUM_LIGHTS
	};

    float outs[4] = {};

	TwoToFour() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

	}

	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void TwoToFour::step()
{
    const float inA = inputs[INA_INPUT].value;
    const float inB = inputs[INB_INPUT].value;

    outs[0] = inA + inB;
    outs[1] = outs[0] * -1.0f;
    outs[3] = inA - inB;
    outs[2] = outs[3] * -1.0f;

    outputs[OUT1_OUTPUT].value = outs[0];
    outputs[OUT2_OUTPUT].value = outs[1];
    outputs[OUT3_OUTPUT].value = outs[2];
	outputs[OUT4_OUTPUT].value = outs[3];

	lights[OUT1_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, outs[0] / 5.0));
    lights[OUT1_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -outs[0] / 5.0));

    lights[OUT2_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, outs[1] / 5.0));
    lights[OUT2_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -outs[1] / 5.0));

    lights[OUT3_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, outs[2] / 5.0));
    lights[OUT3_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -outs[2] / 5.0));

    lights[OUT4_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, outs[3] / 5.0));
    lights[OUT4_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -outs[3] / 5.0));
}

struct TwoToFourWidget : ModuleWidget { TwoToFourWidget(TwoToFour *module); };

TwoToFourWidget::TwoToFourWidget(TwoToFour *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/2To4.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////PARAMS//////

    //////INPUTS//////
    addInput(Port::create<PJ301MPort>(Vec(10, 100), Port::INPUT, module, TwoToFour::INA_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(55, 100), Port::INPUT, module, TwoToFour::INB_INPUT));

    for(int i = 0; i < 4; i++)
    {
        const int yPos = i*45;
        addOutput(Port::create<PJ301MPort>(Vec(33, 150 + yPos), Port::OUTPUT, module, TwoToFour::OUT1_OUTPUT + i));
        addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(70, 158 + yPos), module, TwoToFour::OUT1_POS_LIGHT + i*2));
    }
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, TwoToFour) {
   Model *modelTwoToFour = Model::create<TwoToFour, TwoToFourWidget>("HetrickCV", "2To4", "2 To 4 Mix Matrix", MIXER_TAG);
   return modelTwoToFour;
}
