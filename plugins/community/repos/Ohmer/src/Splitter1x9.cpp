////////////////////////////////////////////////////////////////////////////////////////////////////
////// Splitter 1x9                                                                              ///
////// 2 HP module, having 1 input sent "splitted" to 9 outputs, but limited voltages must stay  ///
////// in -11.7V/+11.7V bounds to every output ("hard clipping").                                ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

namespace rack_plugin_Ohmer {

struct Splitter1x9Module : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		MAIN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
		OUTPUT_6,
		OUTPUT_7,
		OUTPUT_8,
		OUTPUT_9,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_CLIP,
		NUM_LIGHTS
	};

	// Counter used for red LED afterglow (used together with "ledClipAfterglow" boolean flag).
	unsigned long ledClipDelay = 0; // long type is required for highest engine samplerates!
	// This flag controls red LED afterglow (active or not).
	bool ledClipAfterglow = false;

	Splitter1x9Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void Splitter1x9Module::step() {
	// Consider an incoming voltage on input port...
	//
	float raw_input_voltage = inputs[MAIN_INPUT].value;
	float splitted_out_voltage = clamp(raw_input_voltage, -11.7f, 11.7f); // These -11.7V/+11.7V limits are max. possible voltage on Eurorack.
	if (!ledClipAfterglow && (raw_input_voltage != splitted_out_voltage)) {
		// Different is meaning... the voltage was clipped: turn on the LED (reset its afterglow counter).
		ledClipDelay = 0;
		ledClipAfterglow = true;
	}
	for (int i=OUTPUT_1; i<NUM_OUTPUTS; i++) {
		// ...then transmit the same voltage to all output ports, but "clip" the voltage first if it's out of bounds if necessary!
		outputs[i].value = splitted_out_voltage;
	}
	// Afterglow for red LED.
	if (ledClipAfterglow && (ledClipDelay < round(engineGetSampleRate() / 3)))
		ledClipDelay++;
		else {
			ledClipAfterglow = false;
			ledClipDelay = 0;
		}
	// Lit or unlit LED (depending "ledClipAfterglow" flag).
	lights[LED_CLIP].value = ledClipAfterglow ? 1.0 : 0.0f;
}

struct Splitter1x9Widget : ModuleWidget {
	Splitter1x9Widget(Splitter1x9Module *module);
};

Splitter1x9Widget::Splitter1x9Widget(Splitter1x9Module *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Splitter1x9.svg")));
	// Using four screws configuration (even for 2 HP module), due to mechanical constraints on connectors, and by the way, on this small plate :-)
	addChild(Widget::create<Torx_Silver>(Vec(0, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<Torx_Silver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	// Input port.
	addInput(Port::create<PJ301M_In>(Vec(2.5, 22), Port::INPUT, module, Splitter1x9Module::MAIN_INPUT));
	// Output ports.
	addOutput(Port::create<PJ301M_Out>(Vec(2.5, 70), Port::OUTPUT, module, Splitter1x9Module::OUTPUT_1));
	addOutput(Port::create<PJ301M_Out>(Vec(2.5, 100), Port::OUTPUT, module, Splitter1x9Module::OUTPUT_2));
	addOutput(Port::create<PJ301M_Out>(Vec(2.5, 130), Port::OUTPUT, module, Splitter1x9Module::OUTPUT_3));
	addOutput(Port::create<PJ301M_Out>(Vec(2.5, 160), Port::OUTPUT, module, Splitter1x9Module::OUTPUT_4));
	addOutput(Port::create<PJ301M_Out>(Vec(2.5, 190), Port::OUTPUT, module, Splitter1x9Module::OUTPUT_5));
	addOutput(Port::create<PJ301M_Out>(Vec(2.5, 220), Port::OUTPUT, module, Splitter1x9Module::OUTPUT_6));
	addOutput(Port::create<PJ301M_Out>(Vec(2.5, 250), Port::OUTPUT, module, Splitter1x9Module::OUTPUT_7));
	addOutput(Port::create<PJ301M_Out>(Vec(2.5, 280), Port::OUTPUT, module, Splitter1x9Module::OUTPUT_8));
	addOutput(Port::create<PJ301M_Out>(Vec(3, 310), Port::OUTPUT, module, Splitter1x9Module::OUTPUT_9));
	// Clipping red LED.
	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(18, 47), module, Splitter1x9Module::LED_CLIP));
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, Splitter1x9) {
   Model *modelSplitter1x9 = Model::create<Splitter1x9Module, Splitter1x9Widget>("Ohmer Modules", "SplitterModule", "Splitter 1x9", MULTIPLE_TAG, UTILITY_TAG);
   return modelSplitter1x9;
}
