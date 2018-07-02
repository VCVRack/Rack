//**************************************************************************************
//Signal Delay module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//**************************************************************************************
#include "AS.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"

#include <sstream>
#include <iomanip>

#define HISTORY_SIZE (1<<21)

struct SignalDelay : Module {
	enum ParamIds {
		TIME_1_PARAM,
		TIME_2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TIME_1_INPUT,
		TIME_2_INPUT,
		IN_1_INPUT,
		IN_2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		THRU_1_OUTPUT,
		THRU_2_OUTPUT,
		OUT_1_OUTPUT,
		OUT_2_OUTPUT,
		NUM_OUTPUTS
	};

	DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer1;
	DoubleRingBuffer<float, 16> outBuffer1;
	SampleRateConverter<1> src1;
	float lastWet1 = 0.0f;
	int lcd_tempo1 = 0;

	DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer2;
	DoubleRingBuffer<float, 16> outBuffer2;
	SampleRateConverter<1> src2;
	float lastWet2 = 0.0f;
	int lcd_tempo2 = 0;


	SignalDelay() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {

	}

	void step() override;

};

void SignalDelay::step() {

	// DELAY 1 Get input to delay block
	float in1 = inputs[IN_1_INPUT].value;
	float feedback1 = 0;//only one repetition, for regular use: clampf(params[FEEDBACK_PARAM].value + inputs[FEEDBACK_INPUT].value / 10.0, 0.0, 1.0);
	float dry1 = in1 + lastWet1 * feedback1;
	// Compute delay time in seconds
	//delay time in seconds. Linear reading, now easy to setup any value by the digit
	float delay1 = clamp(params[TIME_1_PARAM].value + inputs[TIME_1_INPUT].value, 0.001f, 10.0f);
	//LCD display tempo  - show value as ms
	lcd_tempo1 = std::round(delay1*1000);
	// Number of delay samples
	float index1 = delay1 * engineGetSampleRate();
	// Push dry1 sample into history buffer
	if (!historyBuffer1.full()) {
		historyBuffer1.push(dry1);
	}
	// How many samples do we need consume1 to catch up?
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

	float wet1 = 0.0f;
	if (!outBuffer1.empty()) {
		wet1 = outBuffer1.shift();
	}
	outputs[THRU_1_OUTPUT].value = in1;
	outputs[OUT_1_OUTPUT].value = wet1;	
	lastWet1 = wet1;

	// DELAY 2 Get input to delay block
	float in2 = inputs[IN_2_INPUT].value;
	float feedback2 = 0;//only one repetition, for regular use: clamp(params[FEEDBACK_PARAM].value + inputs[FEEDBACK_INPUT].value / 10.0, 0.0, 1.0);
	float dry2 = in2 + lastWet2 * feedback2;
	// Compute delay time in seconds
	//delay time in seconds. Linear reading, now easy to setup any value by the digit
	float delay2 = clamp(params[TIME_2_PARAM].value + inputs[TIME_2_INPUT].value, 0.001f, 10.0f);
	//LCD display tempo  - show value as ms
	lcd_tempo2 = std::round(delay2*1000);
	// Number of delay samples
	float index2 = delay2 * engineGetSampleRate();
	// Push dry sample into history buffer
	if (!historyBuffer2.full()) {
		historyBuffer2.push(dry2);
	}
	// How many samples do we need consume1 to catch up?
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

	float wet2 = 0.0;
	if (!outBuffer2.empty()) {
		wet2 = outBuffer2.shift();
	}
	outputs[THRU_2_OUTPUT].value = in2;
	outputs[OUT_2_OUTPUT].value = wet2;	
	lastWet2 = wet2;



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
    //NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
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

struct SignalDelayWidget : ModuleWidget 
{ 
    SignalDelayWidget(SignalDelay *module);
};


SignalDelayWidget::SignalDelayWidget(SignalDelay *module) : ModuleWidget(module) {

	setPanel(SVG::load(assetPlugin(plugin, "res/SignalDelay.svg")));

	//DELAY 1
	//MS DISPLAY 
	MsDisplayWidget *display1 = new MsDisplayWidget();
	display1->box.pos = Vec(10,50);
	display1->box.size = Vec(70, 20);
	display1->value = &module->lcd_tempo1;
	addChild(display1); 
	static const float posX[3] = {3,33,63};	
 	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//KNOBS
	addParam(ParamWidget::create<as_KnobBlack>(Vec(47, 80), module, SignalDelay::TIME_1_PARAM, 0.001f, 10.0f, 0.350f));
	//CV INPUT
	addInput(Port::create<as_PJ301MPort>(Vec(posX[0]+5, 87), Port::INPUT, module, SignalDelay::TIME_1_INPUT));
	//INPUT
	addInput(Port::create<as_PJ301MPort>(Vec(posX[0], 160), Port::INPUT, module, SignalDelay::IN_1_INPUT));
	//OUTPUTS
	addOutput(Port::create<as_PJ301MPort>(Vec(posX[1], 160), Port::OUTPUT, module, SignalDelay::THRU_1_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(posX[2], 160), Port::OUTPUT, module, SignalDelay::OUT_1_OUTPUT));

	//DELAY 2
	//MS DISPLAY 
	static const int mod_offset=160;
	MsDisplayWidget *display2 = new MsDisplayWidget();
	display2->box.pos = Vec(10,50+mod_offset);
	display2->box.size = Vec(70, 20);
	display2->value = &module->lcd_tempo2;
	addChild(display2); 
	//KNOBS
	addParam(ParamWidget::create<as_KnobBlack>(Vec(47, 80+mod_offset), module, SignalDelay::TIME_2_PARAM, 0.001f, 10.0f, 0.350f));
	//CV INPUT
	addInput(Port::create<as_PJ301MPort>(Vec(posX[0]+5, 87+mod_offset), Port::INPUT, module, SignalDelay::TIME_2_INPUT));
	//INPUT
	addInput(Port::create<as_PJ301MPort>(Vec(posX[0], 160+mod_offset), Port::INPUT, module, SignalDelay::IN_2_INPUT));
	//OUTPUTS
	addOutput(Port::create<as_PJ301MPort>(Vec(posX[1], 160+mod_offset), Port::OUTPUT, module, SignalDelay::THRU_2_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(posX[2], 160+mod_offset), Port::OUTPUT, module, SignalDelay::OUT_2_OUTPUT));
}

RACK_PLUGIN_MODEL_INIT(AS, SignalDelay) {
   Model *modelSignalDelay = Model::create<SignalDelay, SignalDelayWidget>("AS", "SignalDelay", "Signal Delay", UTILITY_TAG, DELAY_TAG);
   return modelSignalDelay;
}
