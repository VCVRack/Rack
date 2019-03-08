//**************************************************************************************
//TriggersMKII module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//**************************************************************************************
#include "AS.hpp"
#include "dsp/digital.hpp"
#include <sstream>
#include <iomanip>
//#include <iostream>
//#include <cmath>

struct TriggersMKII: Module {
    enum ParamIds {
        LABEL_PARAM_1,
        LABEL_PARAM_2,
        TRIGGER_SWITCH_1,
        MOMENTARY_SWITCH_2,
        NUM_PARAMS
    };
    enum InputIds {
        CV_TRIG_INPUT_1,
        CV_TRIG_INPUT_2,
        NUM_INPUTS
    };
    enum OutputIds {

        TRIGGER_OUT1,
        MOMENTARY_OUT2,

        NUM_OUTPUTS
    };
    enum LightIds {
        RUN_LED,
        TRIGGER_LED_1,
        MOMENTARY_LED_2,
        NUM_LIGHTS
    };

    SchmittTrigger btnTrigger1;
    SchmittTrigger extTrigger1;
    SchmittTrigger btnTrigger2;
    SchmittTrigger extTrigger2;

    const float lightLambda = 0.075f;
    float resetLight1 = 0.0f;
    float resetLight2 = 0.0f;

    int label_num1 = 0;
    int label_num2 = 0;

    PulseGenerator triggerPulse1;
    bool trg_pulse1 = false;

    PulseGenerator triggerPulse2;
    bool trg_pulse2 = false;
 
    TriggersMKII() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
 
    
};

void TriggersMKII::step() {

    label_num1 = roundf(params[LABEL_PARAM_1].value);
    label_num2 = roundf(params[LABEL_PARAM_2].value);

    //TRIGGER 1
    if (btnTrigger1.process(params[TRIGGER_SWITCH_1].value)||extTrigger1.process(inputs[CV_TRIG_INPUT_1].value)) {
        resetLight1 = 1.0;
        triggerPulse1.trigger(1e-3f);
    }

    trg_pulse1 = triggerPulse1.process(1.0 / engineGetSampleRate());
    outputs[TRIGGER_OUT1].value = (trg_pulse1 ? 10.0f : 0.0f);

    resetLight1 -= resetLight1 / lightLambda / engineGetSampleRate();
    lights[TRIGGER_LED_1].value = resetLight1;

    //TRIGGER 2
    if (btnTrigger2.process(params[MOMENTARY_SWITCH_2].value)||extTrigger2.process(inputs[CV_TRIG_INPUT_2].value)) {
        resetLight2 = 1.0;
        triggerPulse2.trigger(1e-3f);
    }
    trg_pulse2 = triggerPulse2.process(1.0 / engineGetSampleRate());
    outputs[MOMENTARY_OUT2].value = (trg_pulse2 ? 10.0f : 0.0f);

    resetLight2 -= resetLight2 / lightLambda / engineGetSampleRate();
    lights[MOMENTARY_LED_2].value = resetLight2;
    
}

static const char *label_values[] = {
    "------",
    "  MUTE",
    "  SOLO",
    " RESET",
	" DRUMS",
    "  KICK",
    " SNARE",
    " HIHAT",
    "  CLAP",
    "  PERC",
	"BASS 1",
	"BASS 2",
	" GTR 1",
	" GTR 2",
	"LEAD 1",
	"LEAD 2",
    " PAD 1",
    " PAD 2",
	"CHORDS",
	"  FX 1",
	"  FX 2",
	" SEQ 1",
	" SEQ 2",
	" MIX 1",
	" MIX 2",    
	" AUX 1",
	" AUX 2",
    "    ON",
    "   OFF",
    " START",
    "  STOP",
    " PAUSE",
    "    UP",
    "  DOWN",
    "  LEFT",
    " RIGHT",
    "   RUN",
};

///////////////////////////////////
struct LabelDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  LabelDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/saxmono.ttf"));
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
    nvgTextLetterSpacing(vg, 2.0);

    std::stringstream to_display;   
    to_display << std::right  << std::setw(5) << *value;

    Vec textPos = Vec(4.0f, 16.0f); 

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "000000", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "------", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    //nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
    nvgText(vg, textPos.x, textPos.y, label_values[*value], NULL);
  }
};
////////////////////////////////////

struct TriggersMKIIWidget : ModuleWidget 
{ 
    TriggersMKIIWidget(TriggersMKII *module);
};

TriggersMKIIWidget::TriggersMKIIWidget(TriggersMKII *module) : ModuleWidget(module) {

   setPanel(SVG::load(assetPlugin(plugin, "res/TriggersMKII.svg")));
   
	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    static const float led_offset = 3.3;
    static const float led_center = 15;
    static const float y_offset = 150;
    //TRIGGER 1
    //LABEL DISPLAY 
	LabelDisplayWidget *display1 = new LabelDisplayWidget();
	display1->box.pos = Vec(6,50);
	display1->box.size = Vec(78, 20);
	display1->value = &module->label_num1;
	addChild(display1); 	

    //PARAM
	addParam(ParamWidget::create<as_KnobBlack>(Vec(46, 77), module, TriggersMKII::LABEL_PARAM_1, 0.0, 36.0, 0.0));
    //SWITCH
    addParam(ParamWidget::create<BigLEDBezel>(Vec(led_center, 132), module, TriggersMKII::TRIGGER_SWITCH_1, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<GiantLight<RedLight>>(Vec(led_center+led_offset, 132+led_offset), module, TriggersMKII::TRIGGER_LED_1));
    //PORTS
    addOutput(Port::create<as_PJ301MPort>(Vec(7, 78), Port::OUTPUT, module, TriggersMKII::TRIGGER_OUT1));
    addInput(Port::create<as_PJ301MPort>(Vec(7, 104), Port::INPUT, module, TriggersMKII::CV_TRIG_INPUT_1));

    //TRIGGER 2
    //LABEL DISPLAY 
	LabelDisplayWidget *display2 = new LabelDisplayWidget();
	display2->box.pos = Vec(6,50+y_offset);
	display2->box.size = Vec(78, 20);
	display2->value = &module->label_num2;
	addChild(display2); 	

    //PARAM
	addParam(ParamWidget::create<as_KnobBlack>(Vec(46, 77+y_offset), module, TriggersMKII::LABEL_PARAM_2, 0.0, 36.0, 0.0));
    //SWITCH
    addParam(ParamWidget::create<BigLEDBezel>(Vec(led_center, 132+y_offset), module, TriggersMKII::MOMENTARY_SWITCH_2, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<GiantLight<RedLight>>(Vec(led_center+led_offset, 132+led_offset+y_offset), module, TriggersMKII::MOMENTARY_LED_2));
    //PORTS
    addOutput(Port::create<as_PJ301MPort>(Vec(7, 78+y_offset), Port::OUTPUT, module, TriggersMKII::MOMENTARY_OUT2));
    addInput(Port::create<as_PJ301MPort>(Vec(7, 104+y_offset), Port::INPUT, module, TriggersMKII::CV_TRIG_INPUT_2));
    
}


RACK_PLUGIN_MODEL_INIT(AS, TriggersMKII) {
   Model *modelTriggersMKII = Model::create<TriggersMKII, TriggersMKIIWidget>("AS", "TriggersMKII", "Triggers MKII",  SWITCH_TAG, UTILITY_TAG);
   return modelTriggersMKII;
}
