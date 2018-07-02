#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

#ifdef USE_VST2
#define plugin "HetrickCV"
#endif // USE_VST2

struct LogicCombine : Module
{
	enum ParamIds
	{
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
		NUM_INPUTS
	};
	enum OutputIds
	{
        OR_OUTPUT,
        NOR_OUTPUT,
        TRIG_OUTPUT,
		NUM_OUTPUTS
    };

    enum LightIds
    {
        OR_LIGHT,
        NOR_LIGHT,
		TRIG_LIGHT,
        NUM_LIGHTS
	};

    bool ins[NUM_INPUTS] = {};
    bool trigs[NUM_INPUTS] = {};
    float outs[3] = {};
    float trigLight;
    SchmittTrigger inTrigs[NUM_INPUTS];
    bool orState = false;
    bool trigState = false;
    const float lightLambda = 0.075;

    TriggerGenerator trigger;

	LogicCombine() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

	}

	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void LogicCombine::step()
{
    orState = false;
    trigState = false;

    for(int i = 0; i < NUM_INPUTS; i++)
    {
        ins[i] = (inputs[IN1_INPUT + i].value >= 1.0f);
        trigs[i] = inTrigs[i].process(inputs[IN1_INPUT + i].value);

        orState = orState || ins[i];
        trigState = trigState || trigs[i];
    }

    outs[0] = orState ? 5.0f : 0.0f;
    outs[1] = 5.0f - outs[0];

    if(trigState)
    {
        trigger.trigger();
        lights[TRIG_LIGHT].value = 5.0f;
    }

    outs[2] = trigger.process() ? 5.0f : 0.0f;

    if (lights[TRIG_LIGHT].value > 0.01)
        lights[TRIG_LIGHT].value -= lights[TRIG_LIGHT].value / lightLambda * engineGetSampleTime();

    outputs[OR_OUTPUT].value = outs[0];
    outputs[NOR_OUTPUT].value = outs[1];
    outputs[TRIG_OUTPUT].value = outs[2];

    lights[OR_LIGHT].setBrightness(outs[0]);
    lights[NOR_LIGHT].setBrightness(outs[1]);
    lights[TRIG_LIGHT].setBrightnessSmooth(outs[2]);
}

struct LogicCombineWidget : ModuleWidget { LogicCombineWidget(LogicCombine *module); };

LogicCombineWidget::LogicCombineWidget(LogicCombine *module) : ModuleWidget(module)
{
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/LogicCombiner.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////PARAMS//////

    //////INPUTS//////
    const int inSpacing = 40;
    const int outPos = 67;
    const int lightPos = outPos + 29;

    for(int i = 0; i < LogicCombine::NUM_INPUTS; i++)
    {
        addInput(Port::create<PJ301MPort>(Vec(10, 50 + (i*inSpacing)), Port::INPUT, module, LogicCombine::IN1_INPUT + i));
    }

    //////OUTPUTS//////
    addOutput(Port::create<PJ301MPort>(Vec(outPos, 150), Port::OUTPUT, module, LogicCombine::OR_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(outPos, 195), Port::OUTPUT, module, LogicCombine::NOR_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(outPos, 240), Port::OUTPUT, module, LogicCombine::TRIG_OUTPUT));

    //////BLINKENLIGHTS//////
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(lightPos, 158), module, LogicCombine::OR_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(lightPos, 203), module, LogicCombine::NOR_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(lightPos, 248), module, LogicCombine::TRIG_LIGHT));
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, LogicCombine) {
   Model *modelLogicCombine = Model::create<LogicCombine, LogicCombineWidget>("HetrickCV", "Logic Combine", "OR Logic (Gate Combiner)", LOGIC_TAG);
   return modelLogicCombine;
}
