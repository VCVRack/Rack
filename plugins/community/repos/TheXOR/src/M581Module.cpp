#include "M581.hpp"
#include <sstream>

namespace rack_plugin_TheXOR {

void M581::on_loaded()
{
	#ifdef DIGITAL_EXT
	connected = 0;
	#endif
	load();
}

void M581::load()
{
	stepCounter.Set(&getter);
	cvControl.Set(&getter);
	gateControl.Set(&getter);
	getter.Set(this);
	_reset();
}

void M581::_reset()
{
	cvControl.Reset();
	gateControl.Reset();
	stepCounter.Reset(&Timer);
	showCurStep(0, 0);
}

void M581::step()
{
	if(resetTrigger.process(inputs[RESET].value))
	{
		_reset();
	} else
	{
		Timer.Step();

		if(clockTrigger.process(inputs[CLOCK].value) && any())
			beginNewStep();

		outputs[CV].value = cvControl.Play(Timer.Elapsed());
		outputs[GATE].value = gateControl.Play(&Timer, stepCounter.PulseCounter());
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

void M581::beginNewStep()
{
	int cur_step;
	if(stepCounter.Play(&Timer, &cur_step)) // inizia un nuovo step?
	{
		gateControl.Begin(cur_step);
		cvControl.Begin(cur_step);	// 	glide note increment in 1/10 di msec. param = new note value
	}

	showCurStep(cur_step, stepCounter.PulseCounter());
}

void M581::showCurStep(int cur_step, int sub_div)
{
	int lled = cur_step;
	int sled = sub_div;
	for(int k = 0; k < 8; k++)
	{
		lights[LED_STEP + k].value = k == lled ? 1.0 : 0.0;
		lights[LED_SUBDIV + k].value = k == sled ? 1.0 : 0.0;
	}
}

bool M581::any()
{
	for(int k = 0; k < 8; k++)
	{
		if(getter.IsEnabled(k))  // step on?
			return true;
	}

	return false;
}

M581Widget::M581Widget(M581 *module) : SequencerWidget(module)
{
	#ifdef OSCTEST_MODULE
	char name[60];
	#endif

	box.size = Vec(29 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/modules/M581Module.svg")));
	addChild(panel);
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2*RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2*RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

	float dist_h = 11.893;
	for(int k = 0; k < 8; k++)
	{
		// page #0 (Session): step enable/disable; gate mode
			  // step enable
		ParamWidget *pwdg = ParamWidget::create<CKSSThreeFix>(Vec(mm2px(14.151+k*dist_h), yncscape(11.744,10.0)), module, M581::STEP_ENABLE + k, 0.0, 2.0, 1.0);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		LaunchpadRadio *radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(5, k), 3, LaunchpadLed::Color(43), LaunchpadLed::Color(32));
		module->drv->Add(radio, pwdg);
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Enable%i", k + 1);
		oscControl *oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		// Gate switches
		pwdg = ParamWidget::create<VerticalSwitch>(Vec(mm2px(14.930 + k*dist_h), yncscape(39.306, 13.2)), module, M581::GATE_SWITCH + k, 0.0, 3.0, 2.0);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(1, k), 4, LaunchpadLed::Color(11), LaunchpadLed::Color(17));
		module->drv->Add(radio, pwdg);
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/GateMode%i", k + 1);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		// page #1 (Note): Notes
		// step notes
		pwdg = ParamWidget::create<BefacoSlidePotFix>(Vec(mm2px(14.943 + k*dist_h), yncscape(95.822, 27.517)), module, M581::STEP_NOTES + k, 0.0, 1.0, 0.5);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		LaunchpadKnob *pknob = new LaunchpadKnob(1, ILaunchpadPro::RC2Key(6, k), LaunchpadLed::Rgb(10, 0, 0), LaunchpadLed::Rgb(63, 63, 63));
		module->drv->Add(pknob, pwdg);
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Knob%i", k + 1);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		//page #2 (Device): Counters
		// Counter switches
		pwdg = ParamWidget::create<CounterSwitch>(Vec(mm2px(14.93 + k*dist_h), yncscape(60.897, 24.0)), module, M581::COUNTER_SWITCH + k, 0.0, 7.0, 0.0);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		radio = new LaunchpadRadio(2, ILaunchpadPro::RC2Key(0, k), 8, LaunchpadLed::Color(1), LaunchpadLed::Color(58));
		module->drv->Add(radio, pwdg);
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Count%i", k + 1);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		// step leds (all pages)
		ModuleLightWidget *plight = ModuleLightWidget::create<LargeLight<RedLight>>(Vec(mm2px(13.491 + k*dist_h), yncscape(27.412, 5.179)), module, M581::LED_STEP + k);
		addChild(plight);
		#ifdef LAUNCHPAD
		LaunchpadLight *ld1 = new LaunchpadLight(launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(0, k), LaunchpadLed::Off(), LaunchpadLed::Color(5));
		module->drv->Add(ld1, plight);
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Led%i", k + 1);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, plight);
		#endif

		// subdiv leds (all pages)
		const float dv = 3.029;
		plight = ModuleLightWidget::create<TinyLight<RedLight>>(Vec(mm2px(11.642), yncscape(82.953-k*dv+0.272, 1.088)), module, M581::LED_SUBDIV + k);
		addChild(plight);
		#ifdef LAUNCHPAD
		ld1 = new LaunchpadLight(launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(8, k), LaunchpadLed::Off(), LaunchpadLed::Color(5));   // colonna PLAY
		module->drv->Add(ld1, plight);
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/SubLed%i", k + 1);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, plight);
		#endif
	}

	// Gate time
	ParamWidget *pwdg = ParamWidget::create<Davies1900hFixWhiteKnob>(Vec(mm2px(121.032), yncscape(112.942, 9.525)), module, M581::GATE_TIME, 0.005, 1.0, 0.25);
	#ifdef OSCTEST_MODULE
	sprintf(name, "/GateTime");
	oscControl *oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif
	addParam(pwdg);    // in sec

	// Slide time
	pwdg = ParamWidget::create<Davies1900hFixWhiteKnob>(Vec(mm2px(121.032), yncscape(95.480, 9.525)), module, M581::SLIDE_TIME, 0.005, 2.0, 0.5);
	addParam(pwdg); // in sec
	#ifdef OSCTEST_MODULE
	sprintf(name, "/SlideTime");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	// volt fondo scala
	pwdg = ParamWidget::create<CKSSFix>(Vec(mm2px(7.066), yncscape(114.224, 5.460)), module, M581::MAXVOLTS, 0.0, 1.0, 1.0);
	addParam(pwdg);
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Voltage");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	// step div
	pwdg = ParamWidget::create<VerticalSwitch>(Vec(mm2px(123.494), yncscape(75.482, 13.2)), module, M581::STEP_DIV, 0.0, 3.0, 0.0);
	addParam(pwdg);
	#ifdef OSCTEST_MODULE
	sprintf(name, "/StepDiv");
		oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	// input
	addInput(Port::create<PJ301RPort>(Vec(mm2px(113.864), yncscape(22.128, 8.255)), Port::INPUT, module, M581::CLOCK));
	addInput(Port::create<PJ301YPort>(Vec(mm2px(129.469), yncscape(22.128, 8.255)), Port::INPUT, module, M581::RESET));
	

	// OUTPUTS
	addOutput(Port::create<PJ301GPort>(Vec(mm2px(113.864), yncscape(7.228, 8.255)), Port::OUTPUT, module, M581::CV));
	addOutput(Port::create<PJ301WPort>(Vec(mm2px(129.469), yncscape(7.228, 8.255)), Port::OUTPUT, module, M581::GATE));

	// # STEPS
	SigDisplayWidget *display2 = new SigDisplayWidget(2);
	display2->box.pos = Vec(mm2px(127.229), yncscape(37.851, 9.525));
	display2->box.size = Vec(30, 20);
	display2->value = module->getAddress(1);
	addChild(display2);
	pwdg = ParamWidget::create<Davies1900hFixRedKnob>(Vec(mm2px(113.229), yncscape(38.851, 9.525)), module, M581::NUM_STEPS, 1.0, 31.0, 8.0);
	((Davies1900hKnob *)pwdg)->snap = true;
	addParam(pwdg);

	// run mode
	RunModeDisplay *display = new RunModeDisplay();
	display->box.pos = Vec(mm2px(127.229), yncscape(57.259, 9.525));
	display->box.size = Vec(42, 20);
	display->mode = module->getAddress(0);
	addChild(display);

	pwdg = ParamWidget::create<Davies1900hFixBlackKnob>(Vec(mm2px(113.229), yncscape(58.259,9.525)), module, M581::RUN_MODE, 0.0, 4.0, 0.0);
	((Davies1900hKnob *)pwdg)->snap = true;
	addParam(pwdg);

	#ifdef DIGITAL_EXT
	addChild(new DigitalLed(mm2px(92.540), yncscape(2.322,3.867), &module->connected));
	#endif
}

Menu *M581Widget::addContextMenu(Menu *menu)
{
	menu->addChild(new SeqMenuItem<M581Widget>("Randomize Pitch", this, RANDOMIZE_PITCH));
	menu->addChild(new SeqMenuItem<M581Widget>("Randomize Counters", this, RANDOMIZE_COUNTER));
	menu->addChild(new SeqMenuItem<M581Widget>("Randomize Modes", this, RANDOMIZE_MODE));
	menu->addChild(new SeqMenuItem<M581Widget>("Randomize Enable/Slide", this, RANDOMIZE_ENABLE));
	return menu;
}

void M581Widget::onMenu(int action)
{
	switch(action)
	{
	case RANDOMIZE_COUNTER: std_randomize(M581::COUNTER_SWITCH, M581::COUNTER_SWITCH + 8); break;
	case RANDOMIZE_PITCH: std_randomize(M581::STEP_NOTES, M581::STEP_NOTES + 8); break;
	case RANDOMIZE_MODE: std_randomize(M581::GATE_SWITCH, M581::GATE_SWITCH + 8); break;
	case RANDOMIZE_ENABLE: std_randomize(M581::STEP_ENABLE, M581::STEP_ENABLE + 8); break;
	}
}

bool ParamGetter::IsEnabled(int numstep) { return pModule->params[M581::STEP_ENABLE + numstep].value > 0.0; }
bool ParamGetter::IsSlide(int numstep) { return pModule->params[M581::STEP_ENABLE + numstep].value > 1.0; }
int ParamGetter::GateMode(int numstep) { return std::round(pModule->params[M581::GATE_SWITCH + numstep].value); }
int ParamGetter::PulseCount(int numstep) { return std::round(pModule->params[M581::COUNTER_SWITCH + numstep].value); }
float ParamGetter::Note(int numstep) { return pModule->params[M581::STEP_NOTES + numstep].value * (pModule->params[M581::MAXVOLTS].value > 0 ? 5.0 : 3.0); }
int ParamGetter::RunMode() { return std::round(pModule->params[M581::RUN_MODE].value); }
int ParamGetter::NumSteps() { return std::round(pModule->params[M581::NUM_STEPS].value); }
float ParamGetter::SlideTime() { return pModule->params[M581::SLIDE_TIME].value; }
float ParamGetter::GateTime() { return pModule->params[M581::GATE_TIME].value; }
int ParamGetter::StepDivision() { return std::round(pModule->params[M581::STEP_DIV].value) + 1; }

} // namespace rack_plugin_TheXOR

using namespace rack_plugin_TheXOR;

RACK_PLUGIN_MODEL_INIT(TheXOR, M581) {
	return Model::create<M581, M581Widget>("TheXOR", "M581", "581 Sequencer", SEQUENCER_TAG);
}
