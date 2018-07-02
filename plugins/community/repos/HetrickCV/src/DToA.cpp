#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

struct DigitalToAnalog : Module
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
        IN1_INPUT,
        IN2_INPUT,
        IN3_INPUT,
        IN4_INPUT,
        IN5_INPUT,
        IN6_INPUT,
        IN7_INPUT,
        IN8_INPUT,

        SYNC_INPUT,

		NUM_INPUTS
	};
	enum OutputIds
	{
        MAIN_OUTPUT,
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

        RECT_NONE_LIGHT,
        RECT_HALF_LIGHT,
        RECT_FULL_LIGHT,

        MODE_UNI8_LIGHT,
        MODE_BOFF_LIGHT,
        MODE_BSIG_LIGHT,

        OUT_POS_LIGHT, OUT_NEG_LIGHT,

		NUM_LIGHTS
    };

    SchmittTrigger clockTrigger;
    SchmittTrigger modeTrigger;
    SchmittTrigger rectTrigger;

    int mode = 0;
    int rectMode = 0;

    float mainOutput = 0.0f;

    bool ins[8] = {};

	DigitalToAnalog() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

	}

    void step() override;

    void processUni8();
    void processBiOff();
    void processBiSig();

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


void DigitalToAnalog::step()
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
        mainOutput = 0.0f;

        for(int i = 0; i < 8; i++)
        {
            ins[i] = inputs[IN1_INPUT + i].value > 1.0f;
            lights[IN1_LIGHT + i].value = ins[i] ? 1.0f : 0.0f;
        }

        if(mode == 0) processUni8();
        else if (mode == 1) processBiOff();
        else if (mode == 2) processBiSig();

        mainOutput *= 5.0f;

        if (rectMode == 1) mainOutput = mainOutput > 0.0f ? mainOutput : 0.0f;
        else if (rectMode == 2) mainOutput = std::abs(mainOutput);

        mainOutput *= params[SCALE_PARAM].value;
        mainOutput += params[OFFSET_PARAM].value;

        lights[OUT_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, mainOutput * 0.2f));
        lights[OUT_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, mainOutput * 0.2f));

        outputs[MAIN_OUTPUT].value = mainOutput;
    }
}

void DigitalToAnalog::processUni8()
{
    if(ins[0]) mainOutput += 1.0f;
    if(ins[1]) mainOutput += 2.0f;
    if(ins[2]) mainOutput += 4.0f;
    if(ins[3]) mainOutput += 8.0f;
    if(ins[4]) mainOutput += 16.0f;
    if(ins[5]) mainOutput += 32.0f;
    if(ins[6]) mainOutput += 64.0f;
    if(ins[7]) mainOutput += 128.0f;

    mainOutput = mainOutput/255.0f;
}

void DigitalToAnalog::processBiOff()
{
    if(ins[0]) mainOutput += 1.0f;
    if(ins[1]) mainOutput += 2.0f;
    if(ins[2]) mainOutput += 4.0f;
    if(ins[3]) mainOutput += 8.0f;
    if(ins[4]) mainOutput += 16.0f;
    if(ins[5]) mainOutput += 32.0f;
    if(ins[6]) mainOutput += 64.0f;
    if(ins[7]) mainOutput += 128.0f;

    mainOutput = mainOutput/255.0f;
    mainOutput *= 2.0f;
    mainOutput = mainOutput - 1.0f;
}

void DigitalToAnalog::processBiSig()
{
    if(ins[0]) mainOutput += 1.0f;
    if(ins[1]) mainOutput += 2.0f;
    if(ins[2]) mainOutput += 4.0f;
    if(ins[3]) mainOutput += 8.0f;
    if(ins[4]) mainOutput += 16.0f;
    if(ins[5]) mainOutput += 32.0f;
    if(ins[6]) mainOutput += 64.0f;

    mainOutput = mainOutput/127.0f;

    if(ins[7]) mainOutput *= -1.0f;
}


struct DigitalToAnalogWidget : ModuleWidget { DigitalToAnalogWidget(DigitalToAnalog *module); };

DigitalToAnalogWidget::DigitalToAnalogWidget(DigitalToAnalog *module) : ModuleWidget(module)
{
	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/DToA.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////PARAMS//////
    addParam(ParamWidget::create<CKD6>(Vec(85, 270), module, DigitalToAnalog::MODE_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<CKD6>(Vec(135, 270), module, DigitalToAnalog::RECTIFY_PARAM, 0.0, 1.0, 0.0));

    //////BLINKENLIGHTS//////
    int modeLightX = 82;
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(modeLightX, 306), module, DigitalToAnalog::MODE_UNI8_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(modeLightX, 319), module, DigitalToAnalog::MODE_BOFF_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(modeLightX, 332), module, DigitalToAnalog::MODE_BSIG_LIGHT));

    int rectLightX = 134;
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(rectLightX, 306), module, DigitalToAnalog::RECT_NONE_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(rectLightX, 319), module, DigitalToAnalog::RECT_HALF_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(rectLightX, 332), module, DigitalToAnalog::RECT_FULL_LIGHT));


    addOutput(Port::create<PJ301MPort>(Vec(78, 70), Port::OUTPUT, module, DigitalToAnalog::MAIN_OUTPUT));
    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(87, 111), module, DigitalToAnalog::OUT_POS_LIGHT));

    //////INPUTS//////
    addInput(Port::create<PJ301MPort>(Vec(112, 152), Port::INPUT, module, DigitalToAnalog::SYNC_INPUT));

    addParam(ParamWidget::create<Trimpot>(Vec(114, 73), module, DigitalToAnalog::SCALE_PARAM, -1.0, 1.0, 0.2));
    addParam(ParamWidget::create<Trimpot>(Vec(150, 73), module, DigitalToAnalog::OFFSET_PARAM, -5.0, 5.0, 0.0));

    const int inXPos = 10;
    const int inLightX = 50;
    for(int i = 0; i < 8; i++)
    {
        const int yPos = 50 + (40 * i);
        const int lightY = 59 + (40 * i);

        //////OUTPUTS//////
        addInput(Port::create<PJ301MPort>(Vec(inXPos, yPos), Port::INPUT, module, DigitalToAnalog::IN1_INPUT + i));

        //////BLINKENLIGHTS//////
        addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(inLightX, lightY), module, DigitalToAnalog::IN1_LIGHT + i));
    }
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, DigitalToAnalog) {
   Model *modelDigitalToAnalog = Model::create<DigitalToAnalog, DigitalToAnalogWidget>("HetrickCV", "DigitalToAnalog", "Digital to Analog", LOGIC_TAG);
   return modelDigitalToAnalog;
}
