//***********************************************************************************************
//
//SuperDriveFx module for VCV Rack by Alfredo Santamaria  - AS - https://github.com/AScustomWorks/AS
//Variable-hardness clipping code from scoofy[ AT ]inf[ DOT ]elte[ DOT ]hu
//Filter code from from VCV rack dsp
//
//***********************************************************************************************

#include "AS.hpp"
#include "dsp/digital.hpp"
#include "dsp/filter.hpp"

//#include <stdlib.h>

struct SuperDriveFx : Module{
	enum ParamIds {
		DRIVE_PARAM,
		OUTPUT_GAIN_PARAM,
		TONE_PARAM,
        BYPASS_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_INPUT,
		DRIVE_CV_INPUT,
		GAIN_CV_INPUT,
		TONE_CV_INPUT,
		BYPASS_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIGNAL_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		GAIN_LIGHT,
		TONE_LIGHT,
		DRIVE_LIGHT,
		BYPASS_LED,
		NUM_LIGHTS
	};

	SchmittTrigger bypass_button_trig;
	SchmittTrigger bypass_cv_trig;

	int drive_scale=50;//to handle cv parameters properly

	RCFilter lowpassFilter;
	RCFilter highpassFilter;

	bool fx_bypass = false;

	float fade_in_fx = 0.0f;
	float fade_in_dry = 0.0f;
	float fade_out_fx = 1.0f;
	float fade_out_dry = 1.0f;
    const float fade_speed = 0.001f;

	SuperDriveFx() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

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

	float input_signal=0.0f;
	float drive = 0.1f;
	float process= 0.0f;
	float inv_atan_drive = 0.0f;
	float output_signal= 0.0f;
	
};

void SuperDriveFx::step() {

  	if (bypass_button_trig.process(params[BYPASS_SWITCH].value) || bypass_cv_trig.process(inputs[BYPASS_CV_INPUT].value) ){
		  fx_bypass = !fx_bypass;
		  resetFades();
	}
    lights[BYPASS_LED].value = fx_bypass ? 1.0f : 0.0f;

	float input_signal = inputs[SIGNAL_INPUT].value;
	//OVERDRIVE SIGNAL
	//float drive = params[DRIVE_PARAM].value;
	drive = clamp(params[DRIVE_PARAM].value + inputs[DRIVE_CV_INPUT].value / 10.0f, 0.1f, 1.0f);

	drive = drive * drive_scale;
	//precalc
	inv_atan_drive = 1.0f/atan(drive);
	//process
	process = inv_atan_drive * atan(input_signal*drive);
	//output_signal = process * params[OUTPUT_GAIN_PARAM].value;
	output_signal = process * clamp(params[OUTPUT_GAIN_PARAM].value + inputs[GAIN_CV_INPUT].value / 10.0f, 0.0f, 1.0f);

	//TONE CONTROL
	float tone = clamp(params[TONE_PARAM].value + inputs[TONE_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
	float lowpassFreq = 10000.0f * powf(10.0f, clamp(2.0f*tone, 0.0f, 1.0f));
	lowpassFilter.setCutoff(lowpassFreq / engineGetSampleRate());
	lowpassFilter.process(output_signal);
	output_signal = lowpassFilter.lowpass();
	float highpassFreq = 10.0f * powf(100.0f, clamp(2.0f*tone - 1.0f, 0.0f, 1.0f));
	highpassFilter.setCutoff(highpassFreq / engineGetSampleRate());
	highpassFilter.process(output_signal);
	output_signal = highpassFilter.highpass();

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
        outputs[SIGNAL_OUTPUT].value = ( input_signal * fade_in_dry ) + ( (output_signal*3.5f) * fade_out_fx );
    }else{
		fade_in_fx += fade_speed;
		if ( fade_in_fx > 1.0f ) {
			fade_in_fx = 1.0f;
		}
		fade_out_dry -= fade_speed;
		if ( fade_out_dry < 0.0f ) {
			fade_out_dry = 0.0f;
		}
        outputs[SIGNAL_OUTPUT].value = ( input_signal * fade_out_dry ) + ( (output_signal*3.5f) * fade_in_fx );
	}

	lights[DRIVE_LIGHT].value = clamp(params[DRIVE_PARAM].value + inputs[DRIVE_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
	lights[TONE_LIGHT].value = clamp(params[TONE_PARAM].value + inputs[TONE_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
	lights[GAIN_LIGHT].value = clamp(params[OUTPUT_GAIN_PARAM].value + inputs[GAIN_CV_INPUT].value / 10.0f, 0.0f, 1.0f);

}

struct SuperDriveFxWidget : ModuleWidget 
{ 
    SuperDriveFxWidget(SuperDriveFx *module);
};


SuperDriveFxWidget::SuperDriveFxWidget(SuperDriveFx *module) : ModuleWidget(module) {
	
   setPanel(SVG::load(assetPlugin(plugin, "res/SuperDrive.svg")));
   
 	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    //KNOBS  
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(43, 60), module, SuperDriveFx::DRIVE_PARAM, 0.1f, 1.0f, 0.1f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(43, 125), module, SuperDriveFx::TONE_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(43, 190), module, SuperDriveFx::OUTPUT_GAIN_PARAM, 0.0f, 1.0f, 0.5f));
	//LIGHTS
	addChild(ModuleLightWidget::create<SmallLight<YellowLight>>(Vec(39, 57), module, SuperDriveFx::DRIVE_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<YellowLight>>(Vec(39, 122), module, SuperDriveFx::TONE_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<YellowLight>>(Vec(39, 187), module, SuperDriveFx::GAIN_LIGHT));
    //BYPASS SWITCH
  	addParam(ParamWidget::create<LEDBezel>(Vec(55, 260), module, SuperDriveFx::BYPASS_SWITCH , 0.0f, 1.0f, 0.0f));
  	addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(57.2, 262), module, SuperDriveFx::BYPASS_LED));
    //INS/OUTS
	addInput(Port::create<as_PJ301MPort>(Vec(10, 310), Port::INPUT, module, SuperDriveFx::SIGNAL_INPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(55, 310), Port::OUTPUT, module, SuperDriveFx::SIGNAL_OUTPUT));
	//CV INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(10, 67), Port::INPUT, module, SuperDriveFx::DRIVE_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(10, 132), Port::INPUT, module, SuperDriveFx::TONE_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(10, 197), Port::INPUT, module, SuperDriveFx::GAIN_CV_INPUT));

	//BYPASS CV INPUT
	addInput(Port::create<as_PJ301MPort>(Vec(10, 259), Port::INPUT, module, SuperDriveFx::BYPASS_CV_INPUT));
 
}

RACK_PLUGIN_MODEL_INIT(AS, SuperDriveFx) {
   Model *modelSuperDriveFx = Model::create<SuperDriveFx, SuperDriveFxWidget>("AS", "SuperDriveFx", "Super Drive FX", AMPLIFIER_TAG, EFFECT_TAG);
   return modelSuperDriveFx;
}

