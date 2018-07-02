//**************************************************************************************
//Steps module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Code taken from Dual Counter - VCV Module, Strum 2017
//**************************************************************************************

#include "AS.hpp"

#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>

struct Steps : Module {
	enum ParamIds {
    RST_BUTTON1,
    COUNT_NUM_PARAM_1,
    RST_BUTTON2,
    COUNT_NUM_PARAM_2,
    RST_BUTTON3,
    COUNT_NUM_PARAM_3,
		NUM_PARAMS
	};  
	enum InputIds {
    CLK_IN_1,
    RESET_IN_1,
    CLK_IN_2,
    RESET_IN_2,
    CLK_IN_3,
    RESET_IN_3,	  
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
    OUTPUT_2,
    OUTPUT_3,    
		NUM_OUTPUTS
	};
    enum LightIds {
		RESET_LIGHT1,
		RESET_LIGHT2,
  	RESET_LIGHT3,
		NUM_LIGHTS
	};

    SchmittTrigger clock_trigger_1;
    SchmittTrigger reset_trigger_1;
    SchmittTrigger reset_ext_trigger_1;
    int count_limit1 = 1;
    int count1 = 0;
    SchmittTrigger clock_trigger_2;
    SchmittTrigger reset_trigger_2;
    SchmittTrigger reset_ext_trigger_2;
    int count_limit_2 = 1;
    int count_2 = 0;
    SchmittTrigger clock_trigger_3;
    SchmittTrigger reset_trigger_3;
    SchmittTrigger reset_ext_trigger_3;
    int count_limit_3 = 1;
    int count_3 = 0;
    const float lightLambda = 0.075f;
    float resetLight1 = 0.0f;
    float resetLight2 = 0.0f;
    float resetLight3 = 0.0f;
  
    PulseGenerator clockPulse1;
    bool pulse1 = false;
    PulseGenerator clockPulse2;
    bool pulse2 = false;
    PulseGenerator clockPulse3;
    bool pulse3 = false;

    Steps() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

    }

	void step() override;
};


void Steps::step(){

  count_limit1 = round(params[COUNT_NUM_PARAM_1].value);
  count_limit_2 = round(params[COUNT_NUM_PARAM_2].value);
  count_limit_3 = round(params[COUNT_NUM_PARAM_3].value);
  
  bool reset1 = false;
  bool reset_2 = false;
  bool reset_3 = false;
  pulse1 = false;
  
    if (reset_trigger_1.process(params[RST_BUTTON1].value)){
        reset1 = true;
        count1 = 0;
        outputs[OUTPUT_1].value = 0; 
        resetLight1 = 1.0f;

    }
    if (reset_ext_trigger_1.process(inputs[RESET_IN_1].value)){
        reset1 = true;
        count1 = 0;
        outputs[OUTPUT_1].value = 0; 
        resetLight1 = 1.0f;

    } 

  resetLight1 -= resetLight1 / lightLambda / engineGetSampleRate();
  lights[RESET_LIGHT1].value = resetLight1;

  if (reset1 == false){
		if (clock_trigger_1.process(inputs[CLK_IN_1].value) && count1 <= count_limit1)
					count1++;	
  }
  if (count1 == count_limit1){
      clockPulse1.trigger(1e-3);
  }
  if (count1 > count_limit1){
    count1 = 0;
  }
  pulse1 = clockPulse1.process(1.0 / engineGetSampleRate());
  outputs[OUTPUT_1].value = pulse1 ? 10.0f : 0.0f;
    
  ///////////// counter 2
  if (reset_trigger_2.process(params[RST_BUTTON2].value)){
    reset_2 = true;
    count_2 = 0;
    outputs[OUTPUT_2].value = 0;
    resetLight2 = 1.0f;
  }
  if (reset_ext_trigger_2.process(inputs[RESET_IN_2].value)){
    reset_2 = true;
    count_2 = 0;
    outputs[OUTPUT_2].value = 0;
    resetLight2 = 1.0f;
  } 
  resetLight2 -= resetLight2 / lightLambda / engineGetSampleRate();
  lights[RESET_LIGHT2].value = resetLight2;

  if (reset_2 == false){
		if (clock_trigger_2.process(inputs[CLK_IN_2].value) && count_2 <= count_limit_2)
					count_2++;	
  }
  if (count_2 == count_limit_2){
    clockPulse2.trigger(1e-3);
  }
  if (count_2 > count_limit_2){
    count_2 = 0;
  }  
  pulse2 = clockPulse2.process(1.0 / engineGetSampleRate());
  outputs[OUTPUT_2].value = pulse2 ? 10.0f : 0.0f;
  ///////////// counter 3
  if (reset_trigger_3.process(params[RST_BUTTON3].value)){
    reset_3 = true;
    count_3 = 0;
    outputs[OUTPUT_3].value = 0;
    resetLight3 = 1.0f;
  }
  if (reset_ext_trigger_3.process(inputs[RESET_IN_3].value)){
    reset_3 = true;
    count_3 = 0;
    outputs[OUTPUT_3].value = 0;
    resetLight3 = 1.0f;
  }  
  resetLight3 -= resetLight3 / lightLambda / engineGetSampleRate();
  lights[RESET_LIGHT3].value = resetLight3;

  if (reset_3 == false){
		if (clock_trigger_3.process(inputs[CLK_IN_3].value) && count_3 <= count_limit_3)
					count_3++;	
  }
  if (count_3 == count_limit_3){
    clockPulse3.trigger(1e-3);
  }
  if (count_3 > count_limit_3){
    count_3 = 0;
  }  
  pulse3 = clockPulse3.process(1.0 / engineGetSampleRate());
  outputs[OUTPUT_3].value = pulse3 ? 10.0f : 0.0f;
}

///////////////////////////////////
struct NumberDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  NumberDisplayWidget() {
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
    to_display << std::right  << std::setw(2) << *value;

    Vec textPos = Vec(4.0f, 17.0f); 

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
////////////////////////////////////

struct StepsWidget : ModuleWidget 
{ 
    StepsWidget(Steps *module);
};


StepsWidget::StepsWidget(Steps *module) : ModuleWidget(module) {

   setPanel(SVG::load(assetPlugin(plugin, "res/Steps.svg"))); 
   
 //SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  // counter 1
  //COUNT DISPLAY
    NumberDisplayWidget *display1 = new NumberDisplayWidget();
    display1->box.pos = Vec(10,50);
    display1->box.size = Vec(30, 20);
    display1->value = &module->count1;
    addChild(display1);
  //STEPS DISPLAY  
    NumberDisplayWidget *display2 = new NumberDisplayWidget();
    display2->box.pos = Vec(50,50);
    display2->box.size = Vec(30, 20);
    display2->value = &module->count_limit1;
    addChild(display2);

   int group_offset = 100;

    addParam(ParamWidget::create<LEDBezel>(Vec(5, 82), module, Steps::RST_BUTTON1 , 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(5+2.2, 82+2.3), module, Steps::RESET_LIGHT1));

    addParam(ParamWidget::create<as_KnobBlack>(Vec(43, 73), module, Steps::COUNT_NUM_PARAM_1, 1.0f, 64.0f, 1.0f)); 

    addInput(Port::create<as_PJ301MPort>(Vec(3, 120), Port::INPUT, module, Steps::RESET_IN_1));
    addInput(Port::create<as_PJ301MPort>(Vec(33, 120), Port::INPUT, module, Steps::CLK_IN_1));
    addOutput(Port::create<as_PJ301MPort>(Vec(63, 120), Port::OUTPUT, module, Steps::OUTPUT_1));
  
  // counter 2
  //COUNT DISPLAY
    NumberDisplayWidget *display3 = new NumberDisplayWidget();
    display3->box.pos = Vec(10,50 + group_offset);
    display3->box.size = Vec(30, 20);
    display3->value = &module->count_2;
    addChild(display3);
  //STEPS DISPLAY  
    NumberDisplayWidget *display4 = new NumberDisplayWidget();
    display4->box.pos = Vec(50,50 + group_offset);
    display4->box.size = Vec(30, 20);
    display4->value = &module->count_limit_2;
    addChild(display4);

    addParam(ParamWidget::create<LEDBezel>(Vec(5, 82+ group_offset), module, Steps::RST_BUTTON2 , 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(5+2.2, 82+2.3+ group_offset), module, Steps::RESET_LIGHT2));

    addParam(ParamWidget::create<as_KnobBlack>(Vec(43, 73 + group_offset), module, Steps::COUNT_NUM_PARAM_2, 1.0f, 64.0f, 1.0f)); 

    addInput(Port::create<as_PJ301MPort>(Vec(3, 120 + group_offset), Port::INPUT, module, Steps::RESET_IN_2));
    addInput(Port::create<as_PJ301MPort>(Vec(33, 120 + group_offset), Port::INPUT, module, Steps::CLK_IN_2));
    addOutput(Port::create<as_PJ301MPort>(Vec(63, 120 + group_offset), Port::OUTPUT, module, Steps::OUTPUT_2));

  // counter 3
  //COUNT DISPLAY
    NumberDisplayWidget *display5 = new NumberDisplayWidget();
    display5->box.pos = Vec(10,50 + group_offset*2);
    display5->box.size = Vec(30, 20);
    display5->value = &module->count_3;
    addChild(display5);
  //STEPS DISPLAY  
    NumberDisplayWidget *display6 = new NumberDisplayWidget();
    display6->box.pos = Vec(50,50 + group_offset*2);
    display6->box.size = Vec(30, 20);
    display6->value = &module->count_limit_3;
    addChild(display6);

    addParam(ParamWidget::create<LEDBezel>(Vec(5, 82+ group_offset*2), module, Steps::RST_BUTTON3 , 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(5+2.2, 82+2.3+ group_offset*2), module, Steps::RESET_LIGHT3));

    addParam(ParamWidget::create<as_KnobBlack>(Vec(43, 73 + group_offset*2), module, Steps::COUNT_NUM_PARAM_3, 1.0f, 64.0f, 1.0f)); 

    addInput(Port::create<as_PJ301MPort>(Vec(3, 120 + group_offset*2), Port::INPUT, module, Steps::RESET_IN_3));
    addInput(Port::create<as_PJ301MPort>(Vec(33, 120 + group_offset*2), Port::INPUT, module, Steps::CLK_IN_3));
    addOutput(Port::create<as_PJ301MPort>(Vec(63, 120 + group_offset*2), Port::OUTPUT, module, Steps::OUTPUT_3));	  
}

RACK_PLUGIN_MODEL_INIT(AS, Steps) {
   Model *modelSteps = Model::create<Steps, StepsWidget>("AS", "Steps", "Steps", SWITCH_TAG, SEQUENCER_TAG, UTILITY_TAG);
   return modelSteps;
}
