///////////////////////////////////////////////////
//
//   Chord Creator VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "mental.hpp"

namespace rack_plugin_mental {

/////////////////////////////////////////////////
struct MentalChord : Module {
	enum ParamIds {
      OFFSET_PARAM,
      INVERSION_PARAM,
      VOICING_PARAM,
      NUM_PARAMS
	};

	enum InputIds {
      INPUT,
      OFFSET_CV_INPUT,
      INVERSION_CV_INPUT,
      VOICING_CV_INPUT,
      FLAT_3RD_INPUT,
      FLAT_5TH_INPUT,
      FLAT_7TH_INPUT,
      SUS_2_INPUT,
      SUS_4_INPUT,
      SIX_FOR_5_INPUT,
      ONE_FOR_7_INPUT,
      FLAT_9_INPUT,
      SHARP_9_INPUT,
      SIX_FOR_7_INPUT,
      SHARP_5_INPUT,      
      NUM_INPUTS
	};
	enum OutputIds {
      OUTPUT_1,
      OUTPUT_2,
      OUTPUT_3,
      OUTPUT_4,
      OUTPUT_ROOT,
      OUTPUT_THIRD,
      OUTPUT_FIFTH,
      OUTPUT_SEVENTH,
      NUM_OUTPUTS
	};
  
	MentalChord() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};


/////////////////////////////////////////////////////
void MentalChord::step() {

  float in = inputs[INPUT].value;  
  int octave = round(in);
  
  float offset_raw = (params[OFFSET_PARAM].value) * 12 - 6 + (inputs[OFFSET_CV_INPUT].value) / 1.5;
  float pitch_offset = round(offset_raw) / 12;
  
  float root = in - 1.0*octave + pitch_offset;
  float root_or_2nd = root;
  
  float inversion_raw = (params[INVERSION_PARAM].value) * 4 - 1 + (inputs[INVERSION_CV_INPUT].value / 3);
  int inversion = round(inversion_raw);
  if (inversion > 2) inversion = 2;
  if (inversion < -1) inversion = -1;
  
  float voicing_raw = (params[VOICING_PARAM].value) * 5 - 2 + (inputs[VOICING_CV_INPUT].value / 3);
  int voicing = round(voicing_raw);
  if (voicing > 2) voicing = 2;
  if (voicing < -2) voicing = -2;
  
  
  float voice_1 = 0.0;
  float voice_2 = 0.0;
  float voice_3 = 0.0;
  float voice_4 = 0.0;
  
  int third = 4;
  int fifth = 7;
  int seventh = 11;
    
  if (inputs[FLAT_3RD_INPUT].value > 0.0) third = 3;
  if (inputs[FLAT_5TH_INPUT].value > 0.0) fifth = 6;
  if (inputs[SHARP_5_INPUT].value > 0.0) fifth = 8;
  if (inputs[FLAT_7TH_INPUT].value > 0.0) seventh = 10;
  
  if (inputs[SUS_2_INPUT].value > 0.0) root_or_2nd = root + (2 * (1.0/12.0));
  if (inputs[SUS_4_INPUT].value > 0.0) third = 5;
  if (inputs[SIX_FOR_5_INPUT].value > 0.0) fifth = 9;
  if (inputs[SIX_FOR_7_INPUT].value > 0.0) seventh = 9;
  
  
  if (inputs[FLAT_9_INPUT].value > 0.0) root_or_2nd = root + 1.0/12.0;
  if (inputs[SHARP_9_INPUT].value > 0.0) root_or_2nd = root + (3 * (1.0/12.0));
  if (inputs[ONE_FOR_7_INPUT].value > 0.0) seventh = 12;
  
  outputs[OUTPUT_ROOT].value = root;
  outputs[OUTPUT_THIRD].value = root + third * (1.0/12.0);
  outputs[OUTPUT_FIFTH].value = root + fifth * (1.0/12.0);
  outputs[OUTPUT_SEVENTH].value = root + seventh * (1.0/12.0);
  
  
  
  if (inversion == -1 )
  {
    voice_1 = root_or_2nd;
    voice_2 = root + third * (1.0/12.0);
    voice_3 = root + fifth * (1.0/12.0);
    voice_4 = root + seventh * (1.0/12.0);
  }
  if (inversion == 0 )
  {
    voice_1 = root + third * (1.0/12.0);
    voice_2 = root + fifth * (1.0/12.0);
    voice_3 = root + seventh * (1.0/12.0);
    voice_4 = root_or_2nd + 1.0;
  }
  if (inversion == 1)
  {
    voice_1 = root + fifth * (1.0/12.0);
    voice_2 = root + seventh * (1.0/12.0);
    voice_3 = root_or_2nd + 1.0;
    voice_4 = root + 1.0 + third * (1.0/12.0);
  }
  if (inversion == 2 )
  {
    voice_1 = root + seventh * (1.0/12.0);
    voice_2 = root_or_2nd + 1.0;
    voice_3 = root + 1.0 + third * (1.0/12.0);
    voice_4 = root + 1.0 + fifth * (1.0/12.0);
  }
  
  if (voicing == -1) voice_2 -= 1.0;
  if (voicing == -0) voice_3 -= 1.0;
  if (voicing == 1)
  {
    voice_2 -= 1.0;
    voice_4 -= 1.0;
  }
  if (voicing == 2)
  {
    voice_2 += 1.0;
    voice_4 += 1.0;
  }
  
  
  outputs[OUTPUT_1].value = voice_1;
  outputs[OUTPUT_2].value = voice_2;
  outputs[OUTPUT_3].value = voice_3;
  outputs[OUTPUT_4].value = voice_4;  
 
}

//////////////////////////////////////////////////////////////////
struct MentalChordWidget : ModuleWidget {
  MentalChordWidget(MentalChord *module);
};

MentalChordWidget::MentalChordWidget(MentalChord *module) : ModuleWidget(module)
{

setPanel(SVG::load(assetPlugin(plugin, "res/MentalChord.svg")));

  addParam(ParamWidget::create<MedKnob>(Vec(3, 20), module, MentalChord::OFFSET_PARAM, 0.0, 1.0, 0.5));
  addInput(Port::create<CVInPort>(Vec(3, 50), Port::INPUT, module, MentalChord::OFFSET_CV_INPUT));
  addParam(ParamWidget::create<MedKnob>(Vec(33, 20), module, MentalChord::INVERSION_PARAM, 0.0, 1.0, 0.0));
  addParam(ParamWidget::create<MedKnob>(Vec(63, 20), module, MentalChord::VOICING_PARAM, 0.0, 1.0, 0.0));
	
  addInput(Port::create<CVInPort>(Vec(3, 100), Port::INPUT, module, MentalChord::INPUT));
  addInput(Port::create<CVInPort>(Vec(33, 50), Port::INPUT, module, MentalChord::INVERSION_CV_INPUT));
  addInput(Port::create<CVInPort>(Vec(63, 50), Port::INPUT, module, MentalChord::VOICING_CV_INPUT));
  
  addInput(Port::create<GateInPort>(Vec(3, 130), Port::INPUT, module, MentalChord::FLAT_3RD_INPUT));
  addInput(Port::create<GateInPort>(Vec(3, 155), Port::INPUT, module, MentalChord::FLAT_5TH_INPUT));
  addInput(Port::create<GateInPort>(Vec(3, 180), Port::INPUT, module, MentalChord::FLAT_7TH_INPUT));
  
  addInput(Port::create<GateInPort>(Vec(3, 210), Port::INPUT, module, MentalChord::SUS_2_INPUT));
  addInput(Port::create<GateInPort>(Vec(3, 235), Port::INPUT, module, MentalChord::SUS_4_INPUT));
  addInput(Port::create<GateInPort>(Vec(3, 260), Port::INPUT, module, MentalChord::SIX_FOR_5_INPUT));
  addInput(Port::create<GateInPort>(Vec(3, 285), Port::INPUT, module, MentalChord::ONE_FOR_7_INPUT));
  addInput(Port::create<GateInPort>(Vec(3, 310), Port::INPUT, module, MentalChord::FLAT_9_INPUT));
  addInput(Port::create<GateInPort>(Vec(3, 335), Port::INPUT, module, MentalChord::SHARP_9_INPUT));
  addInput(Port::create<GateInPort>(Vec(33, 316), Port::INPUT, module, MentalChord::SIX_FOR_7_INPUT));
  addInput(Port::create<GateInPort>(Vec(33, 341), Port::INPUT, module, MentalChord::SHARP_5_INPUT));
  
  
  addOutput(Port::create<CVOutPort>(Vec(63, 100), Port::OUTPUT, module, MentalChord::OUTPUT_ROOT));
  addOutput(Port::create<CVOutPort>(Vec(63, 125), Port::OUTPUT, module, MentalChord::OUTPUT_THIRD));
  addOutput(Port::create<CVOutPort>(Vec(63, 150), Port::OUTPUT, module, MentalChord::OUTPUT_FIFTH));
  addOutput(Port::create<CVOutPort>(Vec(63, 175), Port::OUTPUT, module, MentalChord::OUTPUT_SEVENTH));  
    
  addOutput(Port::create<CVOutPort>(Vec(63, 250), Port::OUTPUT, module, MentalChord::OUTPUT_1));
  addOutput(Port::create<CVOutPort>(Vec(63, 275), Port::OUTPUT, module, MentalChord::OUTPUT_2));
  addOutput(Port::create<CVOutPort>(Vec(63, 300), Port::OUTPUT, module, MentalChord::OUTPUT_3));
  addOutput(Port::create<CVOutPort>(Vec(63, 325), Port::OUTPUT, module, MentalChord::OUTPUT_4));
    
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalChord) {
   Model *modelMentalChord = Model::create<MentalChord, MentalChordWidget>("mental", "MentalChord", "Chord", CONTROLLER_TAG);
   return modelMentalChord;
}
