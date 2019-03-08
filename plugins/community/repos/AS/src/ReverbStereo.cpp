//***********************************************************************************************
//
//Reverb Stereo module for VCV Rack by Alfredo Santamaria  - AS - https://github.com/AScustomWorks/AS
//
//Based on code from ML_Modules by martin-lueders https://github.com/martin-lueders/ML_modules
//And code from Freeverb by Jezar at Dreampoint - http://www.dreampoint.co.uk
//
//***********************************************************************************************

#include "AS.hpp"
#include "dsp/digital.hpp"

#include "../freeverb/revmodel.hpp"

struct ReverbStereoFx : Module{
	enum ParamIds {
		DECAY_PARAM,
		DAMP_PARAM,
		BLEND_PARAM,
		BYPASS_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_INPUT_L,
		SIGNAL_INPUT_R,
		DECAY_CV_INPUT,
		DAMP_CV_INPUT,
		BLEND_CV_INPUT,
		BYPASS_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIGNAL_OUTPUT_L,
		SIGNAL_OUTPUT_R,
		NUM_OUTPUTS
	};
	enum LightIds {
		DECAY_LIGHT,
		DAMP_LIGHT,
		BLEND_LIGHT,
		BYPASS_LED,
		NUM_LIGHTS
	};

	revmodel reverb;
	float roomsize, damp; 

	SchmittTrigger bypass_button_trig;
	SchmittTrigger bypass_cv_trig;

	bool fx_bypass = false;

	float input_signal_L = 0.0f;
	float input_signal_R = 0.0f;
	float mix_value = 0.0f;
	float outL = 0.0f;
	float outR = 0.0f;

	float fade_in_fx = 0.0f;
	float fade_in_dry = 0.0f;
	float fade_out_fx = 1.0f;
	float fade_out_dry = 1.0f;
    const float fade_speed = 0.001f;

	ReverbStereoFx() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

		float gSampleRate = engineGetSampleRate();
		reverb.init(gSampleRate);
	}

	void step() override;

	void onSampleRateChange() override;

	json_t *toJson()override {
		json_t *rootJm = json_object();

		json_t *statesJ = json_array();
		
			json_t *bypassJ = json_boolean(fx_bypass);
			json_array_append_new(statesJ, bypassJ);
		
		json_object_set_new(rootJm, "as_FxBypass", statesJ);

		return rootJm;
	}

	void fromJson(json_t *rootJm)override {
		json_t *statesJ = json_object_get(rootJm, "as_FxBypass");
		
			json_t *bypassJ = json_array_get(statesJ, 0);

			fx_bypass = !!json_boolean_value(bypassJ);
		
	}

	void resetFades(){
		fade_in_fx = 0.0f;
		fade_in_dry = 0.0f;
		fade_out_fx = 1.0f;
		fade_out_dry = 1.0f;
	}
	
};

void ReverbStereoFx::onSampleRateChange() {

	float gSampleRate = engineGetSampleRate();

	reverb.init(gSampleRate);

	reverb.setdamp(damp);
	reverb.setroomsize(roomsize);
	reverb.setwidth(1.0f);

};


void ReverbStereoFx::step() {

	if (bypass_button_trig.process(params[BYPASS_SWITCH].value) || bypass_cv_trig.process(inputs[BYPASS_CV_INPUT].value) ){
		fx_bypass = !fx_bypass;
		resetFades();
	}
    lights[BYPASS_LED].value = fx_bypass ? 1.0f : 0.0f;

	float wetL, wetR;

	wetL = wetR = 0.0f;

	float old_roomsize = roomsize;
	float old_damp = damp;

	input_signal_L = clamp(inputs[SIGNAL_INPUT_L].value,-10.0f,10.0f);

	if(!inputs[SIGNAL_INPUT_R].active){
		input_signal_R = input_signal_L;
	}else{
		input_signal_R = clamp(inputs[SIGNAL_INPUT_R].value,-10.0f,10.0f);
	}

	roomsize = clamp(params[DECAY_PARAM].value + inputs[DECAY_CV_INPUT].value / 10.0f, 0.0f, 0.88f);
	damp = clamp(params[DAMP_PARAM].value + inputs[DAMP_CV_INPUT].value / 10.0f, 0.0f, 1.0f);

	if( old_damp != damp ) reverb.setdamp(damp);
	if( old_roomsize != roomsize) reverb.setroomsize(roomsize);


	reverb.process(input_signal_L + input_signal_R, wetL, wetR);

	/*
	//original mix method, changed to work better when used with a mixer FX loop
	float outL = input_signal_L + wetL * clamp(params[BLEND_PARAM].value + inputs[BLEND_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
	float outR = input_signal_R + wetR * clamp(params[BLEND_PARAM].value + inputs[BLEND_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
	*/

	mix_value = clamp(params[BLEND_PARAM].value + inputs[BLEND_CV_INPUT].value / 10.0f, 0.0f, 1.0f);

	outL = crossfade(input_signal_L, wetL, mix_value);
	outR = crossfade(input_signal_R, wetR, mix_value);

	//check bypass switch status
	if (fx_bypass){
		fade_in_dry += fade_speed;
		if ( fade_in_dry > 1.0f ) {
			fade_in_dry = 1.0f;
		}
		fade_out_fx -= fade_speed;
		if ( fade_out_fx < 0.0f ) {
			fade_out_fx = 0.0f;
		}
        outputs[SIGNAL_OUTPUT_L].value = ( input_signal_L * fade_in_dry ) + ( outL * fade_out_fx );
		outputs[SIGNAL_OUTPUT_R].value = ( input_signal_R * fade_in_dry ) + ( outR * fade_out_fx );
    }else{
		fade_in_fx += fade_speed;
		if ( fade_in_fx > 1.0f ) {
			fade_in_fx = 1.0f;
		}
		fade_out_dry -= fade_speed;
		if ( fade_out_dry < 0.0f ) {
			fade_out_dry = 0.0f;
		}
        outputs[SIGNAL_OUTPUT_L].value = ( input_signal_L * fade_out_dry ) + ( outL * fade_in_fx );
		outputs[SIGNAL_OUTPUT_R].value = ( input_signal_R * fade_out_dry ) + ( outR * fade_in_fx );
	}

	lights[DECAY_LIGHT].value = clamp(params[DECAY_PARAM].value + inputs[DECAY_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
	lights[DAMP_LIGHT].value = clamp(params[DAMP_PARAM].value + inputs[DAMP_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
	lights[BLEND_LIGHT].value = clamp(params[BLEND_PARAM].value + inputs[BLEND_CV_INPUT].value / 10.0f, 0.0f, 1.0f);

}

struct ReverbStereoFxWidget : ModuleWidget 
{ 
    ReverbStereoFxWidget(ReverbStereoFx *module);
};


ReverbStereoFxWidget::ReverbStereoFxWidget(ReverbStereoFx *module) : ModuleWidget(module) {

  setPanel(SVG::load(assetPlugin(plugin, "res/ReverbStereo.svg")));
 	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//KNOBS  
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(43, 60), module, ReverbStereoFx::DECAY_PARAM, 0.0f, 0.95f, 0.475f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(43, 125), module, ReverbStereoFx::DAMP_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(43, 190), module, ReverbStereoFx::BLEND_PARAM, 0.0f, 1.0f, 0.5f));
	//LIGHTS
	addChild(ModuleLightWidget::create<SmallLight<YellowLight>>(Vec(39, 57), module, ReverbStereoFx::DECAY_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<YellowLight>>(Vec(39, 122), module, ReverbStereoFx::DAMP_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<YellowLight>>(Vec(39, 187), module, ReverbStereoFx::BLEND_LIGHT));
    //BYPASS SWITCH
  	addParam(ParamWidget::create<LEDBezel>(Vec(55, 260), module, ReverbStereoFx::BYPASS_SWITCH , 0.0f, 1.0f, 0.0f));
  	addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(57.2, 262), module, ReverbStereoFx::BYPASS_LED));
	/*  
    //INS/OUTS
	addInput(Port::create<as_PJ301MPort>(Vec(10, 310), Port::INPUT, module, ReverbStereoFx::SIGNAL_INPUT_L));
	addOutput(Port::create<as_PJ301MPort>(Vec(55, 310), Port::OUTPUT, module, ReverbStereoFx::SIGNAL_OUTPUT_L));
	*/
	//INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(15, 300), Port::INPUT, module, ReverbStereoFx::SIGNAL_INPUT_L));
	addInput(Port::create<as_PJ301MPort>(Vec(15, 330), Port::INPUT, module, ReverbStereoFx::SIGNAL_INPUT_R));
	//OUTPUTS
	addOutput(Port::create<as_PJ301MPort>(Vec(50, 300), Port::OUTPUT, module, ReverbStereoFx::SIGNAL_OUTPUT_L));
	addOutput(Port::create<as_PJ301MPort>(Vec(50, 330), Port::OUTPUT, module, ReverbStereoFx::SIGNAL_OUTPUT_R));
	//CV INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(10, 67), Port::INPUT, module, ReverbStereoFx::DECAY_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(10, 132), Port::INPUT, module, ReverbStereoFx::DAMP_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(10, 197), Port::INPUT, module, ReverbStereoFx::BLEND_CV_INPUT));

	//BYPASS CV INPUT
	addInput(Port::create<as_PJ301MPort>(Vec(10, 259), Port::INPUT, module, ReverbStereoFx::BYPASS_CV_INPUT));
	
}

RACK_PLUGIN_MODEL_INIT(AS, ReverbStereoFx) {
   Model *modelReverbStereoFx = Model::create<ReverbStereoFx, ReverbStereoFxWidget>("AS", "ReverbStereoFx", "Reverb Stereo FX", REVERB_TAG, EFFECT_TAG);
   return modelReverbStereoFx;
}
