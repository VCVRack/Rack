#include "AS.hpp"

struct BlankPanel6 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	BlankPanel6() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void BlankPanel6::step() 
{
}

struct BlankPanel6Widget : ModuleWidget 
{ 
    BlankPanel6Widget(BlankPanel6 *module);
};

BlankPanel6Widget::BlankPanel6Widget(BlankPanel6 *module) : ModuleWidget(module) {

	setPanel(SVG::load(assetPlugin(plugin, "res/Blanks/BlankPanel6.svg")));
	
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(AS, BlankPanel6) {
   Model *modelBlankPanel6 = Model::create<BlankPanel6, BlankPanel6Widget>("AS", "BlankPanel6", "BlankPanel 6", BLANK_TAG);
   return modelBlankPanel6;
}
