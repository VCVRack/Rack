#include "common.hpp"
#ifdef LPTEST_MODULE
#include "lpTestModule.hpp"
#include <string.h>

namespace rack_plugin_TheXOR {

void LaunchpadTest::step()
{
	v_in = inputs[IN_V].value;
	lights[LP_CONNECTED].value = params[LaunchpadTest::BTN].value > 0 ? 1.0 : 0.0;

	outputs[KNOB_OUT].value = params[KNOB].value;
	outputs[BTN_OUT].value = inputs[BTN].value;
	outputs[SW_OUT].value = params[SW].value;
	drv->ProcessLaunchpad();
}

LaunchpadTestWidget::LaunchpadTestWidget(LaunchpadTest *module) : ModuleWidget(module)
{
	box.size = Vec(13 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/modules/test.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	int y = 40;
	SigDisplayWidget *display = new SigDisplayWidget(5, 2);
	display->box.pos = Vec(60, y);
	display->box.size = Vec(50 + 53, 24);
	display->value = &module->v_in;
	addChild(display);

	addInput(Port::create<PJ301RPort>(Vec(10, y), Port::INPUT, module, LaunchpadTest::IN_V));
	
	y += 60;

	ParamWidget *pctrl = ParamWidget::create<PatternBtn>(Vec(10, y), module, LaunchpadTest::BTN, 0.0, 1.0, 0.0);
	LaunchpadLed offColor;
	LaunchpadLed onColor;
	offColor.r_color = 20;
	onColor.r_color = 3;
	LaunchpadSwitch *sw1 = new LaunchpadSwitch(0, LaunchpadKey::R1C1, offColor, onColor);
	module->drv->Add(sw1, pctrl);
	addParam(pctrl);
	addOutput(Port::create<PJ301GPort>(Vec(150, y), Port::OUTPUT, module, LaunchpadTest::BTN_OUT));

	y += 60;

	LaunchpadKnob *pknob = new LaunchpadKnob(0, ILaunchpadPro::RC2Key(6, 6), LaunchpadLed::Rgb(20, 10, 10), LaunchpadLed::Rgb(60, 40, 40));
	ParamWidget *pEna = ParamWidget::create<Davies1900hBlackKnob>(Vec(10, y), module, LaunchpadTest::KNOB, 0.0, 5.0, 0.25);
	module->drv->Add(pknob, pEna);
	addParam(pEna);
	addOutput(Port::create<PJ301GPort>(Vec(150, y), Port::OUTPUT, module, LaunchpadTest::KNOB_OUT));

	y += 60;

	pEna = ParamWidget::create<BefacoSwitch>(Vec(10, y), module, LaunchpadTest::SW, 0.0, 2.0, 1.0);
	addParam(pEna);
	addOutput(Port::create<PJ301GPort>(Vec(150, y), Port::OUTPUT, module, LaunchpadTest::SW_OUT));

#ifdef DEBUG
	info("RDY");
#endif
		
}

} // namespace rack_plugin_TheXOR

#endif
