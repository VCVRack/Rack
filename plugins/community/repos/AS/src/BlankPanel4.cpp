#include "AS.hpp"

struct BlankPanel4 : Module {
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

	BlankPanel4() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void BlankPanel4::step() 
{
}

struct BlankPanel4Widget : ModuleWidget 
{ 
    BlankPanel4Widget(BlankPanel4 *module);
};

BlankPanel4Widget::BlankPanel4Widget(BlankPanel4 *module) : ModuleWidget(module) {

	setPanel(SVG::load(assetPlugin(plugin, "res/Blanks/BlankPanel4.svg")));

	//SCREWS - MOD SPACING FOR RACK WIDTH*4
	addChild(Widget::create<as_HexScrew>(Vec(0, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(AS, BlankPanel4) {
   Model *modelBlankPanel4 = Model::create<BlankPanel4, BlankPanel4Widget>("AS", "BlankPanel4", "BlankPanel 4", BLANK_TAG);
   return modelBlankPanel4;
}
