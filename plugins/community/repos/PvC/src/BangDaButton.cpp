/*

BangDaButton

a cv sending button to trigger/switch things.

button provides gate/trig outs for press and release events as well
as two flipflops.
it also opens/closes 4 A/B switches 2 x 1to2 and 2 x 2to1
a "hidden" gate port can be used to control the button with external sources.

TODO:
	toggle/momentary modes for the channels
	toggles for the flipflops

*/////////////////////////////////////////////////////////////////////////////



#include "pvc.hpp"
#include "dsp/digital.hpp" // SchmittTrigger // PulseGenerator

namespace rack_plugin_PvC {

struct BangDaButton : Module {
	
	enum ParamIds {
		DA_BUTTON,

		NUM_PARAMS
	};
	
	enum InputIds {
		UP_SW_A_IN,
		UP_SW_B_IN,
		UP_CH1_IN,
		UP_MUX_IN,
		UP_CH2_IN,

		DOWN_SW_A_IN,
		DOWN_SW_B_IN,
		DOWN_CH1_IN,
		DOWN_MUX_IN,
		DOWN_CH2_IN,

		DA_BUTTON_TRIG,

		NUM_INPUTS
	};
	
	enum OutputIds {
		UP_SW_OUT,
		UP_CH1_OUT,
		UP_MUX_A_OUT, UP_MUX_B_OUT,
		UP_CH2_OUT,

		UP_FLIP_OUT, UP_TRIG_OUT, UP_GATE_OUT,
		DOWN_GATE_OUT, DOWN_TRIG_OUT, DOWN_FLIP_OUT,
	
		DOWN_CH1_OUT,
		DOWN_MUX_A_OUT,	DOWN_MUX_B_OUT,
		DOWN_CH2_OUT,
		DOWN_SW_OUT,
		
		NUM_OUTPUTS
	};

	enum LightIds {
		UP_LED,
		DOWN_LED,
		
		NUM_LIGHTS
	};

	bool pressed = false;
	
	bool flipUp = true;
	bool flipDn = false;

	SchmittTrigger upFlipTrg;
	SchmittTrigger dnFlipTrg;

	PulseGenerator upPulse;
	PulseGenerator dnPulse;

	BangDaButton() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

	void reset() override {
		pressed = false;
		flipUp = true;
		flipDn = false;		
	}
};

void BangDaButton::step() {
	pressed = params[DA_BUTTON].value;
	if (inputs[DA_BUTTON_TRIG].active) {
		pressed = inputs[DA_BUTTON_TRIG].value > 0 ? !pressed : pressed;
	}

	if (dnFlipTrg.process(pressed)) {	// HOLD
		flipDn = !flipDn;
		dnPulse.trigger(0.01);
	}
	if (upFlipTrg.process(!pressed)){ // RELEASE
		flipUp = !flipUp;
		upPulse.trigger(0.01);
	}

	
	// outputs[UP_CH1_OUT].value = pressed ? inputs[UP_CH1_IN].normalize(0.0) : 0.0f;
	// outputs[UP_CH2_OUT].value = pressed ? 0.0f : inputs[UP_CH2_IN].normalize(0.0);
	outputs[UP_SW_OUT].value = pressed ? inputs[UP_SW_A_IN].normalize(0.0f) : inputs[UP_SW_B_IN].normalize(0.0f);
	outputs[UP_MUX_A_OUT].value = pressed * inputs[UP_MUX_IN].normalize(0.0f);
	outputs[UP_MUX_B_OUT].value = !pressed * inputs[UP_MUX_IN].normalize(0.0f);
	
	// outputs[DOWN_CH1_OUT].value = pressed ? inputs[DOWN_CH1_IN].normalize(0.0f) : 0.0f;
	// outputs[DOWN_CH2_OUT].value = pressed ? 0.0f : inputs[DOWN_CH2_IN].normalize(0.0f);
	outputs[DOWN_SW_OUT].value = pressed ? inputs[DOWN_SW_B_IN].normalize(0.0f) : inputs[DOWN_SW_A_IN].normalize(0.0f);
	outputs[DOWN_MUX_A_OUT].value = !pressed * inputs[DOWN_MUX_IN].normalize(0.0f);
	outputs[DOWN_MUX_B_OUT].value = pressed * inputs[DOWN_MUX_IN].normalize(0.0f);

	outputs[UP_GATE_OUT].value = !pressed * 10.0f;
	outputs[DOWN_GATE_OUT].value = pressed * 10.0f;

	outputs[UP_FLIP_OUT].value = flipUp * 10.0f;
	outputs[DOWN_FLIP_OUT].value = flipDn * 10.0f;
	
	outputs[UP_TRIG_OUT].value = upPulse.process(1.0/engineGetSampleRate()) * 10.0f;
	outputs[DOWN_TRIG_OUT].value = dnPulse.process(1.0/engineGetSampleRate()) * 10.0f;
	
	lights[DOWN_LED].value = pressed;
	lights[UP_LED].value = !pressed;
	
}


struct DaButton : SVGSwitch, MomentarySwitch {
	DaButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/components/DaButton_up.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/components/DaButton_dn.svg")));
		box.size = Vec(82,82);
	}
};
struct InPortT : SVGPort {
	InPortT() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/empty.svg"));
		background->wrap();
		box.size = Vec(22,22);
	}
};

struct BangDaButtonWidget : ModuleWidget {
	BangDaButtonWidget(BangDaButton *module);
};

BangDaButtonWidget::BangDaButtonWidget(BangDaButton *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/panels/BangDaButton.svg")));
	
	// screws
	addChild(Widget::create<ScrewHead3>(Vec(15, 0)));
	addChild(Widget::create<ScrewHead2>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewHead4>(Vec(15, 365)));
	addChild(Widget::create<ScrewHead1>(Vec(box.size.x - 30, 365)));

	addInput(Port::create<InPortAud>(Vec(4,22), Port::INPUT, module,BangDaButton::UP_SW_A_IN));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(28,30.5),module,BangDaButton::DOWN_LED));
	addOutput(Port::create<OutPortVal>(Vec(34,22), Port::OUTPUT, module,BangDaButton::UP_SW_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(57,30.5),module,BangDaButton::UP_LED));
	addInput(Port::create<InPortAud>(Vec(64,22), Port::INPUT, module,BangDaButton::UP_SW_B_IN));

	// addInput(Port::create<InPortAud>(Vec(4,44), Port::INPUT, module,BangDaButton::UP_CH1_IN));
	// addChild(ModuleLightWidget::create<FourPixLight<RedLight>>(Vec(27,52),module,BangDaButton::UP_LED));
	// addOutput(Port::create<OutPortVal>(Vec(34,44), Port::OUTPUT, module,BangDaButton::UP_CH1_OUT));
	
	addOutput(Port::create<OutPortVal>(Vec(4,68), Port::OUTPUT, module,BangDaButton::UP_MUX_A_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(27,76.5),module,BangDaButton::DOWN_LED));
	addInput(Port::create<InPortAud>(Vec(34,68), Port::INPUT, module,BangDaButton::UP_MUX_IN));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(57,76.5),module,BangDaButton::UP_LED));
	addOutput(Port::create<OutPortVal>(Vec(64,68), Port::OUTPUT, module,BangDaButton::UP_MUX_B_OUT));
	
	// addInput(Port::create<InPortAud>(Vec(34,92), Port::INPUT, module,BangDaButton::UP_CH2_IN));
	// addChild(ModuleLightWidget::create<FourPixLight<RedLight>>(Vec(57,100),module,BangDaButton::DOWN_LED));
	// addOutput(Port::create<OutPortVal>(Vec(64,92), Port::OUTPUT, module,BangDaButton::UP_CH2_OUT));
	
	addOutput(Port::create<OutPortBin>(Vec(6,124), Port::OUTPUT, module,BangDaButton::UP_GATE_OUT));
	addOutput(Port::create<OutPortBin>(Vec(34,124), Port::OUTPUT, module,BangDaButton::UP_TRIG_OUT));
	addOutput(Port::create<OutPortBin>(Vec(62,124), Port::OUTPUT, module,BangDaButton::UP_FLIP_OUT));

	addParam(ParamWidget::create<DaButton>(Vec(4,149),module,BangDaButton::DA_BUTTON, 0, 1, 0));
	addInput(Port::create<InPortT>(Vec(4,179), Port::INPUT, module,BangDaButton::DA_BUTTON_TRIG));
	
	addOutput(Port::create<OutPortBin>(Vec(6,234), Port::OUTPUT, module,BangDaButton::DOWN_FLIP_OUT));
	addOutput(Port::create<OutPortBin>(Vec(34,234), Port::OUTPUT, module,BangDaButton::DOWN_TRIG_OUT));
	addOutput(Port::create<OutPortBin>(Vec(62,234), Port::OUTPUT, module,BangDaButton::DOWN_GATE_OUT));
	
	// addInput(Port::create<InPortAud>(Vec(4,266), Port::INPUT, module,BangDaButton::DOWN_CH1_IN));
	// addChild(ModuleLightWidget::create<FourPixLight<RedLight>>(Vec(27,274),module,BangDaButton::UP_LED));
	// addOutput(Port::create<OutPortVal>(Vec(34,266), Port::OUTPUT, module,BangDaButton::DOWN_CH1_OUT));
	
	addOutput(Port::create<OutPortVal>(Vec(4,290), Port::OUTPUT, module,BangDaButton::DOWN_MUX_A_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(27,298.5),module,BangDaButton::UP_LED));
	addInput(Port::create<InPortAud>(Vec(34,290), Port::INPUT, module,BangDaButton::DOWN_MUX_IN));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(57,298.5),module,BangDaButton::DOWN_LED));
	addOutput(Port::create<OutPortVal>(Vec(64,290), Port::OUTPUT, module,BangDaButton::DOWN_MUX_B_OUT));
	
	// addInput(Port::create<InPortAud>(Vec(34,314), Port::INPUT, module,BangDaButton::DOWN_CH2_IN));
	// addChild(ModuleLightWidget::create<FourPixLight<RedLight>>(Vec(57,322),module,BangDaButton::DOWN_LED));
	// addOutput(Port::create<OutPortVal>(Vec(64,314), Port::OUTPUT, module,BangDaButton::DOWN_CH2_OUT));
	
	addInput(Port::create<InPortAud>(Vec(4,336), Port::INPUT, module,BangDaButton::DOWN_SW_A_IN));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(27,344.5),module,BangDaButton::UP_LED));
	addOutput(Port::create<OutPortVal>(Vec(34,336), Port::OUTPUT, module,BangDaButton::DOWN_SW_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(57,344.5),module,BangDaButton::DOWN_LED));
	addInput(Port::create<InPortAud>(Vec(64,336), Port::INPUT, module,BangDaButton::DOWN_SW_B_IN));

}

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;

RACK_PLUGIN_MODEL_INIT(PvC, BangDaButton) {
   Model *modelBangDaButton = Model::create<BangDaButton, BangDaButtonWidget>(
      "PvC", "BangDaButton", "BangDaButton", CONTROLLER_TAG, SWITCH_TAG);
   return modelBangDaButton;
}
