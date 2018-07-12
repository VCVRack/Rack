///////////////////////////////////////////////////
//  dBiz revisited version of 
//
//   Chord Creator VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "dBiz.hpp"

namespace rack_plugin_dBiz {

/////////////////////////////////////////////////
struct Chord : Module {
	enum ParamIds {
      OFFSET_PARAM,
      INVERSION_PARAM,
      VOICING_PARAM,
      OFFSET_AMT_PARAM,
      INVERSION_AMT_PARAM,
      VOICING_AMT_PARAM,
      FLAT_3RD_PARAM,
      FLAT_5TH_PARAM,
      FLAT_7TH_PARAM,
      SUS_2_PARAM,
      SUS_4_PARAM,
      SIX_FOR_5_PARAM,
      ONE_FOR_7_PARAM,
      FLAT_9_PARAM,
      SHARP_9_PARAM,
      SIX_FOR_7_PARAM,
      SHARP_5_PARAM,
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

  enum LighIds {
      FLAT_3RD_LIGHT,
      FLAT_5TH_LIGHT,
      FLAT_7TH_LIGHT,
      SUS_2_LIGHT,
      SUS_4_LIGHT,
      SIX_FOR_5_LIGHT,
      ONE_FOR_7_LIGHT,
      FLAT_9_LIGHT,
      SHARP_9_LIGHT,
      SIX_FOR_7_LIGHT,
      SHARP_5_LIGHT,
      OUT_1_LIGHT,
      OUT_2_LIGHT,
      OUT_3_LIGHT,
      OUT_4_LIGHT,
      NUM_LIGHTS

  };

  
	Chord() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override ;
};


/////////////////////////////////////////////////////
void Chord::step() {

  float in = inputs[INPUT].value;  
  int octave = round(in);
  
  float offset_raw = (params[OFFSET_PARAM].value) * 12 - 6 + (inputs[OFFSET_CV_INPUT].value*params[OFFSET_AMT_PARAM].value) / 1.5;
  float pitch_offset = round(offset_raw) / 12;
  
  float root = in - 1.0*octave + pitch_offset;
  float root_or_2nd = root;
  
  float inversion_raw = (params[INVERSION_PARAM].value) * 4 - 1 + ((inputs[INVERSION_CV_INPUT].value*params[INVERSION_AMT_PARAM].value) / 3);
  int inversion = round(inversion_raw);
  if (inversion > 2) inversion = 2;
  if (inversion < -1) inversion = -1;
  
  float voicing_raw = (params[VOICING_PARAM].value) * 5 - 2 + ((inputs[VOICING_CV_INPUT].value*params[VOICING_AMT_PARAM].value) / 3);
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
    
  if (inputs[FLAT_3RD_INPUT].value+params[FLAT_3RD_PARAM].value > 0.0)
  {
     third = 3;
     lights[FLAT_3RD_LIGHT].value=1.0;
  }
  else lights[FLAT_3RD_LIGHT].value=0.0;

  if (inputs[FLAT_5TH_INPUT].value+params[FLAT_5TH_PARAM].value > 0.0)
  {
     fifth = 6;
     lights[FLAT_5TH_LIGHT].value=1.0;
  }
  else lights[FLAT_5TH_LIGHT].value=0.0;

  if (inputs[SHARP_5_INPUT].value+params[SHARP_5_PARAM].value > 0.0)
  {
     fifth = 8;
     lights[SHARP_5_LIGHT].value=1.0;
  }
  else lights[SHARP_5_LIGHT].value=0.0;
  

  if (inputs[FLAT_7TH_INPUT].value+params[FLAT_7TH_PARAM].value > 0.0)
  {
     seventh = 10;
     lights[FLAT_7TH_LIGHT].value=1.0;
  }
  else lights[FLAT_7TH_LIGHT].value=0.0;

  if (inputs[SUS_2_INPUT].value+params[SUS_2_PARAM].value > 0.0)
  {
     root_or_2nd = root + (2 * (1.0/12.0));
     lights[SUS_2_LIGHT].value=1.0;
  }
  else lights[SUS_2_LIGHT].value=0.0;

  if (inputs[SUS_4_INPUT].value+params[SUS_4_PARAM].value > 0.0)
  {
     third = 5;
     lights[SUS_4_LIGHT].value=1.0;
  }
  else lights[SUS_4_LIGHT].value=0.0;

  if (inputs[SIX_FOR_5_INPUT].value+params[SIX_FOR_5_PARAM].value > 0.0)
  {
     fifth = 9;
     lights[SIX_FOR_5_LIGHT].value=1.0;
  }
  else lights[SIX_FOR_5_LIGHT].value=0.0;

  if (inputs[SIX_FOR_7_INPUT].value+params[SIX_FOR_7_PARAM].value > 0.0)
  {
     seventh = 9;
     lights[SIX_FOR_7_LIGHT].value=1.0;
  }
  else lights[SIX_FOR_7_LIGHT].value=0.0;

  
  if (inputs[FLAT_9_INPUT].value+params[FLAT_9_PARAM].value > 0.0)
  {
     root_or_2nd = root + 1.0/12.0;
     lights[FLAT_9_LIGHT].value=1.0;
  }
  else lights[FLAT_9_LIGHT].value=0.0;

  if (inputs[SHARP_9_INPUT].value+params[SHARP_9_PARAM].value > 0.0)
  {
     root_or_2nd = root + (3 * (1.0/12.0));
     lights[SHARP_9_LIGHT].value=1.0;
  }
  else lights[SHARP_9_LIGHT].value=0.0;

  if (inputs[ONE_FOR_7_INPUT].value+params[ONE_FOR_7_PARAM].value > 0.0)
  {
     seventh = 12;
     lights[ONE_FOR_7_LIGHT].value=1.0;
  }
  else lights[ONE_FOR_7_LIGHT].value=0.0;

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

template <typename BASE>
struct ChordLight : BASE
{
  ChordLight()
  {
    this->box.size = mm2px(Vec(5, 5));
  }
};
//////////////////////////////////////////////////////////////////
struct ChordWidget : ModuleWidget 
{
ChordWidget(Chord *module) : ModuleWidget(module)
{
	box.size = Vec(15*9, 380);

  {
		SVGPanel *panel = new SVGPanel();
    //Panel *panel = new LightPanel();
		panel->box.size = box.size;
		//panel->backgroundImage = Image::load(assetPlugin(plugin, "res/Chord.png"));
    panel->setBackground(SVG::load(assetPlugin(plugin,"res/Chord.svg")));
		addChild(panel);
	}
//
  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
//
int jacks = 27;
int pot=22;
int off =2.5;
int space = 40;

  addInput(Port::create<PJ301MCPort>(Vec(off,60+jacks*1), Port::INPUT, module, Chord::OFFSET_CV_INPUT));
  addInput(Port::create<PJ301MCPort>(Vec(off,60+jacks*2), Port::INPUT, module, Chord::INVERSION_CV_INPUT));
  addInput(Port::create<PJ301MCPort>(Vec(off,60+jacks*3), Port::INPUT, module, Chord::VOICING_CV_INPUT));

  addParam(ParamWidget::create<Trimpot>(Vec(off*2,pot*1), module, Chord::OFFSET_AMT_PARAM, 0.0, 1.0, 0.0));
  addParam(ParamWidget::create<Trimpot>(Vec(off*2,pot*2), module, Chord::INVERSION_AMT_PARAM, 0.0, 1.0, 0.0));
  addParam(ParamWidget::create<Trimpot>(Vec(off*2,pot*3), module, Chord::VOICING_AMT_PARAM, 0.0, 1.0, 0.0));

  

  addParam(ParamWidget::create<FlatG>(Vec(off + 30 ,space*1-15), module, Chord::OFFSET_PARAM, 0.0, 1.0, 0.5));
  addParam(ParamWidget::create<FlatA>(Vec(off + 30 ,space*2-15), module, Chord::INVERSION_PARAM, 0.0, 1.0, 0.0));
  addParam(ParamWidget::create<FlatR>(Vec(off + 30 ,space*3-15), module, Chord::VOICING_PARAM, 0.0, 1.0, 0.0));




//

int right = 95 ;
int left = 30;

  addInput(Port::create<PJ301MIPort>(Vec(left, 180), Port::INPUT, module, Chord::FLAT_3RD_INPUT));
  addInput(Port::create<PJ301MIPort>(Vec(left, 180+jacks*1), Port::INPUT, module, Chord::FLAT_5TH_INPUT));
  addInput(Port::create<PJ301MIPort>(Vec(left, 180+jacks*2), Port::INPUT, module, Chord::FLAT_7TH_INPUT));
  addInput(Port::create<PJ301MIPort>(Vec(left, 180+jacks*3), Port::INPUT, module, Chord::SIX_FOR_7_INPUT));
  addInput(Port::create<PJ301MIPort>(Vec(left, 180+jacks*4), Port::INPUT, module, Chord::SHARP_5_INPUT));

  //
  addInput(Port::create<PJ301MIPort>(Vec(right, 180), Port::INPUT, module, Chord::SUS_2_INPUT));
  addInput(Port::create<PJ301MIPort>(Vec(right, 180+jacks*1), Port::INPUT, module, Chord::SUS_4_INPUT));
  addInput(Port::create<PJ301MIPort>(Vec(right, 180+jacks*2), Port::INPUT, module, Chord::SIX_FOR_5_INPUT));
  addInput(Port::create<PJ301MIPort>(Vec(right, 180+jacks*3), Port::INPUT, module, Chord::ONE_FOR_7_INPUT));
  addInput(Port::create<PJ301MIPort>(Vec(right, 180+jacks*4), Port::INPUT, module, Chord::FLAT_9_INPUT));
  addInput(Port::create<PJ301MIPort>(Vec(right, 180+jacks*5), Port::INPUT, module, Chord::SHARP_9_INPUT));
//


  addParam(ParamWidget::create<LEDB>(Vec(-22 + left, 3+ 180), module, Chord::FLAT_3RD_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + left, 3+ 180+jacks*1), module, Chord::FLAT_5TH_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + left, 3+ 180+jacks*2), module, Chord::FLAT_7TH_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + left, 3+ 180+jacks*3), module, Chord::SIX_FOR_7_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + left, 3+ 180+jacks*4), module, Chord::SHARP_5_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + right,3+  180), module, Chord::SUS_2_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + right,3+  180+jacks*1), module, Chord::SUS_4_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + right,3+  180+jacks*2), module, Chord::SIX_FOR_5_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + right,3+  180+jacks*3), module, Chord::ONE_FOR_7_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + right,3+  180+jacks*4), module, Chord::FLAT_9_PARAM,0.0,1.0,0.0));
  addParam(ParamWidget::create<LEDB>(Vec(-22 + right,3+  180+jacks*5), module, Chord::SHARP_9_PARAM,0.0,1.0,0.0));




  //

  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(left-19.5,185.5), module, Chord::FLAT_3RD_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(left-19.5,185.5+jacks*1), module, Chord::FLAT_5TH_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(left-19.5,185.5+jacks*2), module, Chord::FLAT_7TH_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(left-19.5,185.5+jacks*3), module, Chord::SIX_FOR_7_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(left-19.5,185.5+jacks*4), module, Chord::SHARP_5_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(right-19.5,185.5), module, Chord::SUS_2_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(right-19.5,185.5+jacks*1), module, Chord::SUS_4_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(right-19.5,185.5+jacks*2), module, Chord::SIX_FOR_5_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(right-19.5,185.5+jacks*3), module, Chord::ONE_FOR_7_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(right-19.5,185.5+jacks*4), module, Chord::FLAT_9_LIGHT));
  addChild(GrayModuleLightWidget::create<ChordLight<OrangeLight>>(Vec(right-19.5,185.5+jacks*5), module, Chord::SHARP_9_LIGHT));
  //

  //

  addOutput(Port::create<PJ301MOPort>(Vec(70,jacks*1), Port::OUTPUT, module, Chord::OUTPUT_ROOT));
  addOutput(Port::create<PJ301MOPort>(Vec(70,jacks*2), Port::OUTPUT, module, Chord::OUTPUT_THIRD));
  addOutput(Port::create<PJ301MOPort>(Vec(70,jacks*3), Port::OUTPUT, module, Chord::OUTPUT_FIFTH));
  addOutput(Port::create<PJ301MOPort>(Vec(70,jacks*4), Port::OUTPUT, module, Chord::OUTPUT_SEVENTH));  
    
  addOutput(Port::create<PJ301MOPort>(Vec(97,jacks*1 ), Port::OUTPUT, module, Chord::OUTPUT_1));
  addOutput(Port::create<PJ301MOPort>(Vec(97,jacks*2 ), Port::OUTPUT, module, Chord::OUTPUT_2));
  addOutput(Port::create<PJ301MOPort>(Vec(97,jacks*3 ), Port::OUTPUT, module, Chord::OUTPUT_3));
  addOutput(Port::create<PJ301MOPort>(Vec(97,jacks*4 ), Port::OUTPUT, module, Chord::OUTPUT_4));

  //addChild(GrayModuleLightWidget::create<TinyLight<RedLight>>(Vec(97+22,jacks*1),module, Chord::OUT_1_LIGHT));
  //addChild(GrayModuleLightWidget::create<TinyLight<RedLight>>(Vec(97+22,jacks*2),module, Chord::OUT_1_LIGHT));
  //addChild(GrayModuleLightWidget::create<TinyLight<RedLight>>(Vec(97+22,jacks*3),module, Chord::OUT_1_LIGHT));
  //addChild(GrayModuleLightWidget::create<TinyLight<RedLight>>(Vec(97+22,jacks*4),module, Chord::OUT_1_LIGHT));

  addInput(Port::create<PJ301MIPort>(Vec(97,57+jacks*3), Port::INPUT, module, Chord::INPUT));

}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, Chord) {
   Model *modelChord = Model::create<Chord, ChordWidget>("dBiz", "Chord", "Chord", QUANTIZER_TAG);
   return modelChord;
}

