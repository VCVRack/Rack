#include "mental.hpp"

namespace rack_plugin_mental {

struct MentalMults : Module {
	enum ParamIds {
		NUM_PARAMS
	};
  
	enum InputIds {		
		INPUT_1,
    INPUT_2,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
    OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
    OUTPUT_2_1,
    OUTPUT_2_2,
		OUTPUT_2_3,
		OUTPUT_2_4,
		OUTPUT_2_5,
		NUM_OUTPUTS
	};

	MentalMults() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};

void MentalMults::step() {
	
  
  float signal_in_1 = inputs[INPUT_1].value;
  
  outputs[OUTPUT_1].value = signal_in_1;
	outputs[OUTPUT_2].value = signal_in_1;
	outputs[OUTPUT_3].value = signal_in_1;
	outputs[OUTPUT_4].value = signal_in_1;
  outputs[OUTPUT_5].value = signal_in_1;
  
  float signal_in_2 = inputs[INPUT_2].value;
  
  outputs[OUTPUT_2_1].value = signal_in_2;
	outputs[OUTPUT_2_2].value = signal_in_2;
	outputs[OUTPUT_2_3].value = signal_in_2;
	outputs[OUTPUT_2_4].value = signal_in_2;
  outputs[OUTPUT_2_5].value = signal_in_2;
}

/////////////////////////////////////////////////////////////////////////////////////////
struct MentalMultsWidget : ModuleWidget {
	MentalMultsWidget(MentalMults *module);
};

MentalMultsWidget::MentalMultsWidget(MentalMults *module) : ModuleWidget(module)
{

	setPanel(SVG::load(assetPlugin(plugin, "res/MentalMults.svg")));
	
	addInput(Port::create<InPort>(Vec(3, 22), Port::INPUT, module, MentalMults::INPUT_1));
  addInput(Port::create<InPort>(Vec(3, 190), Port::INPUT, module, MentalMults::INPUT_2));
  
  addOutput(Port::create<OutPort>(Vec(3, 58), Port::OUTPUT, module, MentalMults::OUTPUT_1));
	addOutput(Port::create<OutPort>(Vec(3, 83), Port::OUTPUT, module, MentalMults::OUTPUT_2));
	addOutput(Port::create<OutPort>(Vec(3, 108), Port::OUTPUT, module, MentalMults::OUTPUT_3));
	addOutput(Port::create<OutPort>(Vec(3, 133), Port::OUTPUT, module, MentalMults::OUTPUT_4));
	addOutput(Port::create<OutPort>(Vec(3, 158), Port::OUTPUT, module, MentalMults::OUTPUT_5));
  
  addOutput(Port::create<OutPort>(Vec(3, 230), Port::OUTPUT, module, MentalMults::OUTPUT_2_1));
	addOutput(Port::create<OutPort>(Vec(3, 255), Port::OUTPUT, module, MentalMults::OUTPUT_2_2));
	addOutput(Port::create<OutPort>(Vec(3, 280), Port::OUTPUT, module, MentalMults::OUTPUT_2_3));
	addOutput(Port::create<OutPort>(Vec(3, 305), Port::OUTPUT, module, MentalMults::OUTPUT_2_4));
	addOutput(Port::create<OutPort>(Vec(3, 330), Port::OUTPUT, module, MentalMults::OUTPUT_2_5));
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalMults) {
   Model *modelMentalMults = Model::create<MentalMults, MentalMultsWidget>("mental", "MentalMults", "Mults", DUAL_TAG, MULTIPLE_TAG);
   return modelMentalMults;
}
