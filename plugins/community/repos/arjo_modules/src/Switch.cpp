#include "arjo_modules.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_arjo_modules {

struct Switch : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		CLK_INPUT,
		RST_INPUT,
		ENUMS(VALUE_INPUT, 8),
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

	Switch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void Switch::step() {
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

	outputs[VALUE_OUTPUT].value = inputs[VALUE_INPUT + current_input].value;
}


struct SwitchWidget : ModuleWidget {
	SwitchWidget(Switch *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/switch.svg")));

		// Screws
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		// Inputs
		addInput(Port::create<small_port>(Vec(4.5, 50.5), Port::INPUT, module, Switch::CLK_INPUT));
		addInput(Port::create<small_port>(Vec(23.5, 50.5), Port::INPUT, module, Switch::RST_INPUT));

		static const float portY[8] = {83-4, 113-4, 142-4, 172-4, 202-4, 232-4, 262-4, 292-4};

		for (int i = 0; i < 8; i++) {
			addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(31.5, portY[i]), module, Switch::LIGHTS + i));
			addInput(Port::create<PJ301MPort>(Vec(10.5, portY[i]), Port::INPUT, module, Switch::VALUE_INPUT + i));
		}

		// Output
		addOutput(Port::create<PJ301MPort>(Vec(10.5, 321.5), Port::OUTPUT, module, Switch::VALUE_OUTPUT));
	}
};

} // namespace rack_plugin_arjo_modules

using namespace rack_plugin_arjo_modules;

RACK_PLUGIN_MODEL_INIT(arjo_modules, Switch) {
   Model *modelSwitch = Model::create<Switch, SwitchWidget>("arjo_modules", "switch", "switch", UTILITY_TAG);
   return modelSwitch;
}
