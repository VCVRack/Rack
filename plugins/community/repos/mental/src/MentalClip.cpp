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
struct MentalClip : Module {
  enum ParamIds {
    THRESH1_PARAM, GAIN1_PARAM,
    THRESH2_PARAM, GAIN2_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    INPUT1, THRESH1_CV_INPUT, GAIN1_CV_INPUT,
    INPUT2, THRESH2_CV_INPUT, GAIN2_CV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    OUTPUT1,
    OUTPUT2,
    NUM_OUTPUTS
  };

  MentalClip() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
  void step() override;
};

/////////////////////////////////////////////////////
void MentalClip::step() {

  float signal_in1 = inputs[INPUT1].value;
  float threshold1 = params[THRESH1_PARAM].value * 6 + inputs[THRESH1_CV_INPUT].value/2;
  float gain1 = params[GAIN1_PARAM].value * 5 + inputs[GAIN1_CV_INPUT].value / 2;

  float signal_in2 = inputs[INPUT2].value;
  float threshold2 = params[THRESH2_PARAM].value * 6 + inputs[THRESH2_CV_INPUT].value/2;
  float gain2 = params[GAIN2_PARAM].value * 5 + inputs[GAIN2_CV_INPUT].value / 2;


  float clipped1 = signal_in1;
  float clipped2 = signal_in2;

  if (std::abs(signal_in1) > threshold1)
  {
    if (signal_in1 > 0)
    {
     clipped1 = threshold1;
    } else
    {
     clipped1 = - threshold1;
    }
  }

  if (std::abs(signal_in2) > threshold2)
  {
    if (signal_in2 > 0)
    {
     clipped2 = threshold2;
    } else
    {
     clipped2 = - threshold2;
    }
  }
  outputs[OUTPUT1].value = clipped1 * gain1;
  outputs[OUTPUT2].value = clipped2 * gain2;
}

//////////////////////////////////////////////////////////////////
struct MentalClipWidget : ModuleWidget {
  MentalClipWidget(MentalClip *module);
};

MentalClipWidget::MentalClipWidget(MentalClip *module) : ModuleWidget(module)
{
  
  setPanel(SVG::load(assetPlugin(plugin, "res/MentalClip.svg")));

  // label
  addParam(ParamWidget::create<SmlKnob>(Vec(6, box.size.y / 2 - 169), module, MentalClip::THRESH1_PARAM, 0.0, 1.0, 1.0));
  addInput(Port::create<CVInPort>(Vec(3, box.size.y / 2 - 148), Port::INPUT, module, MentalClip::THRESH1_CV_INPUT));
  // label
  addParam(ParamWidget::create<SmlKnob>(Vec(6, box.size.y / 2 - 112), module, MentalClip::GAIN1_PARAM, 0.0, 1.0, 0.5));
  addInput(Port::create<CVInPort>(Vec(3, box.size.y / 2 - 91), Port::INPUT, module, MentalClip::GAIN1_CV_INPUT));
  // output  
  addInput(Port::create<InPort>(Vec(3, box.size.y / 2 - 55), Port::INPUT, module, MentalClip::INPUT1));
  addOutput(Port::create<OutPort>(Vec(3, box.size.y / 2 - 28), Port::OUTPUT, module, MentalClip::OUTPUT1));

  
  // label
  addParam(ParamWidget::create<SmlKnob>(Vec(6, box.size.y - 175), module, MentalClip::THRESH2_PARAM, 0.0, 1.0, 1.0));
  addInput(Port::create<CVInPort>(Vec(3, box.size.y - 154), Port::INPUT, module, MentalClip::THRESH2_CV_INPUT));
  // label
  addParam(ParamWidget::create<SmlKnob>(Vec(6, box.size.y - 122), module, MentalClip::GAIN2_PARAM, 0.0, 1.0, 0.5));
  addInput(Port::create<CVInPort>(Vec(3, box.size.y - 101), Port::INPUT, module, MentalClip::GAIN2_CV_INPUT));
  // output  
  addInput(Port::create<InPort>(Vec(3, box.size.y - 65), Port::INPUT, module, MentalClip::INPUT2));
  addOutput(Port::create<OutPort>(Vec(3, box.size.y - 38), Port::OUTPUT, module, MentalClip::OUTPUT2));

}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalClip) {
   Model *modelMentalClip = Model::create<MentalClip, MentalClipWidget>("mental", "MentalClip", "Clipper", DISTORTION_TAG);
   return modelMentalClip;
}
