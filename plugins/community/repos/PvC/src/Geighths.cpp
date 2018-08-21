/*

Geighths

cv input selects one out of eight outputs to fire a gate from.

assumed input range is 0-10V. full range is split into 8 segments.
with a clock signal connected, the unit goes into sample and hold mode.

PLANS/IDEAS:
	- option to use 8bit conversion to break up signal
	- internal clock (maybe)
	- random/stepped mode(s) with no cvIN but clockIN
	- trigger sum bus

*/////////////////////////////////////////////////////////////////////////////

#include "pvc.hpp"

#include "dsp/digital.hpp" // SchmittTrigger PulseGenerator

namespace rack_plugin_PvC {

struct Geighths : Module {
	enum ParamIds {
		INPUT_GAIN,
		INPUT_OFFSET,

		GATE1_LENGTH,

		NUM_PARAMS = GATE1_LENGTH + 8
	};
	enum InputIds {
		CV_IN,
		CLOCK_IN,

		NUM_INPUTS
	};
	enum OutputIds {
		GATE1_OUT,

		NUM_OUTPUTS = GATE1_OUT + 8
	};
	enum LightIds {
		GATE1_LIGHT,

		NUM_LIGHTS = GATE1_LIGHT + 8
	};

	SchmittTrigger clockTrigger;

	SchmittTrigger gateTrigger[8];
	PulseGenerator gatePulse[8];

	bool gateOn[8] {};

	float inVal = 0.0f;
	float sample = 0.0f;

	Geighths() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;
	void reset() override {
		inVal = sample = 0.0f;

		for (int i = 0; i < 8; i++)	{
			gateOn[i] = false;
		}
	}
};


void Geighths::step() {
	float input = inputs[CV_IN].normalize(0.0f) * params[INPUT_GAIN].value + params[INPUT_OFFSET].value;
	 // TODO: nicer input window scaling
	input = clamp(input, 0.0f, 10.0f);
	input = rescale(input, 0.0f, 10.0f, 0, 8);
		
	sample = clockTrigger.process(inputs[CLOCK_IN].value) ? input : sample;
	
	inVal = (inputs[CLOCK_IN].active) ? sample : input;

	// fire pulse on selected out
	for (int i = 0; i < 8; i++) {
		gateOn[i] = ((inVal > i) && (inVal < i+1)) ? true : false;

		if (gateTrigger[i].process(gateOn[i])) {
			gatePulse[i].trigger(params[GATE1_LENGTH + i].value);
		}

		outputs[GATE1_OUT + i].value = gatePulse[i].process(1.0/engineGetSampleRate()) * 10.0f;
		// lights[GATE1_LIGHT + i].value = ((gateOn[i]) || (gatePulse[i].process(1.0/engineGetSampleRate()))) ? 1.0f : 0.0f;
		lights[GATE1_LIGHT + i].value = gateOn[i] * 0.75f + gatePulse[i].process(1.0/engineGetSampleRate()) * 0.25f;
	}
}

struct GeighthsWidget : ModuleWidget {
	GeighthsWidget(Geighths *module);
};

GeighthsWidget::GeighthsWidget(Geighths *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/panels/Geighths.svg")));
	
	// screws
	addChild(Widget::create<ScrewHead1>(Vec(0, 0)));
	addChild(Widget::create<ScrewHead2>(Vec(box.size.x - 15, 0)));
	addChild(Widget::create<ScrewHead3>(Vec(0, 365)));
	addChild(Widget::create<ScrewHead4>(Vec(box.size.x - 15, 365)));

	addInput(Port::create<InPortAud>(Vec(4,22), Port::INPUT, module,Geighths::CV_IN));
	addInput(Port::create<InPortBin>(Vec(34,22), Port::INPUT, module,Geighths::CLOCK_IN));
	addParam(ParamWidget::create<PvCKnob>(Vec(4, 64),module,Geighths::INPUT_GAIN , -2.0f, 2.0f, 1.0f));
	addParam(ParamWidget::create<PvCKnob>(Vec(34, 64),module,Geighths::INPUT_OFFSET, -10.0f, 10.0f, 0.0f));

	for (int i = 0; i < 8; i++)
	{
		addChild(ModuleLightWidget::create<PvCBigLED<BlueLED>>(Vec(4,318 - 30*i),module,Geighths::GATE1_LIGHT + i));
		addParam(ParamWidget::create<PvCLEDKnob>(Vec(4, 318 - 30*i),module,Geighths::GATE1_LENGTH + i, 0.002f, 2.0f, 0.02f));
		addOutput(Port::create<OutPortBin>(Vec(34, 318 - 30*i), Port::OUTPUT, module,Geighths::GATE1_OUT + i));
	}
}

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;

RACK_PLUGIN_MODEL_INIT(PvC, Geighths) {
   Model *modelGeighths = Model::create<Geighths, GeighthsWidget>(
      "PvC", "Geighths", "Geighths", LOGIC_TAG, SAMPLE_AND_HOLD_TAG);
   return modelGeighths;
}
