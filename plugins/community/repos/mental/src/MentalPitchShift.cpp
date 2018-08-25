///////////////////////////////////////////////////
//
//   Pitch Shifter VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "mental.hpp"

namespace rack_plugin_mental {

//////////////////////////////////////////////////////
struct MentalPitchShift : Module {
	enum ParamIds {
    OCTAVE_SHIFT_1,
    OCTAVE_SHIFT_2,
    SEMITONE_SHIFT_1,
    SEMITONE_SHIFT_2,    
    NUM_PARAMS
	};
	enum InputIds {
		OCTAVE_SHIFT_1_INPUT,
    OCTAVE_SHIFT_2_INPUT,
    SEMITONE_SHIFT_1_INPUT,
    SEMITONE_SHIFT_2_INPUT, 
    OCTAVE_SHIFT_1_CVINPUT,
    OCTAVE_SHIFT_2_CVINPUT,
    SEMITONE_SHIFT_1_CVINPUT,
    SEMITONE_SHIFT_2_CVINPUT, 
    NUM_INPUTS
	};
	enum OutputIds {
		OCTAVE_SHIFT_1_OUTPUT,
    OCTAVE_SHIFT_2_OUTPUT,
    SEMITONE_SHIFT_1_OUTPUT,
    SEMITONE_SHIFT_2_OUTPUT, 
    NUM_OUTPUTS
	};

	MentalPitchShift() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
  
  float octave_1_out = 0.0;
  float octave_2_out = 0.0;
  float semitone_1_out = 0.0;
  float semitone_2_out = 0.0;  
  
	void step() override;
};

/////////////////////////////////////////////////////
void MentalPitchShift::step() {

  octave_1_out = inputs[OCTAVE_SHIFT_1_INPUT].value + round(params[OCTAVE_SHIFT_1].value) + round(inputs[OCTAVE_SHIFT_1_CVINPUT].value/2);
  octave_2_out = inputs[OCTAVE_SHIFT_2_INPUT].value + round(params[OCTAVE_SHIFT_2].value) + round(inputs[OCTAVE_SHIFT_1_CVINPUT].value/2);
  semitone_1_out = inputs[SEMITONE_SHIFT_1_INPUT].value + round(params[SEMITONE_SHIFT_1].value)*(1.0/12.0) + round(inputs[SEMITONE_SHIFT_1_CVINPUT].value/2)*(1.0/12.0);
  semitone_2_out = inputs[SEMITONE_SHIFT_2_INPUT].value + round(params[SEMITONE_SHIFT_2].value)*(1.0/12.0) + round(inputs[SEMITONE_SHIFT_2_CVINPUT].value/2)*(1.0/12.0);
    
  outputs[OCTAVE_SHIFT_1_OUTPUT].value = octave_1_out;
  outputs[OCTAVE_SHIFT_2_OUTPUT].value = octave_2_out;
  outputs[SEMITONE_SHIFT_1_OUTPUT].value = semitone_1_out;
  outputs[SEMITONE_SHIFT_2_OUTPUT].value = semitone_2_out;

}

//////////////////////////////////////////////////////////////////
struct MentalPitchShiftWidget : ModuleWidget {
  MentalPitchShiftWidget(MentalPitchShift *module);
};

MentalPitchShiftWidget::MentalPitchShiftWidget(MentalPitchShift *module) : ModuleWidget(module)
{

  setPanel(SVG::load(assetPlugin(plugin, "res/MentalPitchShift.svg")));

  addParam(ParamWidget::create<MedKnob>(Vec(2, 20), module, MentalPitchShift::OCTAVE_SHIFT_1, -4.5, 4.5, 0.0));
  addParam(ParamWidget::create<MedKnob>(Vec(2, 80), module, MentalPitchShift::OCTAVE_SHIFT_2, -4.5, 4.5, 0.0));
  addParam(ParamWidget::create<MedKnob>(Vec(2, 140), module, MentalPitchShift::SEMITONE_SHIFT_1, -6.5, 6.5, 0.0));
  addParam(ParamWidget::create<MedKnob>(Vec(2, 200), module, MentalPitchShift::SEMITONE_SHIFT_2, -6.5, 6.5, 0.0));

  addInput(Port::create<CVInPort>(Vec(3, 50), Port::INPUT, module, MentalPitchShift::OCTAVE_SHIFT_1_INPUT));
	addInput(Port::create<CVInPort>(Vec(3, 110), Port::INPUT, module, MentalPitchShift::OCTAVE_SHIFT_2_INPUT));
  addInput(Port::create<CVInPort>(Vec(3, 170), Port::INPUT, module, MentalPitchShift::SEMITONE_SHIFT_1_INPUT));
	addInput(Port::create<CVInPort>(Vec(3, 230), Port::INPUT, module, MentalPitchShift::SEMITONE_SHIFT_2_INPUT));
  
  addInput(Port::create<CVInPort>(Vec(33, 20), Port::INPUT, module, MentalPitchShift::OCTAVE_SHIFT_1_CVINPUT));
	addInput(Port::create<CVInPort>(Vec(33, 80), Port::INPUT, module, MentalPitchShift::OCTAVE_SHIFT_2_CVINPUT));
  addInput(Port::create<CVInPort>(Vec(33, 140), Port::INPUT, module, MentalPitchShift::SEMITONE_SHIFT_1_CVINPUT));
	addInput(Port::create<CVInPort>(Vec(33, 200), Port::INPUT, module, MentalPitchShift::SEMITONE_SHIFT_2_CVINPUT));

  addOutput(Port::create<CVOutPort>(Vec(33, 50), Port::OUTPUT, module, MentalPitchShift::OCTAVE_SHIFT_1_OUTPUT));
  addOutput(Port::create<CVOutPort>(Vec(33, 110), Port::OUTPUT, module, MentalPitchShift::OCTAVE_SHIFT_2_OUTPUT));
  addOutput(Port::create<CVOutPort>(Vec(33, 170), Port::OUTPUT, module, MentalPitchShift::SEMITONE_SHIFT_1_OUTPUT));
  addOutput(Port::create<CVOutPort>(Vec(33, 230), Port::OUTPUT, module, MentalPitchShift::SEMITONE_SHIFT_2_OUTPUT));

}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalPitchShift) {
   Model *modelMentalPitchShift = Model::create<MentalPitchShift, MentalPitchShiftWidget>("mental", "MentalPitchShift", "Pitch Shifter", CONTROLLER_TAG);
   return modelMentalPitchShift;
}
