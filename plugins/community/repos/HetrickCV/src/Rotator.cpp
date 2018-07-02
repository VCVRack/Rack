#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

#ifdef USE_VST2
#define plugin "HetrickCV"
#endif // USE_VST2

struct Rotator : Module
{
	enum ParamIds
	{
        ROTATE_PARAM,
        STAGES_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
        IN1_INPUT,
        IN2_INPUT,
        IN3_INPUT,
        IN4_INPUT,
        IN5_INPUT,
        IN6_INPUT,
        IN7_INPUT,
        IN8_INPUT,

        ROTATE_INPUT,
        STAGES_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        OUT3_OUTPUT,
        OUT4_OUTPUT,
        OUT5_OUTPUT,
        OUT6_OUTPUT,
        OUT7_OUTPUT,
        OUT8_OUTPUT,
		NUM_OUTPUTS
    };
    enum LightIds
	{
        IN1_POS_LIGHT, IN1_NEG_LIGHT,
        IN2_POS_LIGHT, IN2_NEG_LIGHT,
        IN3_POS_LIGHT, IN3_NEG_LIGHT,
        IN4_POS_LIGHT, IN4_NEG_LIGHT,
        IN5_POS_LIGHT, IN5_NEG_LIGHT,
        IN6_POS_LIGHT, IN6_NEG_LIGHT,
        IN7_POS_LIGHT, IN7_NEG_LIGHT,
        IN8_POS_LIGHT, IN8_NEG_LIGHT,

        OUT1_POS_LIGHT, OUT1_NEG_LIGHT,
        OUT2_POS_LIGHT, OUT2_NEG_LIGHT,
        OUT3_POS_LIGHT, OUT3_NEG_LIGHT,
        OUT4_POS_LIGHT, OUT4_NEG_LIGHT,
        OUT5_POS_LIGHT, OUT5_NEG_LIGHT,
        OUT6_POS_LIGHT, OUT6_NEG_LIGHT,
        OUT7_POS_LIGHT, OUT7_NEG_LIGHT,
        OUT8_POS_LIGHT, OUT8_NEG_LIGHT,

		NUM_LIGHTS
	};

    float ins[8] = {};
    float outs[8] = {};

	Rotator() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

	}

    void step() override;

    int clampInt(const int _in, const int min = 0, const int max = 7)
    {
        if (_in > max) return max;
        if (_in < min) return min;
        return _in;
    }

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Rotator::step()
{
    int rotation = round(params[ROTATE_PARAM].value + inputs[ROTATE_INPUT].value);
    int stages = round(params[STAGES_PARAM].value + inputs[STAGES_INPUT].value);

    rotation = clampInt(rotation);
    stages = clampInt(stages) + 1;

    for(int i = 0; i < 8; i++)
    {
        const int input = (rotation + i) % stages;
        outputs[i].value = inputs[input].value;

        lights[IN1_POS_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, inputs[i].value / 5.0));
		lights[IN1_NEG_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, inputs[i].value / -5.0));

        lights[OUT1_POS_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, outputs[i].value / 5.0));
		lights[OUT1_NEG_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, outputs[i].value / -5.0));
    }
}


struct RotatorWidget : ModuleWidget { RotatorWidget(Rotator *module); };

RotatorWidget::RotatorWidget(Rotator *module) : ModuleWidget(module)
{
	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Rotator.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////PARAMS//////
    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(70, 85), module, Rotator::ROTATE_PARAM, 0, 7.0, 0.0));
    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(70, 245), module, Rotator::STAGES_PARAM, 0, 7.0, 7.0));

    addInput(Port::create<PJ301MPort>(Vec(75, 150), Port::INPUT, module, Rotator::ROTATE_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(75, 310), Port::INPUT, module, Rotator::STAGES_INPUT));

    const int inXPos = 10;
    const int outXPos = 145;
    const int inLightX = 50;
    const int outLightX = 120;
    for(int i = 0; i < 8; i++)
    {
        const int yPos = 50 + (40 * i);
        const int lightY = 59 + (40 * i);

        //////INPUTS//////
        addInput(Port::create<PJ301MPort>(Vec(inXPos, yPos), Port::INPUT, module, i));

        //////OUTPUTS//////
        addOutput(Port::create<PJ301MPort>(Vec(outXPos, yPos), Port::OUTPUT, module, i));

        //////BLINKENLIGHTS//////
        addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(inLightX, lightY), module, Rotator::IN1_POS_LIGHT + 2*i));
        addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(outLightX, lightY), module, Rotator::OUT1_POS_LIGHT + 2*i));
    }
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, Rotator) {
   Model *modelRotator = Model::create<Rotator, RotatorWidget>("HetrickCV", "Rotator", "Rotator", SWITCH_TAG);
   return modelRotator;
}
