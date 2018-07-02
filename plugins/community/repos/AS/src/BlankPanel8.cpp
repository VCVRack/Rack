#include "AS.hpp"

struct BlankPanel8 : Module {
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

	BlankPanel8() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void BlankPanel8::step() 
{
}

struct BlankPanel8Widget : ModuleWidget 
{ 
    BlankPanel8Widget(BlankPanel8 *module);
};

BlankPanel8Widget::BlankPanel8Widget(BlankPanel8 *module) : ModuleWidget(module) {

	setPanel(SVG::load(assetPlugin(plugin, "res/Blanks/BlankPanel8.svg")));

	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(AS, BlankPanel8) {
   Model *modelBlankPanel8 = Model::create<BlankPanel8, BlankPanel8Widget>("AS", "BlankPanel8", "BlankPanel 8", BLANK_TAG);
   return modelBlankPanel8;
}
