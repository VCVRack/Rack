//**************************************************************************************
//TriggersMKIMKI  module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//**************************************************************************************
#include "AS.hpp"
#include "dsp/digital.hpp"
#include <sstream>
#include <iomanip>

struct TriggersMKI: Module {
    enum ParamIds {
        VOLTAGE_PARAM,
        RUN_SWITCH,
        MOMENTARY_SWITCH,
        NUM_PARAMS
    };
    enum InputIds {
        CV_RUN_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        TRIGGER_OUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        RUN_LED,
        MOMENTARY_LED,
        NUM_LIGHTS
    };

    SchmittTrigger LatchTrigger;
    SchmittTrigger LatchExtTrigger;
    SchmittTrigger BtnTrigger;
    SchmittTrigger BtnExtTrigger;

    const float lightLambda = 0.075;
    float resetLight = 0.0f;
    float volts = 0.0f;
    bool running = false;
    float display_volts = 0.0f;
    bool negative_volts = false;

    PulseGenerator triggerPulse;
    bool trg_pulse = false;

    TriggersMKI() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;


    json_t *toJson() override
    {
        json_t *rootJ = json_object();
        json_t *button_statesJ = json_array();
        json_t *button_stateJ = json_integer((int)running);
        json_array_append_new(button_statesJ, button_stateJ);		
        json_object_set_new(rootJ, "run", button_statesJ);    
        return rootJ;
    }

    void fromJson(json_t *rootJ) override
    {
        json_t *button_statesJ = json_object_get(rootJ, "run");
        if (button_statesJ)
        {			
                json_t *button_stateJ = json_array_get(button_statesJ,0);
                if (button_stateJ)
                    running = !!json_integer_value(button_stateJ);			
        }  
    }  
    
};

void TriggersMKI::step() {

    display_volts = 0.0f;

    volts = params[VOLTAGE_PARAM].value;
    display_volts = volts;
    negative_volts = false;
    if(volts< 0.0){
        negative_volts = true;
    }
    if(negative_volts){
        display_volts = - display_volts;
        //doesn't update fast enough to get rid of the negative 0 display color
        /*
        if(display_volts == -0.0){
            display_volts = 0.0;
        }
        */
    }else{
        display_volts = volts;
    }
    //LATCH TRIGGER
    //EXTERNAL TRIGGER
    if (LatchTrigger.process(params[RUN_SWITCH].value)||LatchExtTrigger.process(inputs[CV_RUN_INPUT].value)) {
        running = !running;
    }
    //INTERNAL TRIGGER
    if (running) {
        lights[RUN_LED].value = 1.0f;
        outputs[TRIGGER_OUT].value = volts;
    }else{
        lights[RUN_LED].value = 0.0f;
        outputs[TRIGGER_OUT].value = 0.0f;
    }

    //MOMENTARY TRIGGER
    //updated to use pulses
    if (BtnTrigger.process(params[MOMENTARY_SWITCH].value)) {
        resetLight = 1.0;
        if (!running) {
            triggerPulse.trigger(1e-3f);

        }
    }
    if(!running){
        trg_pulse = triggerPulse.process(1.0 / engineGetSampleRate());
        outputs[TRIGGER_OUT].value = (trg_pulse ? volts : 0.0f);
    }


    resetLight -= resetLight / lightLambda / engineGetSampleRate();
    lights[MOMENTARY_LED].value = resetLight;

}

///////////////////////////////////
struct VoltsDisplayWidget : TransparentWidget {

  float *value;
  bool *negative;
  std::shared_ptr<Font> font;

  VoltsDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) override {
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

    char display_string[10];
    sprintf(display_string,"%5.2f",*value);

    Vec textPos = Vec(3.0f, 17.0f); 

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\\\\\", NULL);

    if(*negative){
        textColor = nvgRGB(0xf0, 0x00, 0x00);
    }else{
        //textColor = nvgRGB(0x90, 0xc6, 0x3e);
        textColor = nvgRGB(0x00, 0xaf, 0x25);
    }
    
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, display_string, NULL);

  }
};
////////////////////////////////////

struct TriggersMKIWidget : ModuleWidget 
{ 
    TriggersMKIWidget(TriggersMKI *module);
};


TriggersMKIWidget::TriggersMKIWidget(TriggersMKI *module) : ModuleWidget(module) {

   setPanel(SVG::load(assetPlugin(plugin, "res/TriggersMKI.svg")));
   
	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	//VOLTS DISPLAY 
	VoltsDisplayWidget *display1 = new VoltsDisplayWidget();
	display1->box.pos = Vec(10,50);
	display1->box.size = Vec(70, 20);
	display1->value = &module->display_volts;
    display1->negative = &module->negative_volts;
	addChild(display1); 

    //PARAMS
	addParam(ParamWidget::create<as_KnobBlack>(Vec(26, 77), module, TriggersMKI::VOLTAGE_PARAM, -10.0f, 10.0f, 0.0f));
    //SWITCHES
    static const float led_offset = 3.3;
    static const float led_center = 15;
    addParam(ParamWidget::create<BigLEDBezel>(Vec(led_center, 182), module, TriggersMKI::RUN_SWITCH, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<GiantLight<RedLight>>(Vec(led_center+led_offset, 182+led_offset), module, TriggersMKI::RUN_LED));

    addParam(ParamWidget::create<BigLEDBezel>(Vec(led_center, 262), module, TriggersMKI::MOMENTARY_SWITCH, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<GiantLight<RedLight>>(Vec(led_center+led_offset, 262+led_offset), module, TriggersMKI::MOMENTARY_LED));

    //PORTS
    addInput(Port::create<as_PJ301MPort>(Vec(10, 145), Port::INPUT, module, TriggersMKI::CV_RUN_INPUT));
    addOutput(Port::create<as_PJ301MPort>(Vec(55, 145), Port::OUTPUT, module, TriggersMKI::TRIGGER_OUT));

}

RACK_PLUGIN_MODEL_INIT(AS, TriggersMKI) {
   Model *modelTriggersMKI = Model::create<TriggersMKI, TriggersMKIWidget>("AS", "TriggersMKI", "Triggers MKI", SWITCH_TAG, UTILITY_TAG);
   return modelTriggersMKI;
}
