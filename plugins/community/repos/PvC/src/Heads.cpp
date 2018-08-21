/*

Heads


*/////////////////////////////////////////////////////////////////////////////



#include "pvc.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_PvC {

struct Heads : Module {
	enum ParamIds {
		PROB_UI,
		TOSS_MODE,
		TOSS_UI,
		FLIP_UI,
		SET_A_UI,
		SET_B_UI,

		NUM_PARAMS
	};
	enum InputIds {
		SIG_A_IN,
		SIG_B_IN,
		PROB_CV,
		TOSS_IN,
		FLIP_IN,
		SET_A_IN,
		SET_B_IN,

		NUM_INPUTS
	};
	enum OutputIds {
		SIG_OUT,
				
		GATE_A_OUT,
		GATE_B_OUT,

		TRIG_A_OUT,
		TRIG_B_OUT,
		
		NUM_OUTPUTS
	};
	enum LightIds {
		A_LED,
		B_LED,

		DIR_LED,
		FLP_LED,

		NUM_LIGHTS
	};

	bool gate = false;
	bool flipMode = false;
	
	SchmittTrigger tossTrigger;
	SchmittTrigger flipTrigger;
	SchmittTrigger resetTrigger;
	PulseGenerator gatePulse;
	
	Heads() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

	void reset() override {
		gate = false;
		flipMode = false;
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		
		json_t *gateStateJ = json_boolean(gate);
		json_object_set_new(rootJ, "gateState", gateStateJ);

		return rootJ;
	}
	
	void fromJson(json_t *rootJ) override {
		json_t *gateStateJ = json_object_get(rootJ, "gateState");
		if (gateStateJ)
			gate = json_boolean_value(gateStateJ);
	}
};

void Heads::step() {
	flipMode = params[TOSS_MODE].value;

	if (tossTrigger.process(inputs[TOSS_IN].value + params[TOSS_UI].value)) {
		gatePulse.trigger(0.01f);
		if (flipMode)
			gate = (randomUniform() < params[PROB_UI].value + inputs[PROB_CV].value) ? !gate : gate;
		else
			gate = (randomUniform() < (params[PROB_UI].value + inputs[PROB_CV].value*0.1f));
	}

	if (flipTrigger.process(inputs[FLIP_IN].value + params[FLIP_UI].value)) {
		gatePulse.trigger(0.01f);
		gate = !gate;
	}

	if (resetTrigger.process(inputs[SET_A_IN].value + params[SET_A_UI].value)) {
		gatePulse.trigger(0.01f);
		gate = false;
	}

	if (resetTrigger.process(inputs[SET_B_IN].value + params[SET_B_UI].value)) {
		gatePulse.trigger(0.01f);
		gate = true;
	}

	outputs[SIG_OUT].value = !gate ? inputs[SIG_A_IN].value : inputs[SIG_B_IN].value;
	outputs[GATE_A_OUT].value = !gate * 10.0f;
	outputs[TRIG_A_OUT].value = !gate * gatePulse.process(1.0/engineGetSampleRate()) * 10.0f;
	outputs[GATE_B_OUT].value = gate * 10.0f;
	outputs[TRIG_B_OUT].value = gate * gatePulse.process(1.0/engineGetSampleRate()) * 10.0f;
	lights[A_LED].value = !gate;
	lights[B_LED].value = gate;
	lights[DIR_LED].value = !flipMode;
	lights[FLP_LED].value = flipMode;
}

struct ModeToggle : SVGSwitch, ToggleSwitch {
	ModeToggle() {
		addFrame(SVG::load(assetPlugin(plugin, "res/components/empty.svg")));
		box.size = Vec(12,6);
	}
};

struct HeadsWidget : ModuleWidget {
	HeadsWidget(Heads *module);
};

HeadsWidget::HeadsWidget(Heads *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/panels/Heads.svg")));
  
	// screws
	addChild(Widget::create<ScrewHead1>(Vec(15, 0)));
	addChild(Widget::create<ScrewHead2>(Vec(box.size.x - 15, 0)));
	addChild(Widget::create<ScrewHead3>(Vec(30, 365)));
	// addChild(Widget::create<ScrewHead4>(Vec(box.size.x - 15, 365)));

	addInput(Port::create<InPortAud>(Vec(4,22), Port::INPUT, module, Heads::SIG_A_IN));
	addInput(Port::create<InPortAud>(Vec(34,22), Port::INPUT, module, Heads::SIG_B_IN));
	  
	addParam(ParamWidget::create<PvCKnob>(Vec(19,64),module, Heads::PROB_UI, 0.0f, 1.0f, 0.5f));
	addInput(Port::create<InPortCtrl>(Vec(19,88), Port::INPUT, module, Heads::PROB_CV));
	addInput(Port::create<InPortBin>(Vec(19,124), Port::INPUT, module, Heads::TOSS_IN));
	addParam(ParamWidget::create<LabelButtonL>(Vec(12,149), module, Heads::TOSS_UI, 0, 1, 0));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(25,165),module, Heads::DIR_LED));
	addChild(ModuleLightWidget::create<FourPixLight<BlueLED>>(Vec(31,165),module, Heads::FLP_LED));
	addParam(ParamWidget::create<ModeToggle>(Vec(24,164), module, Heads::TOSS_MODE, 0, 1, 0));
	addInput(Port::create<InPortBin>(Vec(19,180), Port::INPUT, module, Heads::FLIP_IN));
	addParam(ParamWidget::create<LabelButtonL>(Vec(12,205), module, Heads::FLIP_UI, 0, 1, 0));
	
	addInput(Port::create<InPortBin>(Vec(4,222), Port::INPUT, module, Heads::SET_A_IN));
	addParam(ParamWidget::create<LabelButtonS>(Vec(3,247), module, Heads::SET_A_UI, 0, 1, 0));
	addInput(Port::create<InPortBin>(Vec(34,222), Port::INPUT, module, Heads::SET_B_IN));
	addParam(ParamWidget::create<LabelButtonS>(Vec(33,247), module, Heads::SET_B_UI, 0, 1, 0));

	addOutput(Port::create<OutPortVal>(Vec(19,276), Port::OUTPUT, module, Heads::SIG_OUT));

	addChild(ModuleLightWidget::create<FourPixLight<CyanLED>>(Vec(13,267),module, Heads::A_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,312), Port::OUTPUT, module, Heads::GATE_A_OUT));
	addOutput(Port::create<OutPortBin>(Vec(4,336), Port::OUTPUT, module, Heads::TRIG_A_OUT));

	addChild(ModuleLightWidget::create<FourPixLight<PurpleLED>>(Vec(43,267),module, Heads::B_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,312), Port::OUTPUT, module, Heads::GATE_B_OUT));
	addOutput(Port::create<OutPortBin>(Vec(34,336), Port::OUTPUT, module, Heads::TRIG_B_OUT));
}

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;

RACK_PLUGIN_MODEL_INIT(PvC, Heads) {
   Model *modelHeads = Model::create<Heads, HeadsWidget>(
      "PvC", "Heads", "Heads", LOGIC_TAG, SWITCH_TAG, RANDOM_TAG);
   return modelHeads;
}
