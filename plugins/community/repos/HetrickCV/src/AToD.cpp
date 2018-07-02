#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

struct AnalogToDigital : Module
{
	enum ParamIds
	{
        SCALE_PARAM,
        OFFSET_PARAM,
        MODE_PARAM,
        RECTIFY_PARAM,

		NUM_PARAMS
	};
	enum InputIds
	{
        MAIN_INPUT,
        SYNC_INPUT,

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
        OUT1_LIGHT,
        OUT2_LIGHT,
        OUT3_LIGHT,
        OUT4_LIGHT,
        OUT5_LIGHT,
        OUT6_LIGHT,
        OUT7_LIGHT,
        OUT8_LIGHT,

        RECT_NONE_LIGHT,
        RECT_HALF_LIGHT,
        RECT_FULL_LIGHT,

        MODE_UNI8_LIGHT,
        MODE_BOFF_LIGHT,
        MODE_BSIG_LIGHT,

		NUM_LIGHTS
    };

    SchmittTrigger clockTrigger;
    SchmittTrigger modeTrigger;
    SchmittTrigger rectTrigger;

    int mode = 0;
    int rectMode = 0;

    float outs[8] = {};

	AnalogToDigital() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

	}

    void step() override;

    void processUni8(float _input);
    void processBiOff(float _input);
    void processBiSig(float _input);

    void reset() override
    {
        mode = 0;
        rectMode = 0;
	}
    void randomize() override
    {
        mode = round(randomf() * 2.0f);
        rectMode = round(randomf() * 2.0f);
    }

    json_t *toJson() override
    {
		json_t *rootJ = json_object();
        json_object_set_new(rootJ, "mode", json_integer(mode));
        json_object_set_new(rootJ, "rectMode", json_integer(rectMode));
		return rootJ;
	}
    void fromJson(json_t *rootJ) override
    {
		json_t *modeJ = json_object_get(rootJ, "mode");
		if (modeJ)
            mode = json_integer_value(modeJ);

        json_t *rectModeJ = json_object_get(rootJ, "rectMode");
        if (rectModeJ)
            rectMode = json_integer_value(rectModeJ);
	}

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void AnalogToDigital::step()
{
    if (modeTrigger.process(params[MODE_PARAM].value)) mode = (mode + 1) % 3;
    if (rectTrigger.process(params[RECTIFY_PARAM].value)) rectMode = (rectMode + 1) % 3;

    lights[MODE_UNI8_LIGHT].setBrightness(mode == 0 ? 1.0f : 0.0f);
    lights[MODE_BOFF_LIGHT].setBrightness(mode == 1 ? 1.0f : 0.0f);
    lights[MODE_BSIG_LIGHT].setBrightness(mode == 2 ? 1.0f : 0.0f);

    lights[RECT_NONE_LIGHT].setBrightness(rectMode == 0 ? 1.0f : 0.0f);
    lights[RECT_HALF_LIGHT].setBrightness(rectMode == 1 ? 1.0f : 0.0f);
    lights[RECT_FULL_LIGHT].setBrightness(rectMode == 2 ? 1.0f : 0.0f);

    const bool syncModeEnabled = inputs[SYNC_INPUT].active;
    const bool readyForProcess = (!syncModeEnabled || (syncModeEnabled && clockTrigger.process(inputs[SYNC_INPUT].value)));

    if(readyForProcess)
    {
        float input = inputs[MAIN_INPUT].value;
        input *= params[SCALE_PARAM].value;
        input += params[OFFSET_PARAM].value;
        if (rectMode == 1) input = input > 0.0f? input : 0.0f;
        else if (rectMode == 2) input = std::abs(input);

        if(mode == 0) processUni8(input);
        else if (mode == 1) processBiOff(input);
        else if (mode == 2) processBiSig(input);
    }

    for(int i = 0; i < 8; i++)
    {
        outputs[OUT1_OUTPUT + i].value = outs[i];
        lights[OUT1_LIGHT + i].value = outs[i];
    }
}

void AnalogToDigital::processUni8(float _input)
{
    clampf(_input, 0.0f, 1.0f);
    uint8_t bits = round(_input * 255);

    outs[0] = (bits & 0b00000001) > 0.0f ? 5.0f : 0.0f;
    outs[1] = (bits & 0b00000010) > 0.0f ? 5.0f : 0.0f;
    outs[2] = (bits & 0b00000100) > 0.0f ? 5.0f : 0.0f;
    outs[3] = (bits & 0b00001000) > 0.0f ? 5.0f : 0.0f;
    outs[4] = (bits & 0b00010000) > 0.0f ? 5.0f : 0.0f;
    outs[5] = (bits & 0b00100000) > 0.0f ? 5.0f : 0.0f;
    outs[6] = (bits & 0b01000000) > 0.0f ? 5.0f : 0.0f;
    outs[7] = (bits & 0b10000000) > 0.0f ? 5.0f : 0.0f;
}

void AnalogToDigital::processBiOff(float _input)
{
    clampf(_input, -1.0f, 1.0f);
    _input = (_input + 1.0f) * 0.5f;
    uint8_t bits = round(_input * 255);

    outs[0] = (bits & 0b00000001) > 0.0f ? 5.0f : 0.0f;
    outs[1] = (bits & 0b00000010) > 0.0f ? 5.0f : 0.0f;
    outs[2] = (bits & 0b00000100) > 0.0f ? 5.0f : 0.0f;
    outs[3] = (bits & 0b00001000) > 0.0f ? 5.0f : 0.0f;
    outs[4] = (bits & 0b00010000) > 0.0f ? 5.0f : 0.0f;
    outs[5] = (bits & 0b00100000) > 0.0f ? 5.0f : 0.0f;
    outs[6] = (bits & 0b01000000) > 0.0f ? 5.0f : 0.0f;
    outs[7] = (bits & 0b10000000) > 0.0f ? 5.0f : 0.0f;
}

void AnalogToDigital::processBiSig(float _input)
{
    outs[7] = _input < 0.0f ? 5.0f : 0.0f;

    clampf(_input, -1.0f, 1.0f);
    _input = std::abs(_input);
    uint8_t bits = round(_input * 127);

    outs[0] = (bits & 0b00000001) > 0.0f ? 5.0f : 0.0f;
    outs[1] = (bits & 0b00000010) > 0.0f ? 5.0f : 0.0f;
    outs[2] = (bits & 0b00000100) > 0.0f ? 5.0f : 0.0f;
    outs[3] = (bits & 0b00001000) > 0.0f ? 5.0f : 0.0f;
    outs[4] = (bits & 0b00010000) > 0.0f ? 5.0f : 0.0f;
    outs[5] = (bits & 0b00100000) > 0.0f ? 5.0f : 0.0f;
    outs[6] = (bits & 0b01000000) > 0.0f ? 5.0f : 0.0f;
}


struct AnalogToDigitalWidget : ModuleWidget { AnalogToDigitalWidget(AnalogToDigital *module); };

AnalogToDigitalWidget::AnalogToDigitalWidget(AnalogToDigital *module) : ModuleWidget(module)
{
	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/AToD.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////PARAMS//////
    addParam(ParamWidget::create<CKD6>(Vec(16, 270), module, AnalogToDigital::MODE_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<CKD6>(Vec(65, 270), module, AnalogToDigital::RECTIFY_PARAM, 0.0, 1.0, 0.0));

    //////BLINKENLIGHTS//////
    int modeLightX = 12;
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(modeLightX, 306), module, AnalogToDigital::MODE_UNI8_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(modeLightX, 319), module, AnalogToDigital::MODE_BOFF_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(modeLightX, 332), module, AnalogToDigital::MODE_BSIG_LIGHT));

    int rectLightX = 64;
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(rectLightX, 306), module, AnalogToDigital::RECT_NONE_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(rectLightX, 319), module, AnalogToDigital::RECT_HALF_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(rectLightX, 332), module, AnalogToDigital::RECT_FULL_LIGHT));

    //////INPUTS//////
    addInput(Port::create<PJ301MPort>(Vec(7, 70), Port::INPUT, module, AnalogToDigital::MAIN_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(42, 152), Port::INPUT, module, AnalogToDigital::SYNC_INPUT));

    addParam(ParamWidget::create<Trimpot>(Vec(44, 73), module, AnalogToDigital::SCALE_PARAM, -1.0, 1.0, 0.2));
    addParam(ParamWidget::create<Trimpot>(Vec(80, 73), module, AnalogToDigital::OFFSET_PARAM, -5.0, 5.0, 0.0));

    const int outXPos = 145;
    const int outLightX = 120;
    for(int i = 0; i < 8; i++)
    {
        const int yPos = 50 + (40 * i);
        const int lightY = 59 + (40 * i);

        //////OUTPUTS//////
        addOutput(Port::create<PJ301MPort>(Vec(outXPos, yPos), Port::OUTPUT, module, AnalogToDigital::OUT1_OUTPUT + i));

        //////BLINKENLIGHTS//////
        addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(outLightX, lightY), module, AnalogToDigital::OUT1_LIGHT + i));
    }
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, AnalogToDigital) {
   Model *modelAnalogToDigital = Model::create<AnalogToDigital, AnalogToDigitalWidget>("HetrickCV", "AnalogToDigital", "Analog to Digital", LOGIC_TAG);
   return modelAnalogToDigital;
}
