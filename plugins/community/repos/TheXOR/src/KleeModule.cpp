#include "Klee.hpp"

namespace rack_plugin_TheXOR {

void Klee::on_loaded()
{
	#ifdef DIGITAL_EXT
	connected = 0;
	#endif
	load();
}

void Klee::step()
{
	float deltaTime = 1.0 / engineGetSampleRate();

	if(loadTrigger.process(params[LOAD_PARAM].value + inputs[LOAD_INPUT].value))
	{
		load();
	}

	int clk = clockTrigger.process(inputs[EXT_CLOCK_INPUT].value + params[STEP_PARAM].value); // 1=rise, -1=fall
	if(clk == 1)
	{
		sr_rotate();
		update_bus();
		populate_outputs();
	}

	if(clk != 0)
	{
		populate_gate(clk);
	}

	check_triggers(deltaTime);

	showValues();

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

void Klee::load()
{
	for(int k = 0; k < 16; k++)
	{
		shiftRegister.P[k] = isSwitchOn(LOAD_BUS + k);
	}
}


void Klee::update_bus()
{
	bool bus1 = bus_active[0];
	for(int k = 0; k < 3; k++)
		bus_active[k] = false;

	for(int k = 0; k < 16; k++)
	{
		if(shiftRegister.P[k])
		{
			bus_active[getValue3(k)] = true;
		}
	}
	if(isSwitchOn(BUS2_MODE))
		bus_active[1] = bus_active[0] && bus_active[2];
	else
		bus_active[1] &= !(bus_active[0] || bus_active[2]);  //BUS 2: NOR 0 , 3

	//bus1 load
	if(isSwitchOn(BUS1_LOAD) && !bus1 && bus_active[0])
		load();
}

int Klee::getValue3(int k)
{
	int v = roundf(params[GROUPBUS + k].value);
	return 2-v;
}

bool Klee::isSwitchOn(int ptr)
{
	return params[ptr].value > 0.1;
}

void Klee::check_triggers(float deltaTime)
{
	for(int k = 0; k < 3; k++)
	{
		if(outputs[TRIG_OUT + k].value > 0.5 && !triggers[k].process(deltaTime))
		{
			outputs[TRIG_OUT + k].value = LVL_OFF;
		}
	}
}

void Klee::populate_gate(int clk)
{
	for(int k = 0; k < 3; k++)
	{
		// gate
		if(clk == 1)  // rise
		{
			outputs[GATE_OUT + k].value = bus_active[k] ? LVL_ON : LVL_OFF;
		} else // fall
		{
			if(!bus_active[k] || !isSwitchOn(BUS_MERGE + k))
				outputs[GATE_OUT + k].value = LVL_OFF;
		}
	}
}

void Klee::populate_outputs()
{
	for(int k = 0; k < 3; k++)
	{
		if(bus_active[k])
		{
			outputs[TRIG_OUT + k].value = LVL_ON;
			triggers[k].trigger(pulseTime);
		}
	}

	float a = 0, b = 0;
	float mult = params[RANGE].value + inputs[RANGE_IN].value;

	for(int k = 0; k < 8; k++)
	{
		if(shiftRegister.A[k])
			a += params[PITCH_KNOB + k].value*mult;

		if(shiftRegister.B[k])
			b += params[PITCH_KNOB + k + 8].value*mult;
	}
	outputs[CV_A].value = a;
	outputs[CV_B].value = b;
	outputs[CV_AB].value = a + b;
	outputs[CV_A__B].value = a - b;
}

void Klee::showValues()
{
	for(int k = 0; k < 16; k++)
	{
		lights[LED_PITCH + k].value = shiftRegister.P[k] ? 1.0 : 0;
	}

	for(int k = 0; k < 3; k++)
	{
		lights[LED_BUS + k].value = outputs[GATE_OUT + k].value;
	}
}

void Klee::sr_rotate()
{
	if(!isSwitchOn(X28_X16))  // mode 1 x 16
	{
		int fl = shiftRegister.P[15];
		for(int k = 15; k > 0; k--)
		{
			shiftRegister.P[k] = shiftRegister.P[k - 1];
		}
		if(isSwitchOn(RND_PAT))
			shiftRegister.P[0] = chance();
		else
			shiftRegister.P[0] = fl;
	} else
	{
		int fla = shiftRegister.A[7];
		int flb = shiftRegister.B[7];
		for(int k = 7; k > 0; k--)
		{
			shiftRegister.A[k] = shiftRegister.A[k - 1];
			shiftRegister.B[k] = shiftRegister.B[k - 1];
		}
		if(isSwitchOn(RND_PAT))
			shiftRegister.A[0] = chance();
		else
			shiftRegister.A[0] = fla;
		shiftRegister.B[0] = isSwitchOn(B_INV) ? !flb : flb;
	}
}

bool Klee::chance()
{
	return rand() <= (params[RND_THRESHOLD].value + inputs[RND_THRES_IN].value) * RAND_MAX;
}

KleeWidget::KleeWidget(Klee *module) : SequencerWidget(module)
{
	float nkk_offs = 2.3;
	#ifdef OSCTEST_MODULE
	char name[60];
	#endif

	#ifdef LAUNCHPAD
	int numLaunchpads = module->drv->GetNumLaunchpads();
	#ifdef DEBUG
	info("%i launchpad found", numLaunchpads);
	#endif
	#endif
	box.size = Vec(48 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/modules/KleeModule.svg")));
	addChild(panel);
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

	const float switch_dstx = 22.203 - 11.229;
	for(int k = 0; k < 8; k++)
	{
		// Load switches
		ParamWidget *pwdg = ParamWidget::create<NKK2>(Vec(mm2px(k*switch_dstx+11.229), yncscape(114.071+ nkk_offs,7.336)), module, Klee::LOAD_BUS + k, 0.0, 1.0, 0.0);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		if(numLaunchpads > 1)
		{
			LaunchpadSwitch *sw = new LaunchpadSwitch(0, 0, ILaunchpadPro::RC2Key(1, k), LaunchpadLed::Color(11), LaunchpadLed::Color(5));
			module->drv->Add(sw, pwdg);
		} else
		{
			LaunchpadSwitch *sw = new LaunchpadSwitch(0, ILaunchpadPro::RC2Key(2, k), LaunchpadLed::Color(11), LaunchpadLed::Color(5));
			module->drv->Add(sw, pwdg);
		}
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Load%i", k+1);
		oscControl *oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		pwdg = ParamWidget::create<NKK2>(Vec(mm2px(k*switch_dstx + 118.914), yncscape(114.071+nkk_offs, 7.336)), module, Klee::LOAD_BUS + k + 8, 0.0, 1.0, 0.0);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		if(numLaunchpads > 1)
		{
			LaunchpadSwitch *sw = new LaunchpadSwitch(1, 0, ILaunchpadPro::RC2Key(1, k), LaunchpadLed::Color(19), LaunchpadLed::Color(17));
			module->drv->Add(sw, pwdg);
		} else
		{
			LaunchpadSwitch *sw = new LaunchpadSwitch(0, ILaunchpadPro::RC2Key(3, k), LaunchpadLed::Color(19), LaunchpadLed::Color(17));
			module->drv->Add(sw, pwdg);
		}
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Load%i", k + 9);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		// BUS switches
		pwdg = ParamWidget::create<NKK2>(Vec(mm2px(k*switch_dstx + 11.229), yncscape(4.502 + nkk_offs, 7.336)), module, Klee::GROUPBUS + k, 0.0, 2.0, 2.0);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		if(numLaunchpads > 1)
		{
			LaunchpadRadio *radio = new LaunchpadRadio(0, 0, ILaunchpadPro::RC2Key(5, k), 3, LaunchpadLed::Color(7), LaunchpadLed::Color(5));
			module->drv->Add(radio, pwdg);
		} else
		{
			LaunchpadThree *three = new LaunchpadThree(0, ILaunchpadPro::RC2Key(6, k), LaunchpadLed::Color(7), LaunchpadLed::Color(6), LaunchpadLed::Color(5));
			module->drv->Add(three, pwdg);
		}
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Bus%i", k + 1);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		pwdg = ParamWidget::create<NKK2>(Vec(mm2px(k*switch_dstx + 118.914), yncscape(4.502 + nkk_offs, 7.336)), module, Klee::GROUPBUS + k + 8, 0.0, 2.0, 2.0);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		if(numLaunchpads > 1)
		{
			LaunchpadRadio *radio = new LaunchpadRadio(1, 0, ILaunchpadPro::RC2Key(5, k), 3, LaunchpadLed::Color(17), LaunchpadLed::Color(19));
			module->drv->Add(radio, pwdg);
		} else
		{
			LaunchpadThree *three = new LaunchpadThree(0, ILaunchpadPro::RC2Key(7, k), LaunchpadLed::Color(17), LaunchpadLed::Color(18), LaunchpadLed::Color(19));
			module->drv->Add(three, pwdg);
		}
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Bus%i", k + 9);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif
	}

	// trig/gate out
	float dist_v = 60.582 - 76.986;
	for(int k = 0; k < 3; k++)
	{
		ParamWidget *pwdg = ParamWidget::create<NKK2>(Vec(mm2px(184.360), yncscape(76.986 + nkk_offs +k*dist_v, 7.336)), module, Klee::BUS_MERGE + k, 0.0, 1.0, 0.0);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		LaunchpadSwitch *sw = new LaunchpadSwitch(ALL_LAUNCHPADS, launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(8, 4 + k), LaunchpadLed::Color(11), LaunchpadLed::Color(52));
		module->drv->Add(sw, pwdg);
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Merge%i", k + 1);
		oscControl *oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		ModuleLightWidget *plight = ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(mm2px(205.425), yncscape(78.065 + k*dist_v, 5.179)), module, Klee::LED_BUS + k);
		addChild(plight);
		#ifdef OSCTEST_MODULE
		sprintf(name, "/BusLed%i", k + 1);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, plight);
		#endif

		addOutput(Port::create<PJ301BLUPort>(Vec(mm2px(213.360), yncscape(76.986+k*dist_v, 8.255)), Port::OUTPUT, module, Klee::TRIG_OUT + k));
		addOutput(Port::create<PJ301WPort>(Vec(mm2px(230.822), yncscape(76.986+k*dist_v, 8.255)), Port::OUTPUT, module, Klee::GATE_OUT + k));
	}
	ParamWidget *pwdg = ParamWidget::create<CKSSFix>(Vec(mm2px(172.113), yncscape(61.520, 5.460)), module, Klee::BUS2_MODE, 0.0, 1.0, 0.0);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	LaunchpadSwitch *sw = new LaunchpadSwitch(ALL_LAUNCHPADS, launchpadDriver::ALL_PAGES, LaunchpadKey::STOP_CLIP, LaunchpadLed::Color(55), LaunchpadLed::Color(57));
	module->drv->Add(sw, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Bus2Mode");
	oscControl *oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	//load
	pwdg = ParamWidget::create<BefacoPushBig>(Vec(mm2px(25.360), yncscape(76.686,8.999)), module, Klee::LOAD_PARAM, 0.0, 1.0, 0.0);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	LaunchpadMomentary *mom = new LaunchpadMomentary(ALL_LAUNCHPADS, launchpadDriver::ALL_PAGES, LaunchpadKey::RECORD, LaunchpadLed::Color(1), LaunchpadLed::Color(43));
	module->drv->Add(mom, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Load");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	addInput(Port::create<PJ301BPort>(Vec(mm2px(9.218), yncscape(77.058, 8.255)), Port::INPUT, module, Klee::LOAD_INPUT));

	pwdg = ParamWidget::create<NKK2>(Vec(mm2px(25.627), yncscape(91.395 + nkk_offs, 7.336)), module, Klee::BUS1_LOAD, 0.0, 1.0, 0.0);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	sw = new LaunchpadSwitch(ALL_LAUNCHPADS, launchpadDriver::ALL_PAGES, LaunchpadKey::RECORD_ARM, LaunchpadLed::Color(31), LaunchpadLed::Color(33));
	module->drv->Add(sw, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Bus1Load");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	//step
	pwdg = ParamWidget::create<BefacoPushBig>(Vec(mm2px(25.360), yncscape(24.737, 8.999)), module, Klee::STEP_PARAM, 0.0, 1.0, 0.0);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	mom = new LaunchpadMomentary(ALL_LAUNCHPADS, launchpadDriver::ALL_PAGES, LaunchpadKey::CLICK, LaunchpadLed::Color(1), LaunchpadLed::Color(9));
	module->drv->Add(mom, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Step");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	addInput(Port::create<PJ301RPort>(Vec(mm2px(9.218), yncscape(25.109, 8.255)), Port::INPUT, module, Klee::EXT_CLOCK_INPUT));

	// CV Out
	addOutput(Port::create<PJ301GPort>(Vec(mm2px(213.360), yncscape(113.612, 8.255)), Port::OUTPUT, module, Klee::CV_A));
	addOutput(Port::create<PJ301GPort>(Vec(mm2px(230.822), yncscape(113.612, 8.255)), Port::OUTPUT, module, Klee::CV_B));
	addOutput(Port::create<PJ301GPort>(Vec(mm2px(213.360), yncscape(97.207, 8.255)), Port::OUTPUT, module, Klee::CV_A__B));
	addOutput(Port::create<PJ301GPort>(Vec(mm2px(230.822), yncscape(97.207, 8.255)), Port::OUTPUT, module, Klee::CV_AB));

	// mode
	pwdg = ParamWidget::create<NKK2>(Vec(mm2px(68.915), yncscape(60.582 + nkk_offs, 7.336)), module, Klee::X28_X16, 0.0, 1.0, 0.0);
	addParam(pwdg);     // 2x8 1x16
	#ifdef LAUNCHPAD
	sw = new LaunchpadSwitch(ALL_LAUNCHPADS, launchpadDriver::ALL_PAGES, LaunchpadKey::TRACK_SELECT, LaunchpadLed::Color(1), LaunchpadLed::Color(62));
	module->drv->Add(sw, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Mode");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	pwdg = ParamWidget::create<NKK2>(Vec(mm2px(97.459), yncscape(60.582 + nkk_offs, 7.336)), module, Klee::RND_PAT, 0.0, 1.0, 0.0);
	addParam(pwdg);     // rnd/pattern
	#ifdef LAUNCHPAD
	sw = new LaunchpadSwitch(ALL_LAUNCHPADS, launchpadDriver::ALL_PAGES, LaunchpadKey::MUTE, LaunchpadLed::Color(1), LaunchpadLed::Color(62));
	module->drv->Add(sw, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Random");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	pwdg = ParamWidget::create<NKK2>(Vec(mm2px(126.004), yncscape(60.582 + nkk_offs, 7.336)), module, Klee::B_INV, 0.0, 1.0, 0.0);
	addParam(pwdg);     // norm /B inverted
	#ifdef LAUNCHPAD
	sw = new LaunchpadSwitch(ALL_LAUNCHPADS, launchpadDriver::ALL_PAGES, LaunchpadKey::SOLO, LaunchpadLed::Color(1), LaunchpadLed::Color(62));
	module->drv->Add(sw, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Invert");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	// CV Range
	pwdg = ParamWidget::create<Davies1900hFixBlackKnob>(Vec(mm2px(212.725), yncscape(24.474,9.525)), module, Klee::RANGE, 0.0001, 5.0, 1.0);
	addParam(pwdg);
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Range");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif
	addInput(Port::create<PJ301BPort>(Vec(mm2px(230.822), yncscape(25.109, 8.255)), Port::INPUT, module, Klee::RANGE_IN));

	// RND Threshold
	pwdg = ParamWidget::create<Davies1900hFixBlackKnob>(Vec(mm2px(212.725), yncscape(9.228, 9.525)), module, Klee::RND_THRESHOLD, 0.0, 1.0, 0.0);
	addParam(pwdg);     // rnd threshold
	#ifdef OSCTEST_MODULE
	sprintf(name, "/RndTH");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif
	addInput(Port::create<PJ301BPort>(Vec(mm2px(230.822), yncscape(9.863, 8.255)), Port::INPUT, module, Klee::RND_THRES_IN));

	// pitch Knobs + leds
	float pot_x[8] = {39.440, 45.104, 60.976, 83.912, 109.368, 132.304, 148.175, 153.840};
	float led_x[8] = {51.727, 56.481, 69.800, 89.046, 110.408, 129.655, 142.974, 147.727};
	float pot_y_sup[8] = {66.558, 79.131, 89.222, 94.823, 94.823, 89.222, 79.131, 66.558};
	float pot_y_inf[8] = {52.426, 39.814, 29.751, 24.152, 24.152, 29.751, 39.814, 52.426};
	float led_y_sup[8] = {67.079, 76.624, 84.279, 88.527, 88.527, 84.279, 76.624, 67.079};
	float led_y_inf[8] = {58.245, 48.700, 41.045, 36.797, 36.797, 41.045, 48.700, 58.245};
	for(int k = 0; k < 8; k++)
	{
		pwdg = ParamWidget::create<Davies1900hFixRedKnob>(Vec(mm2px(pot_x[k]), yncscape(pot_y_sup[k], 9.525)), module, Klee::PITCH_KNOB + k, 0.0, 1.0, 0.125);
		addParam(pwdg);
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Knob%i", k+1);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		ModuleLightWidget *plight = ModuleLightWidget::create<MediumLight<RedLight>>(Vec(mm2px(led_x[k]), yncscape(led_y_sup[k], 3.176)), module, Klee::LED_PITCH + k);
		addChild(plight);
		#ifdef LAUNCHPAD
		if(numLaunchpads > 1)
		{
			LaunchpadLight *ld1 = new LaunchpadLight(0, launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(0, k), LaunchpadLed::Off(), LaunchpadLed::Color(5));
			module->drv->Add(ld1, plight);
		} else
		{
			LaunchpadLight *ld1 = new LaunchpadLight(launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(0, k), LaunchpadLed::Off(), LaunchpadLed::Color(5));
			module->drv->Add(ld1, plight);
		}
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Ledbpm_integer");
		oc = new oscControl(name);
		module->oscDrv->Add(oc, plight);
		#endif

		pwdg = ParamWidget::create<Davies1900hFixWhiteKnob>(Vec(mm2px(pot_x[7-k]), yncscape(pot_y_inf[k], 9.525)), module, Klee::PITCH_KNOB + 8 + k, 0.0, 1.0, 0.125);
		addParam(pwdg);
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Knob%i", k + 9);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, pwdg);
		#endif

		plight = ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(mm2px(led_x[7-k]), yncscape(led_y_inf[k], 3.176)), module, Klee::LED_PITCH + k + 8);
		addChild(plight);
		#ifdef LAUNCHPAD
		if(numLaunchpads > 1)
		{
			LaunchpadLight *ld1 = new LaunchpadLight(1, launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(0, k), LaunchpadLed::Off(), LaunchpadLed::Color(21));
			module->drv->Add(ld1, plight);
		} else
		{
			LaunchpadLight *ld1 = new LaunchpadLight(launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(1, k), LaunchpadLed::Off(), LaunchpadLed::Color(21));
			module->drv->Add(ld1, plight);
		}
		#endif
		#ifdef OSCTEST_MODULE
		sprintf(name, "/Led%i", k + 9);
		oc = new oscControl(name);
		module->oscDrv->Add(oc, plight);
		#endif
	}

	#ifdef DIGITAL_EXT
	addChild(new DigitalLed(mm2px(104.205), yncscape(115.806, 3.867), &module->connected));
	#endif
}

Menu *KleeWidget::addContextMenu(Menu *menu)
{
	menu->addChild(new SeqMenuItem<KleeWidget>("Range -> 1V", this, SET_RANGE_1V));
	menu->addChild(new SeqMenuItem<KleeWidget>("Randomize Pitch", this, RANDOMIZE_PITCH));
	menu->addChild(new SeqMenuItem<KleeWidget>("Randomize Bus", this, RANDOMIZE_BUS));
	menu->addChild(new SeqMenuItem<KleeWidget>("Randomize Load", this, RANDOMIZE_LOAD));
	return menu;
}

void KleeWidget::onMenu(int action)
{
	switch(action)
	{
	case RANDOMIZE_BUS: std_randomize(Klee::GROUPBUS, Klee::GROUPBUS + 16); break;
	case RANDOMIZE_PITCH: std_randomize(Klee::PITCH_KNOB, Klee::PITCH_KNOB + 16); break;
	case RANDOMIZE_LOAD: std_randomize(Klee::LOAD_BUS, Klee::LOAD_BUS + 16); break;
	case SET_RANGE_1V:
	{
		int index = getParamIndex(Klee::RANGE);
		if(index >= 0)
			params[index]->setValue(1.0);
	}
	break;
	}
}

} // namespace rack_plugin_TheXOR

using namespace rack_plugin_TheXOR;

RACK_PLUGIN_MODEL_INIT(TheXOR, Klee) {
	return Model::create<Klee, KleeWidget>("TheXOR", "Klee", "Klee Sequencer", SEQUENCER_TAG);
}
