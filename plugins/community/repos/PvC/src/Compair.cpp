/*

COMPAIR

Dual Window Comparator (inspired by Joranalogue Compare2)

checks if an input voltage is between two values.
compare window is set by two parameters: position and width.
top knob on each channel (position) sets a center voltage of +/-5V.
the other one changes the width around the center from near zero to 10V.
so with both knobs at center the window is set at [-2.5..2.5].
both parameters are also controllable via cv input.
cv and knob values are added together when computing the window.
whenever the input signal is within that window the GATE output on that
channel will go from 0V to +5V and the NOT output will do the opposite.
logic outputs at the bottom compare the output values of both channels.
AND output goes high if A is high and B is high
OR output goes high if A or B is high
XOR output goes high if either A or B ar high and the other one is low.
FLIP output changes whenever the XOR out goes high.
channel B inputs are normalized to channel A inputs.
channel output to the logic section can be inverted.
all outputs are 0V-5V.

TODO:
  -proper RGB Light...
  -code clean-up, optimization, simplification

*/////////////////////////////////////////////////////////////////////////////

#include "pvc.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_PvC {

struct Compair : Module {
	enum ParamIds {
		POS_A_PARAM,
		WIDTH_A_PARAM,
		POS_B_PARAM,
		WIDTH_B_PARAM,
		INVERT_A_PARAM,
		INVERT_B_PARAM,
	 	NUM_PARAMS
	};

	enum InputIds {
		AUDIO_A_IN,
		POS_A_IN,
		WIDTH_A_IN,
		AUDIO_B_IN,
		POS_B_IN,
		WIDTH_B_IN,
		NUM_INPUTS
	};

	enum OutputIds {
		GATE_A_OUT,
		NOT_A_OUT,
	 	GATE_B_OUT,
		NOT_B_OUT,
		AND_OUT,
		OR_OUT,
		XOR_OUT,
		FLIP_OUT,
	 	NUM_OUTPUTS
	};

	enum LightIds {
		GATE_A_LED,
		GATE_B_LED,
		AND_LED,
		OR_LED,
		XOR_LED,
		FLIP_LED,
		OVER_A_LED,
		BELOW_A_LED,
		OVER_B_LED,
		BELOW_B_LED,
		NUM_LIGHTS
	};

	bool outA = false;
	bool outB = false;
	bool flip = false;
	SchmittTrigger flipTrigger;

	enum OutputMode {
		ORIGINAL,
		BIPOLAR
	//	INV_BIPOLAR,
	//	INV_ORIGINAL
	};
	OutputMode outputMode = ORIGINAL;

	Compair() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		// reset();
	}

	void step() override;
	void reset() override {
		outA = false;
		outB = false;
		flip = false;

		outputMode = ORIGINAL;
	}
	// void randomize() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();

		// outputMode
		json_t *outputModeJ = json_integer((int) outputMode);
		json_object_set_new(rootJ, "outputMode", outputModeJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// outputMode
		json_t *outputModeJ = json_object_get(rootJ, "outputMode");
		if (outputModeJ)
			outputMode = (OutputMode)json_integer_value(outputModeJ);
	}

};

void Compair::step(){
	// get inputs and normalize B to A
	float inputA = inputs[AUDIO_A_IN].value;
	float inputB = inputs[AUDIO_B_IN].normalize(inputA);

	// get knob values
	float posA = params[POS_A_PARAM].value;
	float widthA = params[WIDTH_A_PARAM].value;
	float posB = params[POS_B_PARAM].value;
	float widthB = params[WIDTH_B_PARAM].value;;

	// testing bi-polar outputs
		float highVal = 5.0f;
		//float lowVal = 0.0f;
		float lowVal = outputMode == BIPOLAR ? -5.0f : 0.0f;

	// channel A CV inputs to knob values
	if (inputs[POS_A_IN].active)
		posA = (params[POS_A_PARAM].value + inputs[POS_A_IN].value);
	if (inputs[WIDTH_A_IN].active)
		widthA = (params[WIDTH_A_PARAM].value + inputs[WIDTH_A_IN].value);

	// compute window A
	float upperThreshA = posA + widthA*0.5f;
	float lowerThreshA = posA - widthA*0.5f;

	// check if input A is in window A
	outA = (inputA <= upperThreshA && inputA >= lowerThreshA) ? true : false;

	// channel B CV inputs to knob values and normalization
	if (inputs[POS_B_IN].active || inputs[POS_A_IN].active)
		posB = (params[POS_B_PARAM].value + inputs[POS_B_IN].normalize(inputs[POS_A_IN].value));

	if (inputs[WIDTH_B_IN].active || inputs[WIDTH_B_IN].active)
		widthB = (params[WIDTH_B_PARAM].value + inputs[WIDTH_B_IN].normalize(inputs[WIDTH_A_IN].value));

	// compute window B
	float upperThreshB = posB + widthB*0.5f;
	float lowerThreshB = posB - widthB*0.5f;

	// check if input B is in window B

	outB = (inputB <= upperThreshB && inputB >= lowerThreshB) ? true : false;

	// Gate/Not outputs and lights
	outputs[GATE_A_OUT].value = outA ? highVal : lowVal;
	outputs[NOT_A_OUT].value = !outA ? highVal : lowVal;
	lights[GATE_A_LED].setBrightness( outA ? 0.9f : (0.25f - clamp( fabsf(inputA-posA) * 0.025f, 0.0f, 0.4f) ) );
	lights[OVER_A_LED].setBrightness( (inputA > upperThreshA) ? (inputA - upperThreshA)*0.1f + 0.4f : 0.0f );
	lights[BELOW_A_LED].setBrightness( (inputA < lowerThreshA) ? (lowerThreshA - inputA)*0.1f + 0.4f : 0.0f );

	outputs[GATE_B_OUT].value = outB ? highVal : lowVal;
	outputs[NOT_B_OUT].value = !outB ? highVal : lowVal;
	lights[GATE_B_LED].setBrightness( outB ? 0.9f : (0.25f - clamp( fabsf(inputB-posB) * 0.025f, 0.0f, 0.4f) ) );
	lights[OVER_B_LED].setBrightness( (inputB > upperThreshB) ? (inputB - upperThreshB)*0.1f + 0.4f : 0.0f );
	lights[BELOW_B_LED].setBrightness( (inputB < lowerThreshB) ? (lowerThreshB - inputB)*0.1f + 0.4f : 0.0f );

	// logic input inverts
	if (params[INVERT_A_PARAM].value)
		outA = !outA;
	if (params[INVERT_B_PARAM].value)
		outB = !outB;

	// logic outputs and lights
	outputs[AND_OUT].value = (outA && outB) ? highVal : lowVal;
	lights[AND_LED].setBrightness( (outA && outB) );
	outputs[OR_OUT].value = (outA || outB) ? highVal : lowVal;
	lights[OR_LED].setBrightness( (outA || outB) );
	outputs[XOR_OUT].value = (outA != outB) ? highVal : lowVal;
	lights[XOR_LED].setBrightness( (outA != outB) );
	if (flipTrigger.process(outputs[XOR_OUT].value)) // trigger the FlipFlop
		flip = !flip;
	outputs[FLIP_OUT].value = flip ? highVal : lowVal;
	lights[FLIP_LED].setBrightness( flip );
}
//

struct CompairToggle : SVGSwitch, ToggleSwitch {
	CompairToggle() {
		addFrame(SVG::load(assetPlugin(plugin, "res/components/CompairToggleUp.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/components/CompairToggleDn.svg")));
	}
};
// backdrop for the compare LEDs
struct CompairLightBg : SVGScrew {
	CompairLightBg() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/CompairLightBg.svg"));
		sw->wrap();
		box.size = Vec(22,22);
	}
};
// LEDs
template <typename BASE>
 struct CompairLight : BASE {
 	CompairLight() {
		this->box.size = Vec(22, 22);
 		this->bgColor = nvgRGBA(0x00, 0x00, 0x00, 0x00);
 	}
 };


struct CompairWidget : ModuleWidget {
	CompairWidget(Compair *module);
	Menu *createContextMenu() override;
};

CompairWidget::CompairWidget(Compair *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/panels/Compair.svg")));
	
	// SCREWS
	addChild(Widget::create<ScrewHead1>(Vec(15, 0)));
	//addChild(Widget::create<ScrewHead2>(Vec(box.size.x - 15, 0)));
	addChild(Widget::create<ScrewHead3>(Vec(15, 365)));
	//addChild(Widget::create<ScrewHead4>(Vec(box.size.x - 30, 365)));

	// A Side
	addInput(Port::create<InPortAud>(Vec(4,22), Port::INPUT, module,Compair::AUDIO_A_IN));
	addInput(Port::create<InPortAud>(Vec(34,22), Port::INPUT, module,Compair::AUDIO_B_IN));

	addParam(ParamWidget::create<PvCKnob>(Vec(4,64), module, Compair::POS_A_PARAM, -5.0f , 5.0f, 0.0f));
	addInput(Port::create<InPortCtrl>(Vec(4,88), Port::INPUT, module,Compair::POS_A_IN));
	addParam(ParamWidget::create<PvCKnob>(Vec(34,64), module, Compair::POS_B_PARAM, -5.0f, 5.0f, 0.0f));
	addInput(Port::create<InPortCtrl>(Vec(34,88), Port::INPUT, module,Compair::POS_B_IN));

	addParam(ParamWidget::create<PvCKnob>(Vec(34,128), module, Compair::WIDTH_B_PARAM, 0.01f , 10.001f, 5.0f));
	addInput(Port::create<InPortCtrl>(Vec(34,152), Port::INPUT, module,Compair::WIDTH_B_IN));
	addParam(ParamWidget::create<PvCKnob>(Vec(4,128), module, Compair::WIDTH_A_PARAM, 0.01f , 10.001f, 5.0f));
	addInput(Port::create<InPortCtrl>(Vec(4,152), Port::INPUT, module,Compair::WIDTH_A_IN));

	addChild(Widget::create<CompairLightBg>(Vec(4, 190)));
	addChild(ModuleLightWidget::create<CompairLight<BlueLED>>(Vec(4,190),module,Compair::BELOW_A_LED));
	addChild(ModuleLightWidget::create<CompairLight<WhiteLED>>(Vec(4,190),module,Compair::GATE_A_LED));
	addChild(ModuleLightWidget::create<CompairLight<RedLED>>(Vec(4,190),module,Compair::OVER_A_LED));
	addParam(ParamWidget::create<CompairToggle>(Vec(4,190),module,Compair::INVERT_A_PARAM, 0, 1, 0));
	addChild(Widget::create<CompairLightBg>(Vec(34, 190)));
	addChild(ModuleLightWidget::create<CompairLight<BlueLED>>(Vec(34,190),module,Compair::BELOW_B_LED));
	addChild(ModuleLightWidget::create<CompairLight<WhiteLED>>(Vec(34,190),module,Compair::GATE_B_LED));
	addChild(ModuleLightWidget::create<CompairLight<RedLED>>(Vec(34,190),module,Compair::OVER_B_LED));
	addParam(ParamWidget::create<CompairToggle>(Vec(34,190),module,Compair::INVERT_B_PARAM, 0, 1, 0));

	addOutput(Port::create<OutPortBin>(Vec(4,230), Port::OUTPUT, module,Compair::GATE_A_OUT));
	addOutput(Port::create<OutPortBin>(Vec(4,254), Port::OUTPUT, module,Compair::NOT_A_OUT));
	addOutput(Port::create<OutPortBin>(Vec(34,230), Port::OUTPUT, module,Compair::GATE_B_OUT));
	addOutput(Port::create<OutPortBin>(Vec(34,254), Port::OUTPUT, module,Compair::NOT_B_OUT));

	addChild(ModuleLightWidget::create<FourPixLight<CyanLED>>(Vec(13,288),module,Compair::AND_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,294), Port::OUTPUT, module,Compair::AND_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(43,288),module,Compair::OR_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,294), Port::OUTPUT, module,Compair::OR_OUT));

	addChild(ModuleLightWidget::create<FourPixLight<YellowLED>>(Vec(13,330),module,Compair::XOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(4,336), Port::OUTPUT, module,Compair::XOR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(43,330),module,Compair::FLIP_LED));
	addOutput(Port::create<OutPortBin>(Vec(34,336), Port::OUTPUT, module,Compair::FLIP_OUT));
}

/*CompairWidget::CompairWidget(){
	Compair *module = new Compair();
	setModule(module);
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/panels/Compair.svg")));
		addChild(panel);
	}
	// SCREWS
	addChild(Widget::create<ScrewHead1>(Vec(15, 0)));
	//addChild(Widget::create<ScrewHead2>(Vec(box.size.x - 15, 0)));
	addChild(Widget::create<ScrewHead3>(Vec(15, 365)));
	addChild(Widget::create<ScrewHead4>(Vec(box.size.x - 30, 365)));

	// A
	addInput(Port::create<InPortAud>(Vec(35,234), Port::INPUT, module,Compair::AUDIO_A_IN));
	addParam(ParamWidget::create<PvCKnob>(Vec(10,60), module, Compair::POS_A_PARAM, -5.0f , 5.0f, 0.0f));
	addInput(Port::create<InPortCtrl>(Vec(7,190), Port::INPUT, module,Compair::POS_A_IN));
	addParam(ParamWidget::create<PvCKnob>(Vec(10,128), module, Compair::WIDTH_A_PARAM, 0.01f , 10.001f, 5.0f));
	addInput(Port::create<InPortCtrl>(Vec(35,190), Port::INPUT, module,Compair::WIDTH_A_IN));
	addOutput(Port::create<OutPortBin>(Vec(7,278), Port::OUTPUT, module,Compair::GATE_A_OUT));
	addChild(Widget::create<LEDback>(Vec(7, 234)));
	addChild(ModuleLightWidget::create<CompairLight<BlueLED>>(Vec(8,235),module,Compair::BELOW_A_LED));
	addChild(ModuleLightWidget::create<CompairLight<WhiteLED>>(Vec(8,235),module,Compair::GATE_A_LED));
	addChild(ModuleLightWidget::create<CompairLight<RedLED>>(Vec(8,235),module,Compair::OVER_A_LED));
	addOutput(Port::create<OutPortBin>(Vec(35,278), Port::OUTPUT, module,Compair::NOT_A_OUT));

	// B
	addInput(Port::create<InPortAud>(Vec(63,234), Port::INPUT, module,Compair::AUDIO_B_IN));
	addParam(ParamWidget::create<PvCKnob>(Vec(66,60), module, Compair::POS_B_PARAM, -5.0f, 5.0f, 0.0f));
	addInput(Port::create<InPortCtrl>(Vec(90,190), Port::INPUT, module,Compair::POS_B_IN));
	addParam(ParamWidget::create<PvCKnob>(Vec(66,128), module, Compair::WIDTH_B_PARAM, 0.01f , 10.001f, 5.0f));
	addInput(Port::create<InPortCtrl>(Vec(63,190), Port::INPUT, module,Compair::WIDTH_B_IN));
	addOutput(Port::create<OutPortBin>(Vec(90,278), Port::OUTPUT, module,Compair::GATE_B_OUT));
	addChild(Widget::create<LEDback>(Vec(90, 234)));
	addChild(ModuleLightWidget::create<CompairLight<BlueLED>>(Vec(91,235),module,Compair::BELOW_B_LED));
	addChild(ModuleLightWidget::create<CompairLight<WhiteLED>>(Vec(91,235),module,Compair::GATE_B_LED));
	addChild(ModuleLightWidget::create<CompairLight<RedLED>>(Vec(91,235),module,Compair::OVER_B_LED));
	addOutput(Port::create<OutPortBin>(Vec(63,278), Port::OUTPUT, module,Compair::NOT_B_OUT));
	// Invert toggles
	addParam(ParamWidget::create<CompairToggle>(Vec(11,238),module,Compair::INVERT_A_PARAM, 0, 1, 0));
	addParam(ParamWidget::create<CompairToggle>(Vec(94,238),module,Compair::INVERT_B_PARAM, 0, 1, 0));
	// LOGIC
	addOutput(Port::create<OutPortBin>(Vec(7,324), Port::OUTPUT, module,Compair::AND_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<CyanLED>>(Vec(16,318),module,Compair::AND_LED));
	addOutput(Port::create<OutPortBin>(Vec(35,324), Port::OUTPUT, module,Compair::OR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<OrangeLED>>(Vec(44,318),module,Compair::OR_LED));
	addOutput(Port::create<OutPortBin>(Vec(63,324), Port::OUTPUT, module,Compair::XOR_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<YellowLED>>(Vec(72,318),module,Compair::XOR_LED));
	addOutput(Port::create<OutPortBin>(Vec(90,324), Port::OUTPUT, module,Compair::FLIP_OUT));
	addChild(ModuleLightWidget::create<FourPixLight<GreenLED>>(Vec(99,318),module,Compair::FLIP_LED));
}*/

struct CompairOutputModeItem : MenuItem {
	Compair *compair;
	Compair::OutputMode outputMode;
	void onAction(EventAction &e) override {
		compair->outputMode = outputMode;
	}
	void step() override {
		rightText = (compair->outputMode == outputMode) ? "✔" : "";
	}
};

Menu *CompairWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	Compair *compair = dynamic_cast<Compair*>(module);
	assert(compair);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Output Mode";
	menu->addChild(modeLabel);

	CompairOutputModeItem *originalItem = new CompairOutputModeItem();
	originalItem->text = "Original [0V..+5V]";
	originalItem->compair = compair;
	originalItem->outputMode = Compair::ORIGINAL;
	menu->addChild(originalItem);

	CompairOutputModeItem *bipolarItem = new CompairOutputModeItem();
	bipolarItem->text = "Bi-Polar [-5V..+5V]";
	bipolarItem->compair = compair;
	bipolarItem->outputMode = Compair::BIPOLAR;
	menu->addChild(bipolarItem);

	return menu;
}

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;

RACK_PLUGIN_MODEL_INIT(PvC, Compair) {
   Model *modelCompair = Model::create<Compair, CompairWidget>(
      "PvC", "Compair", "Compair", LOGIC_TAG, DUAL_TAG, CLOCK_MODULATOR_TAG);
   return modelCompair;
}
