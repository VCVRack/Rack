#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

struct Boolean3 : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
      INA_INPUT,
      INB_INPUT,
      INC_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
      OR_OUTPUT,
      AND_OUTPUT,
      XOR_OUTPUT,
      NOR_OUTPUT,
      NAND_OUTPUT,
      XNOR_OUTPUT,
		NUM_OUTPUTS
    };

    enum LightIds
    {
       OR_LIGHT,
       AND_LIGHT,
       XOR_LIGHT,
       NOR_LIGHT,
       NAND_LIGHT,
       XNOR_LIGHT,
       INA_LIGHT,
       INB_LIGHT,
       INC_LIGHT,
       NUM_LIGHTS
	};

    HysteresisGate ins[3];
    bool inA = false;
    bool inB = false;
    bool inC = false;
    float outs[6] = {};

	Boolean3() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{

	}

	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Boolean3::step()
{
    inA = ins[0].process(inputs[INA_INPUT].value);
    inB = ins[1].process(inputs[INB_INPUT].value);
    inC = ins[2].process(inputs[INC_INPUT].value);

    lights[INA_LIGHT].value = inA ? 5.0f : 0.0f;
    lights[INB_LIGHT].value = inB ? 5.0f : 0.0f;
    lights[INC_LIGHT].value = inC ? 5.0f : 0.0f;

    if(inputs[INC_INPUT].active)
    {
        outs[0] = ((inA || inB) || inC) ? 5.0f : 0.0f;
        outs[1] = ((inA && inB) && inC) ? 5.0f : 0.0f;
        outs[2] = (!inA && (inB ^ inC)) || (inA && !(inB || inC)) ? 5.0f : 0.0f;
        outs[3] = 5.0f - outs[0];
        outs[4] = 5.0f - outs[1];
        outs[5] = 5.0f - outs[2];
    }
    else
    {
        outs[0] = (inA || inB) ? 5.0f : 0.0f;
        outs[1] = (inA && inB) ? 5.0f : 0.0f;
        outs[2] = (inA != inB) ? 5.0f : 0.0f;
        outs[3] = 5.0f - outs[0];
        outs[4] = 5.0f - outs[1];
        outs[5] = 5.0f - outs[2];
    }


    outputs[OR_OUTPUT].value = outs[0];
    outputs[AND_OUTPUT].value = outs[1];
    outputs[XOR_OUTPUT].value = outs[2];
    outputs[NOR_OUTPUT].value = outs[3];
    outputs[NAND_OUTPUT].value = outs[4];
    outputs[XNOR_OUTPUT].value = outs[5];

    lights[OR_LIGHT].value = outs[0];
    lights[AND_LIGHT].value = outs[1];
    lights[XOR_LIGHT].value = outs[2];
    lights[NOR_LIGHT].value = outs[3];
    lights[NAND_LIGHT].value = outs[4];
    lights[XNOR_LIGHT].value = outs[5];
}

struct Boolean3Widget : ModuleWidget { Boolean3Widget(Boolean3 *module); };

Boolean3Widget::Boolean3Widget(Boolean3 *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Boolean3.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

   //////INPUTS//////
   addInput(Port::create<PJ301MPort>(Vec(10, 105), Port::INPUT, module, Boolean3::INA_INPUT));
   addInput(Port::create<PJ301MPort>(Vec(10, 195), Port::INPUT, module, Boolean3::INB_INPUT));
   addInput(Port::create<PJ301MPort>(Vec(10, 285), Port::INPUT, module, Boolean3::INC_INPUT));
   addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(18, 92), module, Boolean3::INA_LIGHT));
   addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(18, 182), module, Boolean3::INB_LIGHT));
   addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(18, 272), module, Boolean3::INC_LIGHT));

   //////OUTPUTS//////
   for(int i = 0; i < 6; i++)
   {
      const int yPos = i*45;
      addOutput(Port::create<PJ301MPort>(Vec(45, 60 + yPos), Port::OUTPUT, module, Boolean3::OR_OUTPUT + i));
      addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(74, 68 + yPos), module, Boolean3::OR_LIGHT + i));
   }

}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, Boolean3) {
   Model *modelBoolean3 = Model::create<Boolean3, Boolean3Widget>("HetrickCV", "Boolean3", "Boolean Logic", LOGIC_TAG);
   return modelBoolean3;
}
