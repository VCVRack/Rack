#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

#ifdef USE_VST2
#define plugin "HetrickCV"
#endif // USE_VST2

struct Scanner : Module
{
	enum ParamIds
	{
        SCAN_PARAM,
        STAGES_PARAM,
        WIDTH_PARAM,
        SLOPE_PARAM,
        OFFSET_PARAM,
        MIXSCALE_PARAM,
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

        SCAN_INPUT,
        STAGES_INPUT,
        WIDTH_INPUT,
        SLOPE_INPUT,
        ALLIN_INPUT,
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
        MIX_OUTPUT,
		NUM_OUTPUTS
    };
    enum LightIds
	{
        IN1_LIGHT,
        IN2_LIGHT,
        IN3_LIGHT,
        IN4_LIGHT,
        IN5_LIGHT,
        IN6_LIGHT,
        IN7_LIGHT,
        IN8_LIGHT,

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
    float inMults[8] = {};
    float widthTable[9] = {0.f, 0.f, 0.f, 0.285f, 0.285f, 0.2608f, 0.23523f, 0.2125f, 0.193f};

	Scanner() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

	}

    void step() override;

    int clampInt(const int _in, const int min = 0, const int max = 7)
    {
        if (_in > max) return max;
        if (_in < min) return min;
        return _in;
    }

    float triShape(float _in)
    {
        _in = _in - round(_in);
        return std::abs(_in + _in);
    }

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Scanner::step()
{
    float allInValue = 0.0f;
    if(inputs[ALLIN_INPUT].active) allInValue = inputs[ALLIN_INPUT].value;
    else if(params[OFFSET_PARAM].value != 0.0f) allInValue = 5.0f;

    for(int i = 0; i < 8; i++)
    {
        if(!inputs[IN1_INPUT + i].active) ins[i] = allInValue;
        else ins[i] = inputs[IN1_INPUT + i].value;
    }

    int stages = round(params[STAGES_PARAM].value + inputs[STAGES_INPUT].value);
    stages = clampInt(stages, 0, 6) + 2;
    const float invStages = 1.0f/stages;
    const float halfStages = stages * 0.5f;
    const float remainInvStages = 1.0f - invStages;

    float widthControl = params[WIDTH_PARAM].value + inputs[WIDTH_INPUT].value;
    widthControl = clampf(widthControl, 0.0f, 5.0f) * 0.2f;
    widthControl = widthControl * widthControl * widthTable[stages];

    float scanControl = params[SCAN_PARAM].value + inputs[SCAN_INPUT].value;
    scanControl = clampf(scanControl, 0.0f, 5.0f) * 0.2f;

    float slopeControl = params[SLOPE_PARAM].value + inputs[SLOPE_INPUT].value;
    slopeControl = clampf(slopeControl, 0.0f, 5.0f) * 0.2f;

    float scanFactor1 = LERP(widthControl, halfStages, invStages);
    float scanFactor2 = LERP(widthControl, halfStages + remainInvStages, 1.0f);
    float scanFinal = LERP(scanControl, scanFactor2, scanFactor1);

    float invWidth = 1.0f/(LERP(widthControl, float(stages), invStages+invStages));

    float subStage = 0.0f;
    for(int i = 0; i < 8; i++)
    {
        inMults[i] = (scanFinal + subStage) * invWidth;
        subStage = subStage - invStages;
    }

    for(int i = 0; i < 8; i++)
    {
        inMults[i] = clampf(inMults[i], 0.0f, 1.0f);
        inMults[i] = triShape(inMults[i]);
        inMults[i] = clampf(inMults[i], 0.0f, 1.0f);

        const float shaped = (2.0f - inMults[i]) * inMults[i];
        inMults[i] = LERP(slopeControl, shaped, inMults[i]);
    }

    outputs[MIX_OUTPUT].value = 0.0f;

    for(int i = 0; i < 8; i++)
    {
        outputs[i].value = ins[i] * inMults[i];

        lights[IN1_LIGHT + i].setBrightnessSmooth(fmaxf(0.0, inMults[i]));

        lights[OUT1_POS_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, outputs[i].value / 5.0));
        lights[OUT1_NEG_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, outputs[i].value / -5.0));
        outputs[MIX_OUTPUT].value = outputs[MIX_OUTPUT].value + outputs[i].value;
    }

    outputs[MIX_OUTPUT].value = outputs[MIX_OUTPUT].value * params[MIXSCALE_PARAM].value;
}


struct ScannerWidget : ModuleWidget { ScannerWidget(Scanner *module); };

ScannerWidget::ScannerWidget(Scanner *module) : ModuleWidget(module)
{
	box.size = Vec(18 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Scanner.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    const int knobX = 75;
    const int jackX = 123;

    //////PARAMS//////
    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(knobX, 65), module, Scanner::SCAN_PARAM, 0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(jackX, 70), Port::INPUT, module, Scanner::SCAN_INPUT));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(knobX, 125), module, Scanner::STAGES_PARAM, 0, 6.0, 6.0));
    addInput(Port::create<PJ301MPort>(Vec(jackX, 130), Port::INPUT, module, Scanner::STAGES_INPUT));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(knobX, 185), module, Scanner::WIDTH_PARAM, 0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(jackX, 190), Port::INPUT, module, Scanner::WIDTH_INPUT));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(knobX, 245), module, Scanner::SLOPE_PARAM, 0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(jackX, 250), Port::INPUT, module, Scanner::SLOPE_INPUT));

    addInput(Port::create<PJ301MPort>(Vec(96, 310), Port::INPUT, module, Scanner::ALLIN_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(141, 310), Port::OUTPUT, module, Scanner::MIX_OUTPUT));

    addParam(ParamWidget::create<CKSS>(Vec(75, 312), module, Scanner::OFFSET_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Trimpot>(Vec(180, 313), module, Scanner::MIXSCALE_PARAM, 0.0, 1.0, 0.125));

    const int inXPos = 10;
    const int inLightX = 50;

    const int outXPos = 235;
    const int outLightX = 210;

    for(int i = 0; i < 8; i++)
    {
        const int yPos = 50 + (40 * i);
        const int lightY = 59 + (40 * i);

        //////INPUTS//////
        addInput(Port::create<PJ301MPort>(Vec(inXPos, yPos), Port::INPUT, module, i));

        //////OUTPUTS//////
        addOutput(Port::create<PJ301MPort>(Vec(outXPos, yPos), Port::OUTPUT, module, i));

        //////BLINKENLIGHTS//////
        addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(inLightX, lightY), module, Scanner::IN1_LIGHT + i));
        addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(outLightX, lightY), module, Scanner::OUT1_POS_LIGHT + 2*i));
    }
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, Scanner) {
   Model *modelScanner = Model::create<Scanner, ScannerWidget>("HetrickCV", "Scanner", "Scanner", MIXER_TAG);
   return modelScanner;
}
