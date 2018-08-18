#include "Spiralone.hpp"
#include "SpiraloneModule.hpp"
#include <math.h>

namespace rack_plugin_TheXOR {

float AccessParam(Spiralone *p, int seq, int id) { return p->params[id + seq].value; }
float AccessParam(Spiralone *p, int id) { return p->params[id].value; }
Input *AccessInput(Spiralone *p, int seq, int id) { return &p->inputs[id + seq]; }
float *AccessOutput(Spiralone *p, int seq, int id) { return &p->outputs[id + seq].value; }
float *AccessLight(Spiralone *p, int id) { return &p->lights[id].value; }

void Spiralone::on_loaded()
{
	#ifdef DIGITAL_EXT
	connected = 0;
	#endif
	load();
}

void Spiralone::load()
{
	for(int k = 0; k < NUM_SEQUENCERS; k++)
		sequencer[k].Reset(k, this);
}

void Spiralone::step()
{
	if(masterReset.process(params[M_RESET].value))
	{
		load();
	} else
	{
		for(int k = 0; k < NUM_SEQUENCERS; k++)
			sequencer[k].Step(k, this);
	}

	#ifdef DIGITAL_EXT
	bool dig_connected = false;

	#ifdef LAUNCHPAD
	if(drv->Connected())
		dig_connected = true;
	drv->ProcessLaunchpad();
	#endif

	#if defined(OSCTEST_MODULE)
	if(oscDrv->Connected())
		dig_connected = true;
	oscDrv->ProcessOSC();
	#endif	
	connected = dig_connected ? 1.0 : 0.0;
	#endif
}

SpiraloneWidget::SpiraloneWidget(Spiralone *module) : SequencerWidget(module)
{
	#ifdef OSCTEST_MODULE
	char name[60];
	#endif

	color[0] = COLOR_RED;
	color[1] = COLOR_WHITE;
	color[2] = COLOR_BLUE;
	color[3] = COLOR_YELLOW;
	color[4] = COLOR_GREEN;

	box.size = Vec(51 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/modules/SpiraloneModule.svg")));

	addChild(panel);
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

	float step = 2 * M_PI / TOTAL_STEPS;
	float angle = M_PI / 2.0;
	for(int k = 0; k < TOTAL_STEPS; k++)
	{
		float r = 56.0;
		float cx = cos(angle);
		float cy = sin(angle);
		float center_y = 64.250;
		float center_x = 66.894;
	
		ParamWidget *pctrl = ParamWidget::create<Davies1900hFixWhiteKnobSmall>(Vec(mm2px(center_x-4.0+r*cx), yncscape(center_y-4.0 +r*cy, 8.0)), module, Spiralone::VOLTAGE_1 + k, 0.0, 6.0, 1.0);
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Knob%i", k + 1);
		oscControl *oc = new oscControl(name);
		module->oscDrv->Add(oc, pctrl);
		#endif
		addParam(pctrl);
		
		r -= 2;
		for(int s = 0; s < NUM_SEQUENCERS; s++)
		{
			int n = s * TOTAL_STEPS + k;
			r -= 6;
			ModuleLightWidget *plight = createLed(s, Vec(mm2px(center_x-1.088+ r*cx), yncscape(center_y-1.088 + r*cy, 2.176)), module, Spiralone::LED_SEQUENCE_1 + n);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/Led%i_%i", s+1, n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
			addChild(plight);

			if(k == 0)
				createSequencer(s);
		}
		angle += step;
	}

	addParam(ParamWidget::create<BefacoPushBig>(Vec(mm2px(7.970), yncscape(113.627, 8.999)), module, Spiralone::M_RESET, 0.0, 1.0, 0.0));
	#ifdef DIGITAL_EXT
	addChild(new DigitalLed(mm2px(6.894), yncscape(8.250,3.867), &module->connected));
	#endif
}

void SpiraloneWidget::createSequencer(int seq)
{
	#ifdef OSCTEST_MODULE
	char name[60];
	#endif

	float dist_v = -25.206;
	
	addInput(Port::create<PJ301RPort>(Vec(mm2px(143.251), yncscape(115.825+dist_v*seq,8.255)), Port::INPUT, module, Spiralone::CLOCK_1 + seq));
	addInput(Port::create<PJ301YPort>(Vec(mm2px(143.251), yncscape(104.395+dist_v*seq,8.255)), Port::INPUT, module, Spiralone::RESET_1 + seq));

	ParamWidget *pwdg = ParamWidget::create<BefacoSnappedSwitch>(Vec(mm2px(158.607), yncscape(109.773 + dist_v*seq, 7.883)), module, Spiralone::MODE_1 + seq, 0.0, 1.0, 0.0);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	int color_launchpad[NUM_SEQUENCERS][2];
	color_launchpad[0][0] = 11; color_launchpad[0][1] = 5;
	color_launchpad[1][0] = 1; color_launchpad[1][1] = 3;
	color_launchpad[2][0] = 47; color_launchpad[2][1] = 37;
	color_launchpad[3][0] = 15; color_launchpad[3][1] = 12;
	color_launchpad[4][0] = 19; color_launchpad[4][1] = 21;
	LaunchpadRadio *sw = new LaunchpadRadio(0, 0, ILaunchpadPro::RC2Key(0, seq), 2, LaunchpadLed::Color(color_launchpad[seq][0]), LaunchpadLed::Color(color_launchpad[seq][1]));
	((Spiralone *)module)->drv->Add(sw, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Mode%i", seq + 1);
	oscControl *oc = new oscControl(name);
	((Spiralone *)module)->oscDrv->Add(oc, pwdg);
	#endif

	pwdg = ParamWidget::create<Davies1900hFixWhiteKnobSmall>(Vec(mm2px(175.427), yncscape(115.953 + dist_v*seq, 8.0)), module, Spiralone::LENGHT_1 + seq, 1.0, TOTAL_STEPS, TOTAL_STEPS);
	((Davies1900hKnob *)pwdg)->snap = true;
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Lenght%i", seq + 1);
	oc = new oscControl(name);
	((Spiralone *)module)->oscDrv->Add(oc, pwdg);
	#endif
	addParam(pwdg);
	addInput(Port::create<PJ301BPort>(Vec(mm2px(181.649), yncscape(104.395 + dist_v*seq, 8.0)), Port::INPUT, module, Spiralone::INLENGHT_1 + seq));

	pwdg = ParamWidget::create<Davies1900hFixWhiteKnobSmall>(Vec(mm2px(195.690), yncscape(115.953 + dist_v*seq, 8.255)), module, Spiralone::STRIDE_1 + seq, 1.0, 8.0, 1.0);
	((Davies1900hKnob *)pwdg)->snap = true;
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Stride%i", seq + 1);
	oc = new oscControl(name);
	((Spiralone *)module)->oscDrv->Add(oc, pwdg);
	#endif
	addParam(pwdg);
	addInput(Port::create<PJ301BPort>(Vec(mm2px(201.913), yncscape(104.395 + dist_v*seq, 8.255)), Port::INPUT, module, Spiralone::INSTRIDE_1 + seq));

	pwdg = ParamWidget::create<Davies1900hFixWhiteKnobSmall>(Vec(mm2px(215.954), yncscape(115.953 + dist_v*seq, 8.0)), module, Spiralone::XPOSE_1 + seq, -3.0, 3.0, 0.0);
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Transpose%i", seq + 1);
	oc = new oscControl(name);
	((Spiralone *)module)->oscDrv->Add(oc, pwdg);
	#endif
	addParam(pwdg);
	addInput(Port::create<PJ301BPort>(Vec(mm2px(222.177), yncscape(104.395 + dist_v*seq, 8.255)), Port::INPUT, module, Spiralone::INXPOSE_1 + seq));

	addOutput(Port::create<PJ301GPort>(Vec(mm2px(238.996), yncscape(115.825 + dist_v*seq, 8.255)), Port::OUTPUT, module, Spiralone::CV_1 + seq));
	addOutput(Port::create<PJ301WPort>(Vec(mm2px(238.996), yncscape(104.395 + dist_v*seq, 8.255)), Port::OUTPUT, module, Spiralone::GATE_1 + seq));
}

ModuleLightWidget *SpiraloneWidget::createLed(int seq, Vec pos, Module *module, int firstLightId, bool big)
{
	ModuleLightWidget * rv = new ModuleLightWidget();
	if(big)
		rv->box.size = mm2px(Vec(3, 3));
	else
		rv->box.size = mm2px(Vec(2.176, 2.176));
	rv->box.pos = pos;
	rv->addBaseColor(color[seq]);
	rv->module = module;
	rv->firstLightId = firstLightId;
	//rv->bgColor = COLOR_BLACK_TRANSPARENT;
	return rv;
}

Menu *SpiraloneWidget::addContextMenu(Menu *menu)
{
	menu->addChild(new SeqMenuItem<SpiraloneWidget>("Randomize Pitch", this, RANDOMIZE_PITCH));
	menu->addChild(new SeqMenuItem<SpiraloneWidget>("Randomize Length", this, RANDOMIZE_LEN));
	menu->addChild(new SeqMenuItem<SpiraloneWidget>("Randomize Stride", this, RANDOMIZE_STRIDE));
	menu->addChild(new SeqMenuItem<SpiraloneWidget>("Randomize Transpose", this, RANDOMIZE_XPOSE));
	return menu;
}

void SpiraloneWidget::onMenu(int action)
{
	switch(action)
	{
	case RANDOMIZE_PITCH:
		std_randomize(Spiralone::VOLTAGE_1, Spiralone::VOLTAGE_1 + TOTAL_STEPS);
		break;

	case RANDOMIZE_LEN:
		std_randomize(Spiralone::LENGHT_1, Spiralone::LENGHT_1 + NUM_SEQUENCERS);
		break;

	case RANDOMIZE_STRIDE:
		std_randomize(Spiralone::STRIDE_1, Spiralone::STRIDE_1 + NUM_SEQUENCERS);
		break;

	case RANDOMIZE_XPOSE:
		std_randomize(Spiralone::XPOSE_1, Spiralone::XPOSE_1 + NUM_SEQUENCERS);
		break;
	}
}

} // namespace rack_plugin_TheXOR

using namespace rack_plugin_TheXOR;

RACK_PLUGIN_MODEL_INIT(TheXOR, Spiralone) {
	return Model::create<Spiralone, SpiraloneWidget>("TheXOR", "Spiralone", "Spiralone Sequencer", SEQUENCER_TAG);
}
