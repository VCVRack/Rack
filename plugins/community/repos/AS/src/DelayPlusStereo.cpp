//**************************************************************************************
//Delay Plus module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//**************************************************************************************
#include "AS.hpp"

#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/filter.hpp"
#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>

#define HISTORY_SIZE (1<<21)

struct DelayPlusStereoFx : Module {
	enum ParamIds {
		TIME_PARAM_1,
		FEEDBACK_PARAM_1,
		COLOR_PARAM_1,

		TIME_PARAM_2,
		FEEDBACK_PARAM_2,
		COLOR_PARAM_2,
		FBK_LINK_PARAM,
		COLOR_LINK_PARAM,

		MIX_PARAM,
		BYPASS_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		TIME_CV_INPUT_1,
		FEEDBACK__CV_INPUT_1,
		COLOR_CV_INPUT_1,
		COLOR_RETURN_1,

		TIME_CV_INPUT_2,
		FEEDBACK__CV_INPUT_2,
		COLOR_CV_INPUT_2,
		COLOR_RETURN_2,

		MIX_CV_INPUT,
		SIGNAL_INPUT_1,
		SIGNAL_INPUT_2,
		BYPASS_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		COLOR_SEND_1,
		COLOR_SEND_2,

		SIGNAL_OUTPUT_1,
		SIGNAL_OUTPUT_2,
		NUM_OUTPUTS
	};
	  enum LightIds {
		BYPASS_LED,
		NUM_LIGHTS
	}; 

	RCFilter lowpassFilter1;
	RCFilter highpassFilter1;

	RCFilter lowpassFilter2;
	RCFilter highpassFilter2;

	DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer1;
	DoubleRingBuffer<float, 16> outBuffer1;

	DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer2;
	DoubleRingBuffer<float, 16> outBuffer2;
	
	SampleRateConverter<1> src1;
	SampleRateConverter<1> src2;

	SchmittTrigger bypass_button_trig;
	SchmittTrigger bypass_cv_trig;

	int lcd_tempo1 = 0;
	int lcd_tempo2 = 0;
	bool fx_bypass = false;
	float lastWet1 = 0.0f;
	float lastWet2 = 0.0f;
	float feedback1 = 0.0f;
	float feedback2 = 0.0f;
	float color1 = 0.0f;
	float color2 = 0.0f;
	float mix_value = 0.0f;

	float fade_in_fx = 0.0f;
	float fade_in_dry = 0.0f;
	float fade_out_fx = 1.0f;
	float fade_out_dry = 1.0f;
    const float fade_speed = 0.001f;

	float signal_input_1 = 0.0f;
	float signal_input_2 = 0.0f;

	DelayPlusStereoFx() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

	}

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

};

void DelayPlusStereoFx::step() {

	if (bypass_button_trig.process(params[BYPASS_SWITCH].value) || bypass_cv_trig.process(inputs[BYPASS_CV_INPUT].value) ){
		  fx_bypass = !fx_bypass;
		  resetFades();
	}
    lights[BYPASS_LED].value = fx_bypass ? 1.0f : 0.0f;

	signal_input_1 = inputs[SIGNAL_INPUT_1].value;
	//check for MONO/STEREO CONNECTIONS
	if(!inputs[SIGNAL_INPUT_2].active){
		signal_input_2 = inputs[SIGNAL_INPUT_1].value;
	}else{
		signal_input_2 = inputs[SIGNAL_INPUT_2].value;
	}
		
	// DELAY L - Feed input to delay block
	feedback1 = clamp(params[FEEDBACK_PARAM_1].value + inputs[FEEDBACK__CV_INPUT_1].value / 10.0f, 0.0f, 1.0f);
	float dry1 = signal_input_1 + lastWet1 * feedback1;

	// Compute delay time in seconds
	//float delay = 1e-3 * powf(10.0 / 1e-3, clampf(params[TIME_PARAM_1].value + inputs[TIME_CV_INPUT_1].value / 10.0, 0.0, 1.0));
	float delay1 = clamp(params[TIME_PARAM_1].value + inputs[TIME_CV_INPUT_1].value, 0.001f, 10.0f);
	//LCD display tempo  - show value as ms
	 lcd_tempo1 = std::round(delay1*1000);
	// Number of delay samples
	float index1 = delay1 * engineGetSampleRate();

	// Push dry sample into history buffer
	if (!historyBuffer1.full()) {
		historyBuffer1.push(dry1);
	}

	// How many samples do we need consume to catch up?
	float consume1 = index1 - historyBuffer1.size();

    if (outBuffer1.empty()) {
        double ratio1 = 1.0;
        if (consume1 <= -16)
            ratio1 = 0.5;
        else if (consume1 >= 16)
            ratio1 = 2.0;

        float inSR1 = engineGetSampleRate();
        float outSR1 = ratio1 * inSR1;

        int inFrames1 = min(historyBuffer1.size(), 16);
        int outFrames1 = outBuffer1.capacity();
        src1.setRates(inSR1, outSR1);
        src1.process((const Frame<1>*)historyBuffer1.startData(), &inFrames1, (Frame<1>*)outBuffer1.endData(), &outFrames1);
        historyBuffer1.startIncr(inFrames1);
        outBuffer1.endIncr(outFrames1);
    }

	float out1;
	float wet1 = 0.0f;
	if (!outBuffer1.empty()) {
		wet1 = outBuffer1.shift();
	}

	if (!outputs[COLOR_SEND_1].active) {
		//internal color
		// Apply color to delay wet output
		color1 = clamp(params[COLOR_PARAM_1].value + inputs[COLOR_CV_INPUT_1].value / 10.0f, 0.0f, 1.0f);
		float lowpassFreq1 = 10000.0f * powf(10.0f, clamp(2.0*color1, 0.0f, 1.0f));
		lowpassFilter1.setCutoff(lowpassFreq1 / engineGetSampleRate());
		lowpassFilter1.process(wet1);
		wet1 = lowpassFilter1.lowpass();
		float highpassFreq1 = 10.0f * powf(100.0f, clamp(2.0f*color1 - 1.0f, 0.0f, 1.0f));
		highpassFilter1.setCutoff(highpassFreq1 / engineGetSampleRate());
		highpassFilter1.process(wet1);
		wet1 = highpassFilter1.highpass();
	}else {
	//external color, to filter the wet delay signal outside of the module, or to feed another module
		outputs[COLOR_SEND_1].value = wet1;
		wet1 = inputs[COLOR_RETURN_1].value;	
	}
	lastWet1 = wet1;
	// end of delay 1 block

	// DELAY R - Feed input to delay block
	//CHECK IF LINK IS ENABLED FOR FEEDBACK KNOBS
	if(params[FBK_LINK_PARAM].value){
			feedback2 = feedback1; 
	}else{
			feedback2 = clamp(params[FEEDBACK_PARAM_2].value + inputs[FEEDBACK__CV_INPUT_2].value / 10.0f, 0.0f, 1.0f);
	}
	float dry2 = signal_input_2 + lastWet2 * feedback2;

	// Compute delay time in seconds
	//float delay = 1e-3 * powf(10.0 / 1e-3, clampf(params[TIME_PARAM_1].value + inputs[TIME_CV_INPUT_1].value / 10.0, 0.0, 1.0));
	float delay2 = clamp(params[TIME_PARAM_2].value + inputs[TIME_CV_INPUT_2].value, 0.001f, 10.0f);
	//LCD display tempo  - show value as ms
	 lcd_tempo2 = std::round(delay2*1000);
	// Number of delay samples
	float index2 = delay2 * engineGetSampleRate();

	// Push dry sample into history buffer
	if (!historyBuffer2.full()) {
		historyBuffer2.push(dry2);
	}

	// How many samples do we need consume to catch up?
	float consume2 = index2 - historyBuffer2.size();

    if (outBuffer2.empty()) {
        double ratio2 = 1.0;
        if (consume2 <= -16)
            ratio2 = 0.5;
        else if (consume2 >= 16)
            ratio2 = 2.0;

        float inSR2 = engineGetSampleRate();
        float outSR2 = ratio2 * inSR2;

        int inFrames2 = min(historyBuffer2.size(), 16);
        int outFrames2 = outBuffer2.capacity();
        src2.setRates(inSR2, outSR2);
        src2.process((const Frame<1>*)historyBuffer2.startData(), &inFrames2, (Frame<1>*)outBuffer2.endData(), &outFrames2);
        historyBuffer2.startIncr(inFrames2);
        outBuffer2.endIncr(outFrames2);
    }

	float out2;
	float wet2 = 0.0f;
	if (!outBuffer2.empty()) {
		wet2 = outBuffer2.shift();
	}

	if (!outputs[COLOR_SEND_2].active) {
		//internal color
		// Apply color to delay wet output
		if ( params[COLOR_LINK_PARAM].value ) {
			color2 = color1;
		} else {
			color2 = clamp(params[COLOR_PARAM_2].value + inputs[COLOR_CV_INPUT_2].value / 10.0f, 0.0f, 1.0f);
		}
		float lowpassFreq2 = 10000.0f * powf(10.0f, clamp(2.0*color2, 0.0f, 1.0f));
		lowpassFilter2.setCutoff(lowpassFreq2 / engineGetSampleRate());
		lowpassFilter2.process(wet2);
		wet2 = lowpassFilter2.lowpass();
		float highpassFreq2 = 10.0f * powf(100.0f, clamp(2.0f*color2 - 1.0f, 0.0f, 1.0f));
		highpassFilter2.setCutoff(highpassFreq2 / engineGetSampleRate());
		highpassFilter2.process(wet2);
		wet2 = highpassFilter2.highpass();
	}else {
	//external color, to filter the wet delay signal outside of the module, or to feed another module
		outputs[COLOR_SEND_2].value = wet2;
		wet2 = inputs[COLOR_RETURN_2].value;	
	}
	lastWet2 = wet2;
	//mix dry & wet signals
	mix_value = clamp(params[MIX_PARAM].value + inputs[MIX_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
	out1 = crossfade(signal_input_1, wet1, mix_value);
	out2 = crossfade(signal_input_2, wet2, mix_value);
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
        outputs[SIGNAL_OUTPUT_1].value = ( signal_input_1 * fade_in_dry ) + ( out1 * fade_out_fx );
		outputs[SIGNAL_OUTPUT_2].value = ( signal_input_2 * fade_in_dry ) + ( out2 * fade_out_fx );
    }else{
		fade_in_fx += fade_speed;
		if ( fade_in_fx > 1.0f ) {
			fade_in_fx = 1.0f;
		}
		fade_out_dry -= fade_speed;
		if ( fade_out_dry < 0.0f ) {
			fade_out_dry = 0.0f;
		}
        outputs[SIGNAL_OUTPUT_1].value = ( signal_input_1 * fade_out_dry ) + ( out1 * fade_in_fx );
		outputs[SIGNAL_OUTPUT_2].value = ( signal_input_2 * fade_out_dry ) + ( out2 * fade_in_fx );
	}

}

///////////////////////////////////
struct MsDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  MsDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
    // Background
	NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.5);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);    
    // text 
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;   
    to_display << std::right  << std::setw(5) << *value;

    Vec textPos = Vec(4.0f, 17.0f); 

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
////////////////////////////////////
struct DelayPlusStereoFxWidget : ModuleWidget 
{ 
    DelayPlusStereoFxWidget(DelayPlusStereoFx *module);
};

DelayPlusStereoFxWidget::DelayPlusStereoFxWidget(DelayPlusStereoFx *module) : ModuleWidget(module) {

	setPanel(SVG::load(assetPlugin(plugin, "res/DelayPlusStereo.svg")));

	//MS DISPLAY L
	MsDisplayWidget *display1 = new MsDisplayWidget();
	display1->box.pos = Vec(7,50);
	display1->box.size = Vec(70, 20);
	display1->value = &module->lcd_tempo1;
	addChild(display1); 	

	//MS DISPLAY R
	MsDisplayWidget *display2 = new MsDisplayWidget();
	display2->box.pos = Vec(103,50);
	display2->box.size = Vec(70, 20);
	display2->value = &module->lcd_tempo2;
	addChild(display2); 
 	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//KNOBS L
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(37, 78), module, DelayPlusStereoFx::TIME_PARAM_1, 0.0f, 10.0f, 0.375f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(37, 130), module, DelayPlusStereoFx::FEEDBACK_PARAM_1, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(37, 180), module, DelayPlusStereoFx::COLOR_PARAM_1, 0.0f, 1.0f, 0.5f));
	//KNOBS R
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(106, 78), module, DelayPlusStereoFx::TIME_PARAM_2, 0.0f, 10.0f, 0.750f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(106, 130), module, DelayPlusStereoFx::FEEDBACK_PARAM_2, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(106, 180), module, DelayPlusStereoFx::COLOR_PARAM_2, 0.0f, 1.0f, 0.5f));
	//FEEDBACK LINK SWITCH
	addParam(ParamWidget::create<as_CKSS>(Vec(82, 145), module, DelayPlusStereoFx::FBK_LINK_PARAM, 0.0f, 1.0f, 0.0f));
	//COLOR LINK SWITCH
	addParam(ParamWidget::create<as_CKSS>(Vec(82, 195), module, DelayPlusStereoFx::COLOR_LINK_PARAM, 0.0f, 1.0f, 0.0f));
	//MIX KNOB
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(71, 251), module, DelayPlusStereoFx::MIX_PARAM, 0.0f, 1.0f, 0.5f));
	//BYPASS SWITCH
  	addParam(ParamWidget::create<LEDBezel>(Vec(79, 292), module, DelayPlusStereoFx::BYPASS_SWITCH , 0.0f, 1.0f, 0.0f));
  	addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(79+2.2, 294), module, DelayPlusStereoFx::BYPASS_LED));
	//INPUTS CV L
	addInput(Port::create<as_PJ301MPort>(Vec(7, 87), Port::INPUT, module, DelayPlusStereoFx::TIME_CV_INPUT_1));
	addInput(Port::create<as_PJ301MPort>(Vec(7, 137), Port::INPUT, module, DelayPlusStereoFx::FEEDBACK__CV_INPUT_1));
	addInput(Port::create<as_PJ301MPort>(Vec(7, 187), Port::INPUT, module, DelayPlusStereoFx::COLOR_CV_INPUT_1));
	//INPUTS CV R
	addInput(Port::create<as_PJ301MPort>(Vec(150, 87), Port::INPUT, module, DelayPlusStereoFx::TIME_CV_INPUT_2));
	addInput(Port::create<as_PJ301MPort>(Vec(150, 137), Port::INPUT, module, DelayPlusStereoFx::FEEDBACK__CV_INPUT_2));
	addInput(Port::create<as_PJ301MPort>(Vec(150, 187), Port::INPUT, module, DelayPlusStereoFx::COLOR_CV_INPUT_2));

	//DELAY SIGNAL SEND L
	addOutput(Port::create<as_PJ301MPort>(Vec(15, 224), Port::OUTPUT, module, DelayPlusStereoFx::COLOR_SEND_1));
	//DELAY SIGNAL RETURN L
	addInput(Port::create<as_PJ301MPort>(Vec(50, 224), Port::INPUT, module, DelayPlusStereoFx::COLOR_RETURN_1));

	//DELAY SIGNAL SEND R
	addOutput(Port::create<as_PJ301MPort>(Vec(105, 224), Port::OUTPUT, module, DelayPlusStereoFx::COLOR_SEND_2));
	//DELAY SIGNAL RETURN R
	addInput(Port::create<as_PJ301MPort>(Vec(140, 224), Port::INPUT, module, DelayPlusStereoFx::COLOR_RETURN_2));

	//MIX CV INPUT
	addInput(Port::create<as_PJ301MPort>(Vec(40, 258), Port::INPUT, module, DelayPlusStereoFx::MIX_CV_INPUT));
	//SIGNAL INPUT L
	addInput(Port::create<as_PJ301MPort>(Vec(20, 300), Port::INPUT, module, DelayPlusStereoFx::SIGNAL_INPUT_1));
	//SIGNAL INPUT R
	addInput(Port::create<as_PJ301MPort>(Vec(20, 330), Port::INPUT, module, DelayPlusStereoFx::SIGNAL_INPUT_2));
	//SIGNAL OUTPUT L
	addOutput(Port::create<as_PJ301MPort>(Vec(135, 300), Port::OUTPUT, module, DelayPlusStereoFx::SIGNAL_OUTPUT_1));
	//SIGNAL OUTPUT R
	addOutput(Port::create<as_PJ301MPort>(Vec(135, 330), Port::OUTPUT, module, DelayPlusStereoFx::SIGNAL_OUTPUT_2));

	//BYPASS CV INPUT
	addInput(Port::create<as_PJ301MPort>(Vec(78, 322), Port::INPUT, module, DelayPlusStereoFx::BYPASS_CV_INPUT));

}

RACK_PLUGIN_MODEL_INIT(AS, DelayPlusStereoFx) {
   Model *modelDelayPlusStereoFx = Model::create<DelayPlusStereoFx, DelayPlusStereoFxWidget>("AS", "DelayPlusStereoFx", "DelayPlus Stereo Fx", DUAL_TAG, DELAY_TAG, EFFECT_TAG);
   return modelDelayPlusStereoFx;
}
