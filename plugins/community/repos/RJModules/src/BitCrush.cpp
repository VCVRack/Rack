#include "RJModules.hpp"
#include <iostream>
#include <cmath>

namespace rack_plugin_RJModules {

// Thanks to http://10rem.net/blog/2013/01/13/a-simple-bitcrusher-and-sample-rate-reducer-in-cplusplus-for-a-windows-store-app
struct BitCrush: Module {
	enum ParamIds {
		CH1_PARAM,
		CH2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CH1_INPUT,
		CH1_CV_INPUT,
		CH2_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CH1_OUTPUT,
		NUM_OUTPUTS
	};

	BitCrush() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};


#define ROUND(f) ((float)((f > 0.0) ? floor(f + 0.5) : ceil(f - 0.5)))

void BitCrush::step() {
	float ch1 = inputs[CH1_INPUT].value;

	float combined_input = params[CH1_PARAM].value * clamp(inputs[CH1_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
	float combined_crush_floor = params[CH2_PARAM].value * clamp(inputs[CH2_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

	// new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
	float mapped_crush_floor = ((combined_crush_floor - 0.0) / (1.0 - 0.0) ) * (32.0 - 1.0) + 1.0;
	int crush_floor = 32 - static_cast<int>(mapped_crush_floor) + 1;

	float mapped_input = ((combined_input - 0.0) / (1.0 - 0.0) ) * (crush_floor - 1.0) + 1.0;
	int bit_depth = crush_floor - static_cast<int>(mapped_input) + 1;

	int max = pow(2, bit_depth) - 1;
	float ch1_crushed = ROUND((ch1 + 1.0) * max) / max - 1.0;

	outputs[CH1_OUTPUT].value = ch1_crushed;
}


struct BitCrushWidget: ModuleWidget {
	BitCrushWidget(BitCrush *module);
};

BitCrushWidget::BitCrushWidget(BitCrush *module) : ModuleWidget(module) {
	box.size = Vec(15*10, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/BitCrush.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(57, 139), module, BitCrush::CH1_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(57, 219), module, BitCrush::CH2_PARAM, 0.0, 1.0, 0.0));

	addInput(Port::create<PJ301MPort>(Vec(22, 129), Port::INPUT, module, BitCrush::CH1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(22, 160), Port::INPUT, module, BitCrush::CH1_CV_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(22, 241), Port::INPUT, module, BitCrush::CH2_CV_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(110, 145), Port::OUTPUT, module, BitCrush::CH1_OUTPUT));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, BitCrush) {
   Model *modelBitCrush = Model::create<BitCrush, BitCrushWidget>("RJModules", "BitCrush", "[FX] BitCrush", DISTORTION_TAG);
   return modelBitCrush;
}
