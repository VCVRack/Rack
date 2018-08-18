#include "common.hpp"
#include "attenuator.hpp"

namespace rack_plugin_TheXOR {

void Attenuator::step()
{
	for(int k = 0; k < NUM_ATTENUATORS; k++)
	{
		if(outputs[OUT_1 + k].active)
			outputs[OUT_1 + k].value = inputs[IN_1 + k].value * params[ATT_1 + k].value;
	}
}

AttenuatorWidget::AttenuatorWidget(Attenuator *module) : ModuleWidget(module)
{
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/modules/attenuator.svg")));		
		addChild(panel);
	}

	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	float in_x = mm2px(2.490);
	float pot_x = mm2px(16.320);
	float out_x = mm2px(29.894);
	float y = yncscape(107.460, 8.255);
	float ypot = yncscape(107.588, 8.0);
	float delta_y = mm2px(19.0);
	
	for(int k = 0; k < NUM_ATTENUATORS; k++)
	{
		addInput(Port::create<PJ301GRPort>(Vec(in_x, y), Port::INPUT, module, Attenuator::IN_1 + k));
		addParam(ParamWidget::create<Davies1900hFixWhiteKnobSmall>(Vec(pot_x, ypot), module, Attenuator::ATT_1+k, 0.0, 1.0, 1.0));
		addOutput(Port::create<PJ301GPort>(Vec(out_x, y), Port::OUTPUT, module, Attenuator::OUT_1+k));
		y += delta_y;
		ypot += delta_y;
	}
}

} // namespace rack_plugin_TheXOR

using namespace rack_plugin_TheXOR;

RACK_PLUGIN_MODEL_INIT(TheXOR, Attenuator) {
	return Model::create<Attenuator, AttenuatorWidget>("TheXOR", "Attenuator", "Attenuator", ATTENUATOR_TAG);
}

