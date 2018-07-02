#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

struct ASR : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
        MAIN_INPUT,
        CLK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
        STAGE1_OUTPUT,
        STAGE2_OUTPUT,
        STAGE3_OUTPUT,
        STAGE4_OUTPUT,
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

    SchmittTrigger clockTrigger;
    float stages[4] = {};

	ASR() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

	}

	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void ASR::step()
{
    if (clockTrigger.process(inputs[CLK_INPUT].value))
    {
        stages[3] = stages[2];
        stages[2] = stages[1];
        stages[1] = stages[0];
        stages[0] = inputs[MAIN_INPUT].value;
    }

    outputs[STAGE1_OUTPUT].value = stages[0];
    outputs[STAGE2_OUTPUT].value = stages[1];
    outputs[STAGE3_OUTPUT].value = stages[2];
    outputs[STAGE4_OUTPUT].value = stages[3];

    lights[OUT1_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, stages[0] / 5.0));
    lights[OUT1_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -stages[0] / 5.0));

    lights[OUT2_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, stages[1] / 5.0));
    lights[OUT2_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -stages[1] / 5.0));

    lights[OUT3_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, stages[2] / 5.0));
    lights[OUT3_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -stages[2] / 5.0));

    lights[OUT4_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, stages[3] / 5.0));
    lights[OUT4_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -stages[3] / 5.0));
}

struct ASRWidget : ModuleWidget { ASRWidget(ASR *module); };

ASRWidget::ASRWidget(ASR *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/ASR.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////PARAMS//////

    //////INPUTS//////
    addInput(Port::create<PJ301MPort>(Vec(10, 100), Port::INPUT, module, ASR::MAIN_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(55, 100), Port::INPUT, module, ASR::CLK_INPUT));

    for(int i = 0; i < 4; i++)
    {
        const int yPos = i*45;
        addOutput(Port::create<PJ301MPort>(Vec(33, 150 + yPos), Port::OUTPUT, module, ASR::STAGE1_OUTPUT + i));
        addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(70, 158 + yPos), module, ASR::OUT1_POS_LIGHT + i*2));
    }
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, ASR) {
   Model *modelASR = Model::create<ASR, ASRWidget>("HetrickCV", "ASR", "ASR", SEQUENCER_TAG);
   return modelASR;
}
