#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

struct Delta : Module
{
	enum ParamIds
	{
		AMOUNT_PARAM,
        SCALE_PARAM,
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
		GT_GATE_OUTPUT,
		GT_TRIG_OUTPUT,
		LT_GATE_OUTPUT,
		LT_TRIG_OUTPUT,
		CHANGE_OUTPUT,
        DELTA_OUTPUT,
		NUM_OUTPUTS
	};

	 enum LightIds
    {
        GT_LIGHT,
        LT_LIGHT,
		CHANGE_LIGHT,
        NUM_LIGHTS
	};

	Delta() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

	}

	TriggerGenWithSchmitt ltTrig, gtTrig;
    float lastInput = 0.0f;
    bool rising = false;
    bool falling = false;

	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Delta::step()
{
	float input = inputs[MAIN_INPUT].value;

    float delta = input - lastInput;
    lastInput = input;

    rising = (delta > 0.0f);
    falling = (delta < 0.0f);

	float boost = params[AMOUNT_PARAM].value + (inputs[AMOUNT_INPUT].value * params[SCALE_PARAM].value);
	boost = clampf(boost, 0.0f, 5.0f) * 8000.0f + 1;

	outputs[GT_TRIG_OUTPUT].value = gtTrig.process(rising) ? 5.0f : 0.0f;
	outputs[LT_TRIG_OUTPUT].value = ltTrig.process(falling) ? 5.0f : 0.0f;
	outputs[GT_GATE_OUTPUT].value = rising ? 5.0f : 0.0f;
	outputs[LT_GATE_OUTPUT].value = falling ? 5.0f : 0.0f;

	float allTrigs = outputs[GT_TRIG_OUTPUT].value + outputs[LT_TRIG_OUTPUT].value;
	allTrigs = clampf(allTrigs, 0.0f, 5.0f);

    const float deltaOutput = clampf(delta * boost, -5.0f, 5.0f);

	outputs[CHANGE_OUTPUT].value = allTrigs;
    outputs[DELTA_OUTPUT].value = deltaOutput;

	lights[GT_LIGHT].setBrightnessSmooth(outputs[GT_GATE_OUTPUT].value);
	lights[LT_LIGHT].setBrightnessSmooth(outputs[LT_GATE_OUTPUT].value);
	lights[CHANGE_LIGHT].setBrightnessSmooth(allTrigs);
}


struct DeltaWidget : ModuleWidget { DeltaWidget(Delta *module); };

DeltaWidget::DeltaWidget(Delta *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Delta.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	//////PARAMS//////
	addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(27, 62), module, Delta::AMOUNT_PARAM, 0.0, 5.0, 0.0));
    addParam(ParamWidget::create<Trimpot>(Vec(36, 112), module, Delta::SCALE_PARAM, -1.0, 1.0, 1.0));

	//////INPUTS//////
    addInput(Port::create<PJ301MPort>(Vec(12, 195), Port::INPUT, module, Delta::MAIN_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(33, 145), Port::INPUT, module, Delta::AMOUNT_INPUT));

	//////OUTPUTS//////
    addOutput(Port::create<PJ301MPort>(Vec(53, 195), Port::OUTPUT, module, Delta::DELTA_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(12, 285), Port::OUTPUT, module, Delta::LT_GATE_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(53, 285), Port::OUTPUT, module, Delta::GT_GATE_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(12, 315), Port::OUTPUT, module, Delta::LT_TRIG_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(53, 315), Port::OUTPUT, module, Delta::GT_TRIG_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(32.5, 245), Port::OUTPUT, module, Delta::CHANGE_OUTPUT));

	//////BLINKENLIGHTS//////
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(22, 275), module, Delta::LT_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(62, 275), module, Delta::GT_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(42, 275), module, Delta::CHANGE_LIGHT));
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, Delta) {
   Model *modelDelta = Model::create<Delta, DeltaWidget>("HetrickCV", "Delta", "Delta", LOGIC_TAG);
   return modelDelta;
}
