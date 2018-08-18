#include "common.hpp"
#include "switch.hpp"

namespace rack_plugin_TheXOR {

void Switch::step()
{
	for(int k = 0; k < NUM_SWITCHES; k++)
	{
		if(outputs[OUT_1 + k].active && inputs[IN_1 + k].active)
		{
			if(getSwitch(k))
			{
				lights[LED_1 + k].value = 5;;
				outputs[OUT_1 + k].value = inputs[IN_1 + k].value;
			} else
			{
				lights[LED_1 + k].value = outputs[OUT_1 + k].value = 0;
			}
		} else
		{
			lights[LED_1 + k].value = outputs[OUT_1 + k].value = 0;
		}
	}
}

SwitchWidget::SwitchWidget(Switch *module) : ModuleWidget(module)
{
	box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/modules/Switch.svg")));		
		addChild(panel);
	}

	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	float in_x = mm2px(2.500);
	float mod_x = mm2px(17.306);
	float sw_x = mm2px(25.027);
	float led_x = mm2px(28.173);
	float out_x = mm2px(40.045);
	float y = 101.567;
	float y1 = 98.387;
	float yled = 114.949;
	float ysw = 105.667;
	float delta_y = 79.394 - 101.567;
	
	for(int k = 0; k < NUM_SWITCHES; k++)
	{
		addInput(Port::create<PJ301GRPort>(Vec(in_x, yncscape(y, 8.255)), Port::INPUT, module, Switch::IN_1 + k));
		addInput(Port::create<PJ301BPort>(Vec(mod_x, yncscape(y1, 8.255)), Port::INPUT, module, Switch::MOD_1 + k));
		addParam(ParamWidget::create<NKK2>(Vec(sw_x, yncscape(ysw, 7.336)), module, Switch::SW_1+k, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(led_x, yncscape(yled, 2.176)), module, Switch::LED_1 + k ));
		addOutput(Port::create<PJ301GPort>(Vec(out_x, yncscape(y, 8.255)), Port::OUTPUT, module, Switch::OUT_1+k));
		y += delta_y;
		y1 += delta_y;
		ysw += delta_y;
		yled += delta_y;
	}
}

} // namespace rack_plugin_TheXOR

using namespace rack_plugin_TheXOR;

RACK_PLUGIN_MODEL_INIT(TheXOR, Switch) {
	return Model::create<Switch, SwitchWidget>("TheXOR", "Switch", "Switch", SWITCH_TAG);
}

