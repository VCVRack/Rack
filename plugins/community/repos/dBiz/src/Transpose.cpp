///////////////////////////////////////////////////
//  dBiz revisited version of 
//
//   Pitch Shifter VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "dBiz.hpp"

namespace rack_plugin_dBiz {

/////added fine out /////////////////////////////////////////////////
struct Transpose : Module {
	enum ParamIds {
    OCTAVE_SHIFT_1,
    OCTAVE_SHIFT_2,
    SEMITONE_SHIFT_1,
    SEMITONE_SHIFT_2,
    FINE_SHIFT_1,
    FINE_SHIFT_2,
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
    FINE_SHIFT_1_INPUT,
    FINE_SHIFT_2_INPUT, 
    FINE_SHIFT_1_CVINPUT,
    FINE_SHIFT_2_CVINPUT,
    NUM_INPUTS
	};
	enum OutputIds {
		OCTAVE_SHIFT_1_OUTPUT,
    OCTAVE_SHIFT_2_OUTPUT,
    SEMITONE_SHIFT_1_OUTPUT,
    SEMITONE_SHIFT_2_OUTPUT,
    FINE_SHIFT_1_OUTPUT,
    FINE_SHIFT_2_OUTPUT,
    NUM_OUTPUTS
	};

	Transpose() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
  
  float octave_1_out = 0.0;
  float octave_2_out = 0.0;
  float semitone_1_out = 0.0;
  float semitone_2_out = 0.0;
  float fine_1_out = 0.0;
  float fine_2_out = 0.0;

  void step() override;
  
};


/////////////////////////////////////////////////////
void Transpose::step() {

  octave_1_out = inputs[OCTAVE_SHIFT_1_INPUT].value + round(params[OCTAVE_SHIFT_1].value) + round(inputs[OCTAVE_SHIFT_1_CVINPUT].value/2);
  octave_2_out = inputs[OCTAVE_SHIFT_2_INPUT].value + round(params[OCTAVE_SHIFT_2].value) + round(inputs[OCTAVE_SHIFT_1_CVINPUT].value/2);
  semitone_1_out = inputs[SEMITONE_SHIFT_1_INPUT].value + round(params[SEMITONE_SHIFT_1].value)*(1.0/12.0) + round(inputs[SEMITONE_SHIFT_1_CVINPUT].value/2)*(1.0/12.0);
  semitone_2_out = inputs[SEMITONE_SHIFT_2_INPUT].value + round(params[SEMITONE_SHIFT_2].value)*(1.0/12.0) + round(inputs[SEMITONE_SHIFT_2_CVINPUT].value/2)*(1.0/12.0);
  fine_1_out = inputs[FINE_SHIFT_1_INPUT].value + (params[FINE_SHIFT_1].value)*(1.0/12.0) + (inputs[FINE_SHIFT_1_CVINPUT].value/2)*(1.0/2.0);
  fine_2_out = inputs[FINE_SHIFT_2_INPUT].value + (params[FINE_SHIFT_2].value)*(1.0/12.0) + (inputs[FINE_SHIFT_2_CVINPUT].value/2)*(1.0/2.0);

  outputs[OCTAVE_SHIFT_1_OUTPUT].value = octave_1_out;
  outputs[OCTAVE_SHIFT_2_OUTPUT].value = octave_2_out;
  outputs[SEMITONE_SHIFT_1_OUTPUT].value = semitone_1_out;
  outputs[SEMITONE_SHIFT_2_OUTPUT].value = semitone_2_out;
  outputs[FINE_SHIFT_1_OUTPUT].value = fine_1_out;
  outputs[FINE_SHIFT_2_OUTPUT].value = fine_2_out;

}

//////////////////////////////////////////////////////////////////
struct TransposeWidget : ModuleWidget 
{
TransposeWidget(Transpose *module) : ModuleWidget(module)
{
	box.size = Vec(15*4, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin,"res/Transpose.svg")));
		addChild(panel);
	}

//Screw

	  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	  addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	
//

    addParam(ParamWidget::create<FlatASnap>(Vec(4, 15), module, Transpose::OCTAVE_SHIFT_1, -4.5, 4.5, 0.0));
    addParam(ParamWidget::create<FlatASnap>(Vec(4, 75), module, Transpose::OCTAVE_SHIFT_2, -4.5, 4.5, 0.0));
    addParam(ParamWidget::create<FlatASnap>(Vec(4, 135), module, Transpose::SEMITONE_SHIFT_1, -6.5, 6.5, 0.0));
    addParam(ParamWidget::create<FlatASnap>(Vec(4, 195), module, Transpose::SEMITONE_SHIFT_2, -6.5, 6.5, 0.0));
    addParam(ParamWidget::create<FlatA>(Vec(4, 255), module, Transpose::FINE_SHIFT_1, -1, 1, 0.0));
    addParam(ParamWidget::create<FlatA>(Vec(4, 315), module, Transpose::FINE_SHIFT_2, -1, 1, 0.0));

    addInput(Port::create<PJ301MIPort>(Vec(3, 2+45), Port::INPUT, module, Transpose::OCTAVE_SHIFT_1_INPUT));
	  addInput(Port::create<PJ301MIPort>(Vec(3, 2+105), Port::INPUT, module, Transpose::OCTAVE_SHIFT_2_INPUT));
    addInput(Port::create<PJ301MIPort>(Vec(3, 2+165), Port::INPUT, module, Transpose::SEMITONE_SHIFT_1_INPUT));
    addInput(Port::create<PJ301MIPort>(Vec(3, 2+225), Port::INPUT, module, Transpose::SEMITONE_SHIFT_2_INPUT));
    addInput(Port::create<PJ301MIPort>(Vec(3, 2+285), Port::INPUT, module, Transpose::FINE_SHIFT_1_INPUT));
    addInput(Port::create<PJ301MIPort>(Vec(3, 2+345), Port::INPUT, module, Transpose::FINE_SHIFT_2_INPUT));

  
    addInput(Port::create<PJ301MCPort>(Vec(33, 15), Port::INPUT, module, Transpose::OCTAVE_SHIFT_1_CVINPUT));
	  addInput(Port::create<PJ301MCPort>(Vec(33, 75), Port::INPUT, module, Transpose::OCTAVE_SHIFT_2_CVINPUT));
    addInput(Port::create<PJ301MCPort>(Vec(33, 135), Port::INPUT, module, Transpose::SEMITONE_SHIFT_1_CVINPUT));
    addInput(Port::create<PJ301MCPort>(Vec(33, 195), Port::INPUT, module, Transpose::SEMITONE_SHIFT_2_CVINPUT));
    addInput(Port::create<PJ301MCPort>(Vec(33, 255), Port::INPUT, module, Transpose::FINE_SHIFT_1_CVINPUT));
    addInput(Port::create<PJ301MCPort>(Vec(33, 315), Port::INPUT, module, Transpose::FINE_SHIFT_2_CVINPUT));


    addOutput(Port::create<PJ301MOPort>(Vec(33, 2+45), Port::OUTPUT, module, Transpose::OCTAVE_SHIFT_1_OUTPUT));
    addOutput(Port::create<PJ301MOPort>(Vec(33, 2+105), Port::OUTPUT, module, Transpose::OCTAVE_SHIFT_2_OUTPUT));
    addOutput(Port::create<PJ301MOPort>(Vec(33, 2+165), Port::OUTPUT, module, Transpose::SEMITONE_SHIFT_1_OUTPUT));
    addOutput(Port::create<PJ301MOPort>(Vec(33, 2+225), Port::OUTPUT, module, Transpose::SEMITONE_SHIFT_2_OUTPUT));
    addOutput(Port::create<PJ301MOPort>(Vec(33, 2+285), Port::OUTPUT, module, Transpose::FINE_SHIFT_1_OUTPUT));
    addOutput(Port::create<PJ301MOPort>(Vec(33, 2+345), Port::OUTPUT, module, Transpose::FINE_SHIFT_2_OUTPUT));

}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, Transpose) {
   Model *modelTranspose = Model::create<Transpose, TransposeWidget>("dBiz", "Transpose", "Transpose", QUANTIZER_TAG);
   return modelTranspose;
}

