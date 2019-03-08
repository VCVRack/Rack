#include "Bark.hpp"
#include "dsp/digital.hpp"
#include "barkComponents.hpp"

namespace rack_plugin_Bark {

struct QuadLogic : Module
{
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		LOGIC_A1_INPUT,
		LOGIC_B1_INPUT,
		LOGIC_A2_INPUT,
		LOGIC_B2_INPUT,
		LOGIC_A3_INPUT,
		LOGIC_B3_INPUT,
		LOGIC_A4_INPUT,
		LOGIC_B4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MAX1_OUTPUT,
		MIN1_OUTPUT,
		MAX2_OUTPUT,
		MIN2_OUTPUT,
		MAX3_OUTPUT,
		MIN3_OUTPUT,
		MAX4_OUTPUT,
		MIN4_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LOGIC_POS1_LIGHT, LOGIC_NEG1_LIGHT,
		LOGIC_POS2_LIGHT, LOGIC_NEG2_LIGHT,
		LOGIC_POS3_LIGHT, LOGIC_NEG3_LIGHT,
		LOGIC_POS4_LIGHT, LOGIC_NEG4_LIGHT,
		NUM_LIGHTS
	};
	QuadLogic() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	
	}
	void step() override;
};

void QuadLogic::step()
{
	float logicSum1 = inputs[LOGIC_A1_INPUT].value + inputs[LOGIC_B1_INPUT].value;
	float logicSum2 = inputs[LOGIC_A2_INPUT].value + inputs[LOGIC_B2_INPUT].value;
	float logicSum3 = inputs[LOGIC_A3_INPUT].value + inputs[LOGIC_B3_INPUT].value;
	float logicSum4 = inputs[LOGIC_A4_INPUT].value + inputs[LOGIC_B4_INPUT].value;
	lights[LOGIC_POS1_LIGHT].setBrightnessSmooth(fmaxf(0.0f, logicSum1 / 5.0f));
	lights[LOGIC_NEG1_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -logicSum1 / 5.0f));
	lights[LOGIC_POS2_LIGHT].setBrightnessSmooth(fmaxf(0.0f, logicSum2 / 5.0f));
	lights[LOGIC_NEG2_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -logicSum2 / 5.0f));
	lights[LOGIC_POS3_LIGHT].setBrightnessSmooth(fmaxf(0.0f, logicSum3 / 5.0f));
	lights[LOGIC_NEG3_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -logicSum3 / 5.0f));
	lights[LOGIC_POS4_LIGHT].setBrightnessSmooth(fmaxf(0.0f, logicSum4 / 5.0f));
	lights[LOGIC_NEG4_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -logicSum4 / 5.0f));

	outputs[MAX1_OUTPUT].value = fmaxf(inputs[LOGIC_A1_INPUT].value, inputs[LOGIC_B1_INPUT].value);
	outputs[MIN1_OUTPUT].value = fminf(inputs[LOGIC_A1_INPUT].value, inputs[LOGIC_B1_INPUT].value);
	outputs[MAX2_OUTPUT].value = fmaxf(inputs[LOGIC_A2_INPUT].value, inputs[LOGIC_B2_INPUT].value);
	outputs[MIN2_OUTPUT].value = fminf(inputs[LOGIC_A2_INPUT].value, inputs[LOGIC_B2_INPUT].value);
	outputs[MAX3_OUTPUT].value = fmaxf(inputs[LOGIC_A3_INPUT].value, inputs[LOGIC_B3_INPUT].value);
	outputs[MIN3_OUTPUT].value = fminf(inputs[LOGIC_A3_INPUT].value, inputs[LOGIC_B3_INPUT].value);
	outputs[MAX4_OUTPUT].value = fmaxf(inputs[LOGIC_A4_INPUT].value, inputs[LOGIC_B4_INPUT].value);
	outputs[MIN4_OUTPUT].value = fminf(inputs[LOGIC_A4_INPUT].value, inputs[LOGIC_B4_INPUT].value);
}


struct QuadLogicWidget : ModuleWidget {
	QuadLogicWidget(QuadLogic *module);
};

QuadLogicWidget::QuadLogicWidget(QuadLogic *module) : ModuleWidget(module) {
	box.size = Vec(5 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/BarkQuadLogic.svg")));
		addChild(panel);
	}

//PortIn---
	addInput(Port::create<BarkPatchPortIn>(Vec(8.45f, 380 - 348.52f + 0.35f), Port::INPUT, module, QuadLogic::LOGIC_A1_INPUT));
	addInput(Port::create<BarkPatchPortIn>(Vec(42.71f, 380 - 348.52f + 0.35f), Port::INPUT, module, QuadLogic::LOGIC_B1_INPUT));	
	addInput(Port::create<BarkPatchPortIn>(Vec(8.1f + 0.35f, 380 - 266.09f + 0.35f), Port::INPUT, module, QuadLogic::LOGIC_B2_INPUT));
	addInput(Port::create<BarkPatchPortIn>(Vec(42.71f, 380 - 266.09f + 0.35f), Port::INPUT, module, QuadLogic::LOGIC_A2_INPUT));	
	addInput(Port::create<BarkPatchPortIn>(Vec(8.1f + 0.35f, 380 - 133.18f), Port::INPUT, module, QuadLogic::LOGIC_A3_INPUT));
	addInput(Port::create<BarkPatchPortIn>(Vec(42.71f, 380 - 133.18f), Port::INPUT, module, QuadLogic::LOGIC_B3_INPUT));
	addInput(Port::create<BarkPatchPortIn>(Vec(8.1f + 0.35f, 380 - 49.53f), Port::INPUT, module, QuadLogic::LOGIC_B4_INPUT));
	addInput(Port::create<BarkPatchPortIn>(Vec(42.71f, 380 - 49.53f), Port::INPUT, module, QuadLogic::LOGIC_A4_INPUT));
//PortOut---
	addOutput(Port::create<BarkPatchPortOut>(Vec(8.1f + 0.35f, 380 - 320.3f), Port::OUTPUT, module, QuadLogic::MIN1_OUTPUT));
	addOutput(Port::create<BarkPatchPortOut>(Vec(42.71f, 380 - 320.3f), Port::OUTPUT, module, QuadLogic::MAX1_OUTPUT));
	addOutput(Port::create<BarkPatchPortOut>(Vec(8.1f + 0.35f, 380 - 230.2f), Port::OUTPUT, module, QuadLogic::MAX2_OUTPUT));
	addOutput(Port::create<BarkPatchPortOut>(Vec(42.71, 380 - 230.2f), Port::OUTPUT, module, QuadLogic::MIN2_OUTPUT));
	addOutput(Port::create<BarkPatchPortOut>(Vec(8.1f + 0.35f, 380 - 169.08f + 0.35f), Port::OUTPUT, module, QuadLogic::MIN3_OUTPUT));
	addOutput(Port::create<BarkPatchPortOut>(Vec(42.71, 380 - 169.08f + 0.35f), Port::OUTPUT, module, QuadLogic::MAX3_OUTPUT));
	addOutput(Port::create<BarkPatchPortOut>(Vec(8.1f + 0.35f, 380 - 77.73f + 0.35f), Port::OUTPUT, module, QuadLogic::MAX4_OUTPUT));
	addOutput(Port::create<BarkPatchPortOut>(Vec(42.71f, 380 - 77.73f + 0.35f), Port::OUTPUT, module, QuadLogic::MIN4_OUTPUT));
//screw---
	addChild(Widget::create<BarkScrew1>(Vec(2, 367.2f)));						//pos3
	addChild(Widget::create<BarkScrew3>(Vec(box.size.x - 13, 3)));				//pos2
//Lights---
	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(34.82f - 0.35f, 380 - 326.8f), module, QuadLogic::LOGIC_POS1_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(34.82f - 0.35f, 380 - 240.7f), module, QuadLogic::LOGIC_POS2_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(34.82f, 380 - 144.68f), module, QuadLogic::LOGIC_POS3_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(34.82f, 380 - 56.04f), module, QuadLogic::LOGIC_POS4_LIGHT));
}

} // namespace rack_plugin_Bark

using namespace rack_plugin_Bark;

RACK_PLUGIN_MODEL_INIT(Bark, QuadLogic) {
   Model *modelQuadLogic = Model::create<QuadLogic, QuadLogicWidget>("Bark", "QuadLogic", "Quad Logic", UTILITY_TAG, LOGIC_TAG);
   return modelQuadLogic;
}
