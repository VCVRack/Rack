#include "pwmClock.hpp"

namespace rack_plugin_TheXOR {

#define BPM_MINVALUE (10)
#define BPM_MAXVALUE (300)
#define PWM_MINVALUE (0.05)
#define PWM_MAXVALUE (0.95)
#define SWING_MINVALUE (0.0)
#define SWING_MAXVALUE (0.5)
void PwmClock::on_loaded()
{
	bpm = 0;
	swing = 0;
	_reset();
	load();
}

void PwmClock::_reset()
{
	for(int k = 0; k < OUT_SOCKETS; k++)
	{
		sa_timer[k].Reset();
		odd_beat[k] = false;
	}
}

void PwmClock::load()
{
	updateBpm();
}

void PwmClock::updateBpm()
{
	bool updated = false;
	float new_bpm;
	if(inputs[EXT_BPM].active)
		new_bpm = rescale(inputs[EXT_BPM].value, LVL_OFF, LVL_ON, BPM_MINVALUE, BPM_MAXVALUE);
	else
		new_bpm = (roundf(params[BPMDEC].value) + 10 * bpm_integer) / 10.0;

	if(bpm != new_bpm)
	{
		updated = true;
		bpm = new_bpm;
		duration[0] = 240.0 / bpm;	// 1/1
		duration[1] = duration[0] + duration[0] / 2.0;
		duration[2] = 2.0* duration[0] / 3.0;

		for(int k = 1; k < 7; k++)
		{
			duration[3 * k] = duration[3 * (k - 1)] / 2.0;
			duration[3 * k + 1] = duration[3 * (k - 1) + 1] / 2.0;
			duration[3 * k + 2] = duration[3 * (k - 1) + 2] / 2.0;
		}
	}
	float new_swing = getSwing();
	if(updated || new_swing != swing)
	{
		swing = new_swing;
		for(int k = 0; k < OUT_SOCKETS; k++)
			swingAmt[k] = duration[k] + duration[k] * swing;
	}
}

void PwmClock::process_keys()
{
	if(btnup.process(params[BPM_INC].value))
	{
		if(bpm_integer < BPM_MAXVALUE)
			bpm_integer += 1;
		pWidget->SetBpm(bpm_integer);
	}

	if(btndwn.process(params[BPM_DEC].value))
	{
		if(bpm_integer > 0)
			bpm_integer -= 1;
		pWidget->SetBpm(bpm_integer);
	}
}

void PwmClock::step()
{
	process_keys();
	bpm_integer = roundf(params[BPM].value);
	updateBpm();

	if((params[OFFON].value + inputs[OFFON_IN].value) > 0.5)
	{
		lights[ACTIVE].value = LVL_ON;
		if(resetTrigger.process(inputs[RESET].value))
		{
			_reset();
		} else
		{
			for(int k = 0; k < OUT_SOCKETS; k++)
			{
				float gate_len = getDuration(k) * getPwm();
				sa_timer[k].Step();
				float elps = sa_timer[k].Elapsed();
				if(elps >= getDuration(k))
				{
					elps = sa_timer[k].Reset();
					odd_beat[k] = !odd_beat[k];
				}
				if(elps <= gate_len)
					outputs[OUT_1 + k].value = LVL_ON;
				else
					outputs[OUT_1 + k].value = LVL_OFF;
			}
		}
	} else
	{
		for(int k = 0; k < OUT_SOCKETS; k++)
		{
			outputs[OUT_1 + k].value = LVL_OFF;
		}
		lights[ACTIVE].value = LVL_OFF;
	}
}

float PwmClock::getPwm()
{
	float offs = inputs[PWM_IN].active ? rescale(inputs[PWM_IN].value, 0.0, 5.0, PWM_MINVALUE, PWM_MAXVALUE) : 0.0;
	return clamp(offs + params[PWM].value, PWM_MINVALUE, PWM_MAXVALUE);
}

float PwmClock::getSwing()
{
	float offs = inputs[SWING_IN].active ? rescale(inputs[SWING_IN].value, 0.0, 5.0, SWING_MINVALUE, SWING_MAXVALUE) : 0.0;
	return clamp(offs + params[SWING].value, SWING_MINVALUE, SWING_MAXVALUE);
}

PwmClockWidget::PwmClockWidget(PwmClock *module) : SequencerWidget(module)
{
	module->setWidget(this);
	box.size = Vec(15 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;

		panel->setBackground(SVG::load(assetPlugin(plugin, "res/modules/PwmClock.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewBlack>(Vec(15, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(15, 365)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 30, 365)));

	addParam(ParamWidget::create<UPSWITCH>(Vec(mm2px(7.572), yncscape(104.588,4.115)), module, PwmClock::BPM_INC, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<DNSWITCH>(Vec(mm2px(7.572), yncscape(99.788, 4.115)), module, PwmClock::BPM_DEC, 0.0, 1.0, 0.0));

	SigDisplayWidget *display = new SigDisplayWidget(4, 1);
	display->box.pos = Vec(mm2px(20), RACK_GRID_HEIGHT-mm2px(108));
	display->box.size = Vec(30+53, 24);
	display->value = &module->bpm;
	addChild(display);
	
	ParamWidget *pw = ParamWidget::create<Davies1900hFixWhiteKnobSmall>(Vec(mm2px(50.364), yncscape(100.245, 8)), module, PwmClock::BPMDEC, 0.0, 9.0, 0.0);
	((Davies1900hKnob *)pw)->snap = true;
	addParam(pw);
	pw = ParamWidget::create<Davies1900hFixWhiteKnob>(Vec(mm2px(62.528), yncscape(99.483, 9.525)), module, PwmClock::BPM, BPM_MINVALUE, BPM_MAXVALUE, 120.0);
	((Davies1900hKnob *)pw)->snap = true;
	addParam(pw);
	addInput(Port::create<PJ301BPort>(Vec(mm2px(50.326), yncscape(86.857, 8.255)), Port::INPUT, module, PwmClock::EXT_BPM));
	addInput(Port::create<PJ301YPort>(Vec(mm2px(63.162), yncscape(86.857, 8.255)), Port::INPUT, module, PwmClock::RESET));

	addParam(ParamWidget::create<NKK2>(Vec(mm2px(49.040), yncscape(64.997, 9.488)), module, PwmClock::OFFON, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(mm2px(59.141), yncscape(67.715, 2.176)), module, PwmClock::ACTIVE));
	addInput(Port::create<PJ301BPort>(Vec(mm2px(63.162), yncscape(64.675, 8.255)), Port::INPUT, module, PwmClock::OFFON_IN));

	addParam(ParamWidget::create<Davies1900hFixRedKnob>(Vec(mm2px(48.511), yncscape(42.040, 9.525)), module, PwmClock::SWING, SWING_MINVALUE, SWING_MAXVALUE, SWING_MINVALUE));
	addInput(Port::create<PJ301BPort>(Vec(mm2px(63.162), yncscape(42.675, 8.255)), Port::INPUT, module, PwmClock::SWING_IN));

	addParam(ParamWidget::create<Davies1900hFixBlackKnob>(Vec(mm2px(48.511), yncscape(20.040, 9.525)), module, PwmClock::PWM, PWM_MINVALUE, PWM_MAXVALUE, 0.5));
	addInput(Port::create<PJ301BPort>(Vec(mm2px(63.162), yncscape(20.675, 8.255)), Port::INPUT, module, PwmClock::PWM_IN));

	float col_x[3] = {7.875, 21.633, 35.392};
	float pos_y = yncscape(70.175, 8.255);
	int col = 0;
	for(int k = 0; k < OUT_SOCKETS; k++)
	{
		addOutput(Port::create<PJ301OPort>(Vec(mm2px(col_x[col]), pos_y), Port::OUTPUT, module, PwmClock::OUT_1 + k));
		if(++col >= 3)
		{
			col = 0;
			pos_y += mm2px(11);
		}
	}
}

void PwmClockWidget::SetBpm(float bpm_integer)
{
	int index = getParamIndex(PwmClock::BPM);
	if(index >= 0)
	{
		// VCV interface update is ahem.... migliorabile....
		bool smooth = params[index]->smooth;
		params[index]->smooth = false;
		params[index]->setValue((double)bpm_integer);
		params[index]->smooth = smooth;
	}
}

} // namespace rack_plugin_TheXOR

using namespace rack_plugin_TheXOR;

RACK_PLUGIN_MODEL_INIT(TheXOR, PwmClock) {
	return Model::create<PwmClock, PwmClockWidget>("TheXOR", "PWMClock", "PWM Clock", CLOCK_TAG);
}
