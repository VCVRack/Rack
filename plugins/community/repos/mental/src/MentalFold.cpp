///////////////////////////////////////////////////
//
//   Wave Folder VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "mental.hpp"

namespace rack_plugin_mental {

//////////////////////////////////////////////////////
struct MentalFold : Module {
	enum ParamIds {
    THRESH_PARAM,
    GAIN_PARAM,
    THRESH_PARAM2,
    GAIN_PARAM2,
    NUM_PARAMS
	};
	enum InputIds {
		INPUT_1,
    THRESH_CV_INPUT,
    GAIN_CV_INPUT,
    INPUT_2,
    THRESH_CV_INPUT2,
    GAIN_CV_INPUT2,
    NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
    OUTPUT_2,
    NUM_OUTPUTS
	};

	MentalFold() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};

/////////////////////////////////////////////////////
void MentalFold::step() {

  float signal_in_1 = inputs[INPUT_1].value;
  float signal_in_2 = inputs[INPUT_2].value;
  
  float threshold_fold = params[THRESH_PARAM].value * 6 + inputs[THRESH_CV_INPUT].value;
  float threshold_fold2 = params[THRESH_PARAM2].value * 6 + inputs[THRESH_CV_INPUT2].value;
  
  float gain = params[GAIN_PARAM].value * 5 + inputs[GAIN_CV_INPUT].value / 2;
  float gain2 = params[GAIN_PARAM2].value * 5 + inputs[GAIN_CV_INPUT2].value / 2;

  float modified = signal_in_1;
  float modified2 = signal_in_2;
  
  if (std::abs(signal_in_1) > threshold_fold )
  {
    if (signal_in_1 > 0)
    {
     modified = threshold_fold - (signal_in_1 - threshold_fold );
    } else
    {
     modified = - threshold_fold - (signal_in_1 + threshold_fold );
    }
  }
  
  if (std::abs(signal_in_2) > threshold_fold2 )
  {
    if (signal_in_2 > 0)
    {
     modified2 = threshold_fold2 - (signal_in_2 - threshold_fold2 );
    } else
    {
     modified2 = - threshold_fold2 - (signal_in_2 + threshold_fold2 );
    }
  }

  outputs[OUTPUT_1].value = modified * gain;
  outputs[OUTPUT_2].value = modified2 * gain2;

}

//////////////////////////////////////////////////////////////////
struct MentalFoldWidget : ModuleWidget {
  MentalFoldWidget(MentalFold *module);
};

MentalFoldWidget::MentalFoldWidget(MentalFold *module) : ModuleWidget(module)
{

  setPanel(SVG::load(assetPlugin(plugin, "res/MentalFold.svg")));
  
  // label
  addParam(ParamWidget::create<SmlKnob>(Vec(6, box.size.y / 2 - 169), module, MentalFold::THRESH_PARAM, 0.0, 1.0, 1.0));
  addInput(Port::create<CVInPort>(Vec(3, box.size.y / 2 - 148), Port::INPUT, module, MentalFold::THRESH_CV_INPUT));
  // label
  addParam(ParamWidget::create<SmlKnob>(Vec(6, box.size.y / 2 - 112), module, MentalFold::GAIN_PARAM, 0.0, 1.0, 0.5));
  addInput(Port::create<CVInPort>(Vec(3, box.size.y / 2 - 91), Port::INPUT, module, MentalFold::GAIN_CV_INPUT));
  // output  
  addInput(Port::create<InPort>(Vec(3, box.size.y / 2 - 55), Port::INPUT, module, MentalFold::INPUT_1));
  addOutput(Port::create<OutPort>(Vec(3, box.size.y / 2 - 28), Port::OUTPUT, module, MentalFold::OUTPUT_1));

  
  // label
  addParam(ParamWidget::create<SmlKnob>(Vec(6, box.size.y - 177), module, MentalFold::THRESH_PARAM2, 0.0, 1.0, 1.0));
  addInput(Port::create<CVInPort>(Vec(3, box.size.y - 156), Port::INPUT, module, MentalFold::THRESH_CV_INPUT2));
  // label
  addParam(ParamWidget::create<SmlKnob>(Vec(6, box.size.y - 120), module, MentalFold::GAIN_PARAM2, 0.0, 1.0, 0.5));
  addInput(Port::create<CVInPort>(Vec(3, box.size.y - 99), Port::INPUT, module, MentalFold::GAIN_CV_INPUT2));
  // output  
  addInput(Port::create<InPort>(Vec(3, box.size.y - 65), Port::INPUT, module, MentalFold::INPUT_2));
  addOutput(Port::create<OutPort>(Vec(3, box.size.y - 38), Port::OUTPUT, module, MentalFold::OUTPUT_2));

}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalFold) {
   Model *modelMentalFold = Model::create<MentalFold, MentalFoldWidget>("mental", "MentalFold", "Wave Folder", DISTORTION_TAG);
   return modelMentalFold;
}
