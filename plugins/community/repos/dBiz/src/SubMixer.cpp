///////////////////////////////////////////////////
//
//   dBiz revisited version of 
//   Sub Mixer VCV Module
//   Built from fundamental VCMixer 
//
//   Strum 2017
//
///////////////////////////////////////////////////
#include "dBiz.hpp"
#include "dsp/vumeter.hpp"

namespace rack_plugin_dBiz {

struct SubMix : Module {
	enum ParamIds {
		MIX_PARAM,
		CH1_PARAM,
    CH1_PAN_PARAM,
		CH2_PARAM,
    CH2_PAN_PARAM,
		CH3_PARAM,
    CH3_PAN_PARAM,
    CH4_PARAM,
    CH4_PAN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		MIX_CV_INPUT,
		CH1_INPUT,
		CH1_CV_INPUT,
    CH1_PAN_INPUT,
		CH2_INPUT,
		CH2_CV_INPUT,
    CH2_PAN_INPUT,
		CH3_INPUT,
		CH3_CV_INPUT,
    CH3_PAN_INPUT,
    CH4_INPUT,
		CH4_CV_INPUT,
    CH4_PAN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MIX_OUTPUT_L,
    MIX_OUTPUT_R,
		CH1_OUTPUT,
		CH2_OUTPUT,
		CH3_OUTPUT,
    CH4_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds
	{
		VU1_LIGHT,
		VU2_LIGHT = VU1_LIGHT + 5,
		VU3_LIGHT = VU2_LIGHT + 5,
		VU4_LIGHT = VU3_LIGHT + 5,
		NUM_LIGHTS = VU4_LIGHT + 5
	};

	SubMix() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS,NUM_LIGHTS) {};
	void step() override ;
};


void SubMix::step() {

  
  float ch1 = inputs[CH1_INPUT].value * params[CH1_PARAM].value * clamp(inputs[CH1_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

  float pan_cv_in_1 = inputs[CH1_PAN_INPUT].value/5;
  float pan_position_1 = pan_cv_in_1 + params[CH1_PAN_PARAM].value;
   
  if (pan_position_1 < 0) pan_position_1 = 0;
  if (pan_position_1 > 1) pan_position_1 = 1;  
    
  float ch1_l = ch1 * (1-pan_position_1)* 2;
  float ch1_r = ch1 * pan_position_1 * 2;

	VUMeter vuMeter1;
	vuMeter1.dBInterval = 6.0;
	vuMeter1.setValue(ch1 / 10.0);
	for (int j = 0; j < 5; j++)
	{
		lights[VU1_LIGHT+j].setBrightnessSmooth(vuMeter1.getBrightness(j));
	}

	float ch2 = inputs[CH2_INPUT].value * params[CH2_PARAM].value * clamp(inputs[CH2_CV_INPUT].normalize(10.0f) / 10.0f, -1.0f, 1.0f);
  
  float pan_cv_in_2 = inputs[CH2_PAN_INPUT].value/5;
  float pan_position_2 = pan_cv_in_2 + params[CH2_PAN_PARAM].value;
   
  if (pan_position_2 < 0) pan_position_2 = 0;
  if (pan_position_2 > 1) pan_position_2 = 1;  
    
  float ch2_l = ch2 * (1-pan_position_2)* 2;
  float ch2_r = ch2 * pan_position_2 * 2;

	VUMeter vuMeter2;
	vuMeter2.dBInterval = 6.0;
	vuMeter2.setValue(ch2 / 10.0);
	for (int j = 0; j < 5; j++)
	{
		lights[VU2_LIGHT + j].setBrightnessSmooth(vuMeter2.getBrightness(j));
	}

	float ch3 = inputs[CH3_INPUT].value * params[CH3_PARAM].value * clamp(inputs[CH3_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

  float pan_cv_in_3 = inputs[CH3_PAN_INPUT].value/5;
  float pan_position_3 = pan_cv_in_3 + params[CH3_PAN_PARAM].value;
   
  if (pan_position_3 < 0) pan_position_3 = 0;
  if (pan_position_3 > 1) pan_position_3 = 1;  
    
  float ch3_l = ch3 * (1-pan_position_3)* 2;
  float ch3_r = ch3 * pan_position_3 * 2;

	VUMeter vuMeter3;
	vuMeter3.dBInterval = 6.0;
	vuMeter3.setValue(ch3 / 10.0);
	for (int j = 0; j < 5; j++)
	{
		lights[VU3_LIGHT + j].setBrightnessSmooth(vuMeter3.getBrightness(j));
	}

	float ch4 = inputs[CH4_INPUT].value * params[CH4_PARAM].value * clamp(inputs[CH4_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

  float pan_cv_in_4 = inputs[CH4_PAN_INPUT].value/5;
  float pan_position_4 = pan_cv_in_4 + params[CH4_PAN_PARAM].value;
   
  if (pan_position_4 < 0) pan_position_4 = 0;
  if (pan_position_4 > 1) pan_position_4 = 1;  
    
  float ch4_l = ch4 * (1-pan_position_4)* 2;
  float ch4_r = ch4 * pan_position_4 * 2;

	VUMeter vuMeter4;
	vuMeter4.dBInterval = 6.0;
	vuMeter4.setValue(ch4 / 10.0);
	for (int j = 0; j < 5; j++)
	{
		lights[VU4_LIGHT + j].setBrightnessSmooth(vuMeter4.getBrightness(j));
	}

	float mix_l = (ch1_l + ch2_l + ch3_l + ch4_l) * params[MIX_PARAM].value * inputs[MIX_CV_INPUT].normalize(10.0) / 10.0;
  float mix_r = (ch1_r + ch2_r + ch3_r + ch4_r) * params[MIX_PARAM].value * inputs[MIX_CV_INPUT].normalize(10.0) / 10.0;

	outputs[CH1_OUTPUT].value = ch1;
	outputs[CH2_OUTPUT].value = ch2;
	outputs[CH3_OUTPUT].value = ch3;
  outputs[CH4_OUTPUT].value = ch4;
	outputs[MIX_OUTPUT_L].value = mix_l;
  outputs[MIX_OUTPUT_R].value = mix_r;
}


struct SubMixWidget : ModuleWidget 
{
SubMixWidget(SubMix *module) : ModuleWidget(module)
{
	box.size = Vec(15*12, 380);


	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin,"res/SubMix.svg")));
		addChild(panel);
	}
  int knob = 40;
	int jcv = 27;
	int cvs = 230;
	int border=17;
	int borderl = 25;
	int light = 13;

	addParam(ParamWidget::create<LRoundWhy>(Vec(10, 15), module, SubMix::MIX_PARAM, 0.0, 1.0, 0.5));
  addInput(Port::create<PJ301MIPort>(Vec(box.size.x-10-jcv*4, 20), Port::INPUT, module, SubMix::MIX_CV_INPUT));

//Screw

  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

  
//mixer out

  addOutput(Port::create<PJ301MLPort>(Vec(box.size.x-10-jcv*1, 20), Port::OUTPUT, module, SubMix::MIX_OUTPUT_L));
	addOutput(Port::create<PJ301MRPort>(Vec(box.size.x-10-jcv*2, 20), Port::OUTPUT, module, SubMix::MIX_OUTPUT_R));
  
//


  	addParam(ParamWidget::create<RoundRed>(Vec(10+knob*0,150), module, SubMix::CH1_PARAM, 0.0, 1.0, 0.0));
 	  addParam(ParamWidget::create<RoundRed>(Vec(10+knob*1,150), module, SubMix::CH2_PARAM, 0.0, 1.0, 0.0));
  	addParam(ParamWidget::create<RoundRed>(Vec(10+knob*2,150), module, SubMix::CH3_PARAM, 0.0, 1.0, 0.0));
  	addParam(ParamWidget::create<RoundRed>(Vec(10+knob*3,150), module, SubMix::CH4_PARAM, 0.0, 1.0, 0.0));
//  
    addParam(ParamWidget::create<RoundAzz>(Vec(10+knob*0,190), module, SubMix::CH1_PAN_PARAM, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundAzz>(Vec(10+knob*1,190), module, SubMix::CH2_PAN_PARAM, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundAzz>(Vec(10+knob*2,190), module, SubMix::CH3_PAN_PARAM, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundAzz>(Vec(10+knob*3,190), module, SubMix::CH4_PAN_PARAM, 0.0, 1.0, 0.5));		
//  
//
		addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(borderl,80), module, SubMix::VU1_LIGHT + 0));
		addChild(GrayModuleLightWidget::create<MediumLight<YellowLight>>(Vec(borderl,80 + light), module, SubMix::VU1_LIGHT + 1));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl, 80 + light * 2), module, SubMix::VU1_LIGHT + 2));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl,80 + light * 3 ), module, SubMix::VU1_LIGHT + 3));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl,80 + light * 4 ), module, SubMix::VU1_LIGHT + 4));
		addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(borderl+knob,80 ), module, SubMix::VU2_LIGHT + 0));
		addChild(GrayModuleLightWidget::create<MediumLight<YellowLight>>(Vec(borderl+knob,80 + light ), module, SubMix::VU2_LIGHT + 1));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl + knob, 80 + light * 2), module, SubMix::VU2_LIGHT + 2));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl + knob, 80 + light * 3), module, SubMix::VU2_LIGHT + 3));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl + knob, 80 + light * 4), module, SubMix::VU2_LIGHT + 4));
		addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(borderl + knob * 2,80 ), module, SubMix::VU3_LIGHT + 0));
		addChild(GrayModuleLightWidget::create<MediumLight<YellowLight>>(Vec(borderl + knob * 2,80 + light ), module, SubMix::VU3_LIGHT + 1));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl + knob * 2, 80 + light * 2), module, SubMix::VU3_LIGHT + 2));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl + knob * 2, 80 + light * 3), module, SubMix::VU3_LIGHT + 3));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl + knob * 2, 80 + light * 4), module, SubMix::VU3_LIGHT + 4));
		addChild(GrayModuleLightWidget::create<MediumLight<RedLight>>(Vec(borderl + knob * 3,80 ), module, SubMix::VU4_LIGHT + 0));
		addChild(GrayModuleLightWidget::create<MediumLight<YellowLight>>(Vec(borderl + knob * 3,80 + light ), module, SubMix::VU4_LIGHT + 1));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl + knob * 3, 80 + light * 2), module, SubMix::VU4_LIGHT + 2));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl + knob * 3, 80 + light * 3), module, SubMix::VU4_LIGHT + 3));
		addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(borderl + knob * 3, 80 + light * 4), module, SubMix::VU4_LIGHT + 4));

		//
		//
		addInput(Port::create<PJ301MIPort>(Vec(border,cvs+jcv*0), Port::INPUT, module, SubMix::CH1_INPUT));
		addInput(Port::create<PJ301MCPort>(Vec(border,cvs+jcv*1), Port::INPUT, module, SubMix::CH1_CV_INPUT));
  	addInput(Port::create<PJ301MCPort>(Vec(border,cvs+jcv*2), Port::INPUT, module, SubMix::CH1_PAN_INPUT));
  	addOutput(Port::create<PJ301MOPort>(Vec(border,cvs+jcv*3), Port::OUTPUT, module, SubMix::CH1_OUTPUT));
//  
		addInput(Port::create<PJ301MIPort>(Vec(border+knob,cvs+jcv*0), Port::INPUT, module, SubMix::CH2_INPUT));
		addInput(Port::create<PJ301MCPort>(Vec(border+knob,cvs+jcv*1), Port::INPUT, module, SubMix::CH2_CV_INPUT));
 	 	addInput(Port::create<PJ301MCPort>(Vec(border+knob,cvs+jcv*2), Port::INPUT, module, SubMix::CH2_PAN_INPUT));
		addOutput(Port::create<PJ301MOPort>(Vec(border+knob,cvs+jcv*3), Port::OUTPUT, module, SubMix::CH2_OUTPUT));
//  
		addInput(Port::create<PJ301MIPort>(Vec(border+knob*2,cvs+jcv*0), Port::INPUT, module, SubMix::CH3_INPUT));
		addInput(Port::create<PJ301MCPort>(Vec(border+knob*2,cvs+jcv*1), Port::INPUT, module, SubMix::CH3_CV_INPUT));
  	addInput(Port::create<PJ301MCPort>(Vec(border+knob*2,cvs+jcv*2), Port::INPUT, module, SubMix::CH3_PAN_INPUT));
		addOutput(Port::create<PJ301MOPort>(Vec(border+knob*2,cvs+jcv*3), Port::OUTPUT, module, SubMix::CH3_OUTPUT));
//  
	 	addInput(Port::create<PJ301MIPort>(Vec(border+knob*3,cvs+jcv*0), Port::INPUT, module, SubMix::CH4_INPUT));
		addInput(Port::create<PJ301MCPort>(Vec(border+knob*3,cvs+jcv*1), Port::INPUT, module, SubMix::CH4_CV_INPUT));
	 	addInput(Port::create<PJ301MCPort>(Vec(border+knob*3,cvs+jcv*2), Port::INPUT, module, SubMix::CH4_PAN_INPUT));
 	  addOutput(Port::create<PJ301MOPort>(Vec(border+knob*3,cvs+jcv*3), Port::OUTPUT, module, SubMix::CH4_OUTPUT));
    
}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, SubMix) {
   Model *modelSubMix = Model::create<SubMix, SubMixWidget>("dBiz", "SubMix", "SubMix", MIXER_TAG);
   return modelSubMix;
}

