#include "dBiz.hpp"

namespace rack_plugin_dBiz {

struct Multiple : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		A_INPUT,
		B_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		A1_OUTPUT,
		A2_OUTPUT,
		A3_OUTPUT,
		B1_OUTPUT,
		B2_OUTPUT,
		B3_OUTPUT,
		NUM_OUTPUTS
	};

	Multiple()  : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
	}
	void step() override;
};

void Multiple::step() 
{
	float in1 = inputs[A_INPUT].normalize(0.0);
	float in2 = inputs[B_INPUT].normalize(0.0);
	
	outputs[A1_OUTPUT].value = in1;
	outputs[A2_OUTPUT].value = in1;
	outputs[A3_OUTPUT].value = in1;
	outputs[B1_OUTPUT].value = in2;
	outputs[B2_OUTPUT].value = in2;
	outputs[B3_OUTPUT].value = in2;


}

struct MultipleWidget : ModuleWidget 
{
MultipleWidget(Multiple *module) : ModuleWidget(module)
{
	box.size = Vec(15 * 2, 380);

	{
		Panel *panel = new LightPanel();
		panel->backgroundImage = Image::load("plugins/dBiz/res/Multiple.png");
		panel->box.size = box.size;
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15,   0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

	addInput (Port::create<PJ301MIPort>(Vec(3,  18), Port::INPUT, module, Multiple::A_INPUT));
	addOutput(Port::create<PJ301MOPort>(Vec(3,  58), Port::OUTPUT, module, Multiple::A1_OUTPUT));
	addOutput(Port::create<PJ301MOPort>(Vec(3,  98), Port::OUTPUT, module, Multiple::A2_OUTPUT));
	addOutput(Port::create<PJ301MOPort>(Vec(3, 138), Port::OUTPUT, module, Multiple::A3_OUTPUT));
	addInput (Port::create<PJ301MIPort>(Vec(3, 178), Port::INPUT, module, Multiple::B_INPUT));
	addOutput(Port::create<PJ301MOPort>(Vec(3, 218), Port::OUTPUT, module, Multiple::B1_OUTPUT));
	addOutput(Port::create<PJ301MOPort>(Vec(3, 258), Port::OUTPUT, module, Multiple::B2_OUTPUT));
	addOutput(Port::create<PJ301MOPort>(Vec(3, 298), Port::OUTPUT, module, Multiple::B3_OUTPUT));
	
}

};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, Multiple) {
   Model *modelMultiple = Model::create<Multiple, MultipleWidget>("dBiz", "Multiple", "Multiple", MULTIPLE_TAG);
   return modelMultiple;
}
