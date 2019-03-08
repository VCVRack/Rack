#include "Bark.hpp"
#include "barkComponents.hpp"

namespace rack_plugin_Bark {

struct Panel6 : Module {
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

	Panel6() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void Panel6::step()
{
}

struct Panel6Widget : ModuleWidget
{
	Panel6Widget(Panel6 *module);
};

Panel6Widget::Panel6Widget(Panel6 *module) : ModuleWidget(module) {
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/BarkPanel6.svg")));
		panel->box.size = box.size;
		addChild(panel);
	}
	//screw
	addChild(Widget::create<BarkScrew2>(Vec(2, 3)));								//pos1
	addChild(Widget::create<BarkScrew1>(Vec(box.size.x - 13, 367.2)));			//pos4
}

} // namespace rack_plugin_Bark

using namespace rack_plugin_Bark;

RACK_PLUGIN_MODEL_INIT(Bark, Panel6) {
   //p->addModel(createModel<Panel6Widget>("Bark", "Panel6", "Bark Panel 6", BLANK_TAG));
   Model *modelPanel6 = Model::create<Panel6, Panel6Widget>("Bark", "Panel6", "Bark Panel 6", BLANK_TAG);
   return modelPanel6;
}
