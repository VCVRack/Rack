#include "arjo_modules.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_arjo_modules {

struct Count : Module {
	enum ParamIds {
		COUNT_PARAM_1,
		COUNT_PARAM_2,
		COUNT_PARAM_3,
		COUNT_PARAM_4,
		NUM_PARAMS
	};
	enum InputIds {
		CLK_INPUT,
		RST_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT_1,
		LIGHT_2,
		LIGHT_3,
		LIGHT_4,
		NUM_LIGHTS
	};

	SchmittTrigger clock_trigger;
	SchmittTrigger reset_trigger;
	PulseGenerator pulse_1;
	PulseGenerator pulse_2;
	PulseGenerator pulse_3;
	PulseGenerator pulse_4;

	int count_1 = 0;
	int count_2 = 0;
	int count_3 = 0;
	int count_4 = 0;

	int max_count_1 = 0;
	int max_count_2 = 0;
	int max_count_3 = 0;
	int max_count_4 = 0;

	Count() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

};


void Count::step() {
	max_count_1 = round(params[COUNT_PARAM_1].value);
	max_count_2 = round(params[COUNT_PARAM_2].value);
	max_count_3 = round(params[COUNT_PARAM_3].value);
	max_count_4 = round(params[COUNT_PARAM_4].value);


	if (reset_trigger.process(inputs[RST_INPUT].value)) {
		count_1 = max_count_1;
		count_2 = max_count_2;
		count_3 = max_count_3;
		count_4 = max_count_4;
	}

	if (clock_trigger.process(inputs[CLK_INPUT].value)) {
		count_1++;
		count_2++;
		count_3++;
		count_4++;
	}

	if (count_1 > max_count_1) {count_1 = 0; pulse_1.trigger(1e-3);}
	if (count_2 > max_count_2) {count_2 = 0; pulse_2.trigger(1e-3);}
	if (count_3 > max_count_3) {count_3 = 0; pulse_3.trigger(1e-3);}
	if (count_4 > max_count_4) {count_4 = 0; pulse_4.trigger(1e-3);}

	outputs[OUTPUT_1].value = pulse_1.process(1.0 / engineGetSampleRate()) ? 10.0f : 0.0f;
	outputs[OUTPUT_2].value = pulse_2.process(1.0 / engineGetSampleRate()) ? 10.0f : 0.0f;
	outputs[OUTPUT_3].value = pulse_3.process(1.0 / engineGetSampleRate()) ? 10.0f : 0.0f;
	outputs[OUTPUT_4].value = pulse_4.process(1.0 / engineGetSampleRate()) ? 10.0f : 0.0f;

}

struct CountWidget : ModuleWidget {
	CountWidget(Count *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/count.svg")));

		// Screws
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		// Inputs
		addInput(Port::create<small_port>(Vec(4.5, 50.5), Port::INPUT, module, Count::CLK_INPUT));
		addInput(Port::create<small_port>(Vec(23.5, 50.5), Port::INPUT, module, Count::RST_INPUT));

		// Knobs
		addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(10.5, 79), module, Count::COUNT_PARAM_1, 0.0, 15.0, 0.0));
		addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(10.5, 149), module, Count::COUNT_PARAM_2, 0.0, 15.0, 0.0));
		addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(10.5, 219), module, Count::COUNT_PARAM_3, 0.0, 15.0, 0.0));
		addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(10.5, 289), module, Count::COUNT_PARAM_4, 0.0, 15.0, 0.0));

		// Outputs
		addOutput(Port::create<PJ301MPort>(Vec(10.5, 111), Port::OUTPUT, module, Count::OUTPUT_1));
		addOutput(Port::create<PJ301MPort>(Vec(10.5, 180), Port::OUTPUT, module, Count::OUTPUT_2));
		addOutput(Port::create<PJ301MPort>(Vec(10.5, 251), Port::OUTPUT, module, Count::OUTPUT_3));
		addOutput(Port::create<PJ301MPort>(Vec(10.5, 321), Port::OUTPUT, module, Count::OUTPUT_4));

	}
};

} // namespace rack_plugin_arjo_modules

using namespace rack_plugin_arjo_modules;

RACK_PLUGIN_MODEL_INIT(arjo_modules, Count) {
   Model *modelCount = Model::create<Count, CountWidget>("arjo_modules", "count", "count", UTILITY_TAG);
   return modelCount;
}
