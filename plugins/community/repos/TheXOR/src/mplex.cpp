#include "common.hpp"
#include "mplex.hpp"

namespace rack_plugin_TheXOR {

void Mplex::on_loaded()
{
	load();
}

void Mplex::load()
{
	set_output(0);
}

void Mplex::set_output(int n)
{
	cur_sel = n;
	for(int k = 0; k < NUM_MPLEX_INPUTS; k++)
	{
		lights[LED_1 + k].value = k == cur_sel ? LVL_ON : LVL_OFF;
	}
}

void Mplex::step()
{
	if(upTrigger.process(params[BTDN].value + inputs[INDN].value))
	{
		if(++cur_sel >= NUM_MPLEX_INPUTS)
			cur_sel = 0;
		set_output(cur_sel);
	} else if(dnTrigger.process(params[BTUP].value + inputs[INUP].value))
	{
		if(--cur_sel < 0)
			cur_sel = NUM_MPLEX_INPUTS-1;
		set_output(cur_sel);
	}

	outputs[OUT_1].value = inputs[IN_1 + cur_sel].value;
}

MplexWidget::MplexWidget(Mplex *module) : ModuleWidget(module)
{
	box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/modules/mplex.svg")));		
		addChild(panel);
	}

	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addParam(ParamWidget::create<BefacoPushBig>(Vec(mm2px(25.322), yncscape(85.436, 8.999)), module, Mplex::BTUP, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<BefacoPushBig>(Vec(mm2px(25.322), yncscape(33.452, 8.999)), module, Mplex::BTDN, 0.0, 1.0, 0.0));
	addInput(Port::create<PJ301BPort>(Vec(mm2px(25.694), yncscape(71.230, 8.255)), Port::INPUT, module, Mplex::INUP));
	addInput(Port::create<PJ301BPort>(Vec(mm2px(25.694), yncscape(49.014, 8.255)), Port::INPUT, module, Mplex::INDN));
	addOutput(Port::create<PJ301GPort>(Vec(mm2px(40.045), yncscape(60.122, 8.255)), Port::OUTPUT, module, Mplex::OUT_1));

	float y = 105.068f;
	float x = 3.558f;
	float led_x = 13.843f;
	float y_offs = y - 108.108f;
	float delta_y = 92.529f - 105.068f;
	for(int k = 0; k < NUM_MPLEX_INPUTS; k++)
	{
		addInput(Port::create<PJ301GRPort>(Vec(mm2px(x), yncscape(y, 8.255)), Port::INPUT, module, Mplex::IN_1 + k));
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(mm2px(led_x), yncscape(y-y_offs, 2.176)), module, Mplex::LED_1 + k));
		y += delta_y;
		if(k == 3)
			y -= 2.117f;
	}
}

} // namespace rack_plugin_TheXOR

using namespace rack_plugin_TheXOR;

RACK_PLUGIN_MODEL_INIT(TheXOR, Mplex) {
	return Model::create<Mplex, MplexWidget>("TheXOR", "Mplex", "Mplex", SWITCH_TAG);
}
