#include "arjo_modules.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_arjo_modules {

struct Seq : Module {
	enum ParamIds {
		ENUMS(VALUE_PARAM, 8),
		NUM_PARAMS
	};
	enum InputIds {
		CLK_INPUT,
		RST_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		VALUE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(LIGHTS, 8),
		NUM_LIGHTS
	};


	SchmittTrigger clock_trigger;
	SchmittTrigger reset_trigger;

	int current_input = 0;
	int max_input = 7;

	Seq() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

};


void Seq::step() {
	if (reset_trigger.process(inputs[RST_INPUT].value)) {
		current_input = 0;
	}

	if (clock_trigger.process(inputs[CLK_INPUT].value)) {
		current_input++;
	}

	if (current_input > max_input) {
		current_input = 0;
	}

	for (int i=0; i < 8; i++) {
		lights[LIGHTS + i].setBrightnessSmooth(0);
	}

	lights[LIGHTS + current_input].setBrightnessSmooth(1);

	outputs[VALUE_OUTPUT].value = params[VALUE_PARAM + current_input].value;
}


struct SeqWidget : ModuleWidget {
	SeqWidget(Seq *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/seq.svg")));

		// Screws
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	

		// Inputs
		addInput(Port::create<small_port>(Vec(4.5, 50.5), Port::INPUT, module, Seq::CLK_INPUT));
		addInput(Port::create<small_port>(Vec(23.5, 50.5), Port::INPUT, module, Seq::RST_INPUT));


		// Knobs
		static const float portY[8] = {83-4, 113-4, 142-4, 172-4, 202-4, 232-4, 262-4, 292-4};

		for (int i = 0; i < 8; i++) {
			addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(31.5, portY[i]), module, Seq::LIGHTS + i));
			addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(10.5, portY[i]), module, Seq::VALUE_PARAM + i, 0.0f, 10.0f, 0.0f));
		}

		// Output
		addOutput(Port::create<PJ301MPort>(Vec(10.5, 321.5), Port::OUTPUT, module, Seq::VALUE_OUTPUT));
	}
};

} // namespace rack_plugin_arjo_modules

using namespace rack_plugin_arjo_modules;

RACK_PLUGIN_MODEL_INIT(arjo_modules, Seq) {
   Model *modelSeq = Model::create<Seq, SeqWidget>("arjo_modules", "seq", "seq", SEQUENCER_TAG);
   return modelSeq;
}
