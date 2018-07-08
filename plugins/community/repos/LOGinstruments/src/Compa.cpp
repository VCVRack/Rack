#include <string.h>
#include <math.h>
#include <float.h>
#include "LOGinstruments.hpp"

namespace rack_plugin_LOGinstruments {

struct Compa : Module {
	enum ParamIds {
		NUM_PARAMS,
	};
	enum InputIds {
		INPUTA1,
		INPUTB1,
		INPUTA2,
		INPUTB2,
		NUM_INPUTS,
	};
	enum OutputIds {
		OUTPUT1,
		OUTPUT2,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		LIGHT_1,
		LIGHT_2,
		NUM_LIGHTS,
	};

	Compa() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

	void reset() override {
		;
	}
};

void Compa::step() {

	for (int o = 0; o < NUM_OUTPUTS; o++) {
		if (inputs[o*2].active && inputs[o*2+1].active) {
			outputs[o].value = lights[o].value = inputs[o*2].value >= inputs[o*2+1].value ? 1.0 : 0.0;
		}
	}
}

#define BOTTOM_VDIST 175
struct CompaWidget : ModuleWidget {
	CompaWidget(Compa *module);
};

CompaWidget::CompaWidget(Compa *module) : ModuleWidget(module) {


	box.size = Vec(15*6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/compa_nofonts.svg")));
		addChild(panel);
	}

	addInput(Port::create<PJ301MPort>(Vec(50, 58), Port::INPUT, module, Compa::INPUTA1));
	addInput(Port::create<PJ301MPort>(Vec(50, 88), Port::INPUT, module, Compa::INPUTB1));
	addInput(Port::create<PJ301MPort>(Vec(50, 58+BOTTOM_VDIST), Port::INPUT, module, Compa::INPUTA2));
	addInput(Port::create<PJ301MPort>(Vec(50, 88+BOTTOM_VDIST), Port::INPUT, module, Compa::INPUTB2));

	addOutput(Port::create<PJ3410Port>(Vec(46, 118), Port::OUTPUT, module, Compa::OUTPUT1));
	addOutput(Port::create<PJ3410Port>(Vec(46, 118+BOTTOM_VDIST), Port::OUTPUT, module, Compa::OUTPUT2));

	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(35, 130), module, Compa::LIGHT_1));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(35, 130+BOTTOM_VDIST), module, Compa::LIGHT_2));


}

} // namespace rack_plugin_LOGinstruments

using namespace rack_plugin_LOGinstruments;

RACK_PLUGIN_MODEL_INIT(LOGinstruments, Compa) {
   Model *modelCompa = Model::create<Compa, CompaWidget>("LOGinstruments", "Compa", "Comparator", DIGITAL_TAG, QUANTIZER_TAG);
   return modelCompa;
}

