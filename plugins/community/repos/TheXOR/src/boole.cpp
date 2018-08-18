#include "common.hpp"
#include "boole.hpp"

namespace rack_plugin_TheXOR {

void Boole::step()
{
	for(int k = 0; k < NUM_BOOL_OP; k++)
	{
		int index = 2 * k;
		if(inputs[IN_1 + index].active && (k == 0 || inputs[IN_1 + index - 1].active))
		{
			bool o = process(k, index);
			if(k > 0 && params[INVERT_1 + k - 1].value > 0)
				o = !o;
			lights[LED_1+k+ 2 * NUM_BOOL_OP-1].value = o ? 5.0 : 0.0;
			outputs[OUT_1 + k].value = o ? LVL_ON : LVL_OFF;
		} else
		{
			outputs[OUT_1 + k].value = lights[LED_1 + k + 2 * NUM_BOOL_OP - 1].value = LVL_OFF;
		}
	}
}

bool Boole::process(int num_op, int index)
{
	bool x;
	if(num_op == 0)	// not?
	{
		x = inputs[IN_1].normalize(0.0) > params[THRESH_1 ].value;
		lights[LED_1].value = x ? 5.0 : 0.0;
		return !x;
	} else
	{
		x = inputs[IN_1 + index-1].normalize(0.0) > params[THRESH_1 + index-1].value;
		lights[LED_1 + index - 1].value = x ? 5.0 : 0.0;
	}
	bool y = inputs[IN_1 + index].normalize(0.0) > params[THRESH_1 + index].value;
	lights[LED_1 + index].value = y ? 5.0 : 0.0;
		
	switch(num_op)
	{	
		case 1: return x && y;	//and
		case 2: return x || y;	//or
		case 3: return x ^ y;	//the xor
		case 4: return !x || y;	// implication
	}

	return false;
}

BooleWidget::BooleWidget(Boole *module) : ModuleWidget(module)
{
	box.size = Vec(14* RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/modules/boole.svg")));		
		addChild(panel);
	}

	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	float in_x = mm2px(5.170);
	float in_led_x = mm2px(15.778);
	float out_led_x = mm2px(53.878);
	float pot_x = mm2px(20.561);
	float out_x = mm2px(58.091);
	float y = 112.349;
	float yout = 112.349;
	float ypot = 112.477;
	float yled = 115.389;
	float yinv = 97.892;
	float yled_out = 115.389;
	float delta_y =- 14.771;
	float sub_dy = -11.92;
	float out_dy = -26.691;
	
	for(int k = 0; k < NUM_BOOL_OP; k++)
	{
		int index = 2 * k;
		if(k > 0)
			index--;

		// X
		addInput(Port::create<PJ301GRPort>(Vec(in_x, yncscape(y, 8.255)), Port::INPUT, module, Boole::IN_1 + index));
		addParam(ParamWidget::create<Davies1900hFixWhiteKnobSmall>(Vec(pot_x, yncscape(ypot, 8.0)), module, Boole::THRESH_1 + index, 0.0, 10.0, 0.0));
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(in_led_x, yncscape(yled, 2.176)), module, Boole::LED_1+index));

		// Y
		if(k > 0)
		{
			index++;
			y += sub_dy;
			ypot += sub_dy;
			yled += sub_dy;
			addInput(Port::create<PJ301GRPort>(Vec(in_x, yncscape(y, 8.255)), Port::INPUT, module, Boole::IN_1 + index));
			addParam(ParamWidget::create<Davies1900hFixWhiteKnobSmall>(Vec(pot_x, yncscape(ypot, 8.0) ), module, Boole::THRESH_1 + index, 0.0, 10.0, 0.0));
			addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(in_led_x, yncscape(yled, 2.176)), module, Boole::LED_1 + index ));
		}
		
		// OUT
		addOutput(Port::create<PJ301WPort>(Vec(out_x, yncscape(yout, 8.255)), Port::OUTPUT, module, Boole::OUT_1+k));
		addChild(ModuleLightWidget::create<SmallLight<WhiteLight>>(Vec(out_led_x, yncscape(yled_out, 2.176)), module, Boole::LED_1 + k+ 2 * NUM_BOOL_OP-1));
		if(k == 0)
		{
			yled_out -= 20.731;
			yout -= 20.731;
		} else
		{
			addParam(ParamWidget::create<CKSSFix>(Vec(mm2px(53.116), yncscape(yinv, 5.460)), module, Boole::INVERT_1 + k - 1, 0.0, 1.0, 0.0));
			yled_out += out_dy;
			yout += out_dy;
			yinv += out_dy;
		}

		y += delta_y;
		ypot += delta_y;
		yled += delta_y;
	}
}

} // namespace rack_plugin_TheXOR

using namespace rack_plugin_TheXOR;

RACK_PLUGIN_MODEL_INIT(TheXOR, Boole) {
	return Model::create<Boole, BooleWidget>("TheXOR", "Boole", "Boole", LOGIC_TAG);
}

