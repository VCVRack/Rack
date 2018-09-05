//**************************************************************************************
//
//BPM Clock module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Based on code taken from Master Clock Module VCV Module Strum 2017 https://github.com/Strum/Strums_Mental_VCV_Modules
//**************************************************************************************

#include "AS.hpp"
#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>

struct LFOGenerator {
	float phase = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;
	SchmittTrigger resetTrigger;
	void setFreq(float freq_to_set)
  {
    freq = freq_to_set;
  }		
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}
  void setReset(float reset) {
		if (resetTrigger.process(reset)) {
			phase = 0.0f;
		}
	}
	float sqr() {
		float sqr = phase < pw ? 1.0f : -1.0f;
		return sqr;
	}
};

struct BPMClock : Module {
	enum ParamIds {    
    TEMPO_PARAM,
    MODE_PARAM,    
    TIMESIGTOP_PARAM,
    TIMESIGBOTTOM_PARAM,
    RESET_SWITCH,
    RUN_SWITCH,    
		NUM_PARAMS
	};  
	enum InputIds { 
    RUN_CV,
    RESET_INPUT,    
		NUM_INPUTS
	};
	enum OutputIds {
		BEAT_OUT,
    EIGHTHS_OUT,
    SIXTEENTHS_OUT,
    BAR_OUT,
    RESET_OUTPUT, 
    RUN_OUTPUT,       
		NUM_OUTPUTS
	};
  enum LightIds {
		RESET_LED,
    RUN_LED,
		NUM_LIGHTS
	}; 
  
  LFOGenerator clock;
  
  SchmittTrigger eighths_trig;
	SchmittTrigger quarters_trig;
  SchmittTrigger bars_trig;
  SchmittTrigger run_button_trig;
  SchmittTrigger ext_run_trig;
	SchmittTrigger reset_btn_trig;
  SchmittTrigger reset_ext_trig;
  SchmittTrigger bpm_mode_trig;

  PulseGenerator resetPulse;
  bool reset_pulse = false;

  PulseGenerator runPulse;
  bool run_pulse = false;

  // PULSES FOR TRIGGER OUTPUTS INSTEAD OF GATES
	PulseGenerator clockPulse8s;
  bool pulse8s = false;
  PulseGenerator clockPulse4s;
  bool pulse4s = false;
	PulseGenerator clockPulse1s;
  bool pulse1s = false;
	PulseGenerator clockPulse16s;
  bool pulse16s = false;

	float trigger_length = 0.0001f;

  const float lightLambda = 0.075f;
  float resetLight = 0.0f;

  bool running = true;
  
  int eighths_count = 0;
	int quarters_count = 0;
  int bars_count = 0;
  
  float tempo =120.0f;
  int time_sig_top, time_sig_bottom = 0;
  int time_sig_bottom_old = 0;
  float frequency = 2.0f;
  int quarters_count_limit = 4;
  int eighths_count_limit = 2;
  int bars_count_limit = 16;
  
	BPMClock() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

  json_t *toJson() override{
		json_t *rootJ = json_object();
    json_t *button_statesJ = json_array();
		json_t *button_stateJ = json_integer((int)running);
		json_array_append_new(button_statesJ, button_stateJ);		
		json_object_set_new(rootJ, "run", button_statesJ);    
    return rootJ;
  }
  
  void fromJson(json_t *rootJ) override{
    json_t *button_statesJ = json_object_get(rootJ, "run");
		if (button_statesJ){			
				json_t *button_stateJ = json_array_get(button_statesJ,0);
				if (button_stateJ)
					running = !!json_integer_value(button_stateJ);			
		}  
  } 

};

void BPMClock::step() {

  if (run_button_trig.process(params[RUN_SWITCH].value) || ext_run_trig.process(inputs[RUN_CV].value)){
		  running = !running;
          runPulse.trigger(0.01f);
	}

  lights[RUN_LED].value = running ? 1.0f : 0.0f;

  run_pulse = runPulse.process(1.0 / engineGetSampleRate());
  outputs[RUN_OUTPUT].value = (run_pulse ? 10.0f : 0.0f);

  if (params[MODE_PARAM].value){
    //regular 40 to 250 bpm mode
    tempo = std::round(params[TEMPO_PARAM].value);
	}else{
    //extended 30 to 300 mode
    tempo = std::round(rescale(params[TEMPO_PARAM].value,40.0f,250.0f, 30.0f, 300.0f) );
  }
  //tempo = std::round(params[TEMPO_PARAM].value);

  time_sig_top = std::round(params[TIMESIGTOP_PARAM].value);
  time_sig_bottom = std::round(params[TIMESIGBOTTOM_PARAM].value);
  time_sig_bottom = std::pow(2,time_sig_bottom+1);
 
  frequency = tempo/60.0f;

  //RESET TRIGGER
	if(reset_ext_trig.process(inputs[RESET_INPUT].value) || reset_btn_trig.process(params[RESET_SWITCH].value)) {
    clock.setReset(1.0f);
    eighths_count = 0;
    quarters_count = 0;
    bars_count = 0;
    resetLight = 1.0;
    resetPulse.trigger(0.01f);
  }

  resetLight -= resetLight / lightLambda / engineGetSampleRate();
  lights[RESET_LED].value = resetLight;
  reset_pulse = resetPulse.process(1.0 / engineGetSampleRate());
  outputs[RESET_OUTPUT].value = (reset_pulse ? 10.0f : 0.0f);


  if(!running){

    eighths_count = 0;
    quarters_count = 0;
    bars_count = 0; 
    outputs[BAR_OUT].value = 0.0f;
    outputs[BEAT_OUT].value = 0.0f;
    outputs[EIGHTHS_OUT].value = 0.0f;
    outputs[SIXTEENTHS_OUT].value = 0.0f;

  }else{

    if (time_sig_top == time_sig_bottom){
      quarters_count_limit = 4;
      eighths_count_limit = 2;
      bars_count_limit = 16;    
      clock.setFreq(frequency*4);   
    }else{
      if(time_sig_bottom == 4){
        quarters_count_limit = 4;
        eighths_count_limit = 2;
        bars_count_limit = time_sig_top * 4;  
        clock.setFreq(frequency*4);
      }
      if(time_sig_bottom == 8){
        quarters_count_limit = 4;
        eighths_count_limit = 2;
        bars_count_limit = time_sig_top * 2;
        clock.setFreq(frequency*4);
      }
      if((time_sig_top % 3) == 0){
        quarters_count_limit = 6;
        eighths_count_limit = 2;
        bars_count_limit = (time_sig_top/3) * 6;
        clock.setFreq(frequency*6);
      }      
    }
  }
 
  if(running){
    clock.step(1.0 / engineGetSampleRate());

    //16ths
    float clock16s = clamp(10.0f * clock.sqr(), 0.0f, 10.0f);

    if(clock16s>0){
      clockPulse16s.trigger(trigger_length);
    }

    //8ths
    if (eighths_trig.process(clock.sqr()) && eighths_count <= eighths_count_limit){
      eighths_count++;
    }
    if (eighths_count >= eighths_count_limit){
      eighths_count = 0;    
    }

    if(eighths_count == 0){
      clockPulse8s.trigger(trigger_length);
    }
    //4ths
    if (quarters_trig.process(clock.sqr()) && quarters_count <= quarters_count_limit){
      quarters_count++;
    }
    if (quarters_count >= quarters_count_limit){
      quarters_count = 0;    
    }

    if(quarters_count == 0){
      clockPulse4s.trigger(trigger_length);
    }
    
    //bars
    if (bars_trig.process(clock.sqr()) && bars_count <= bars_count_limit){
      bars_count++;
    }
    if (bars_count >= bars_count_limit){
      bars_count = 0;    
    }

    if(bars_count == 0){
      clockPulse1s.trigger(trigger_length);
    }
  }

    pulse1s = clockPulse1s.process(1.0 / engineGetSampleRate());
    pulse4s = clockPulse4s.process(1.0 / engineGetSampleRate());
    pulse8s = clockPulse8s.process(1.0 / engineGetSampleRate());
    pulse16s = clockPulse16s.process(1.0 / engineGetSampleRate());
    
    outputs[BAR_OUT].value = (pulse1s ? 10.0f : 0.0f);
    outputs[BEAT_OUT].value = (pulse4s ? 10.0f : 0.0f);
    outputs[EIGHTHS_OUT].value = (pulse8s ? 10.0f : 0.0f);
    outputs[SIXTEENTHS_OUT].value = (pulse16s ? 10.0f : 0.0f);

}

////////////////////////////////////
struct BpmDisplayWidget : TransparentWidget {
  float *value;
  std::shared_ptr<Font> font;

  BpmDisplayWidget() {
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
    to_display << std::setw(3) << *value;

    Vec textPos = Vec(4.0f, 17.0f); 

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
////////////////////////////////////
struct SigDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  SigDisplayWidget() {
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
    nvgStrokeWidth(vg, 1.0);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);    
    // text 
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;   
    to_display << std::setw(2) << *value;

    Vec textPos = Vec(3.0f, 17.0f); 

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
//////////////////////////////////

struct BPMClockWidget : ModuleWidget { 
    BPMClockWidget(BPMClock *module);
};

BPMClockWidget::BPMClockWidget(BPMClock *module) : ModuleWidget(module) {

  setPanel(SVG::load(assetPlugin(plugin, "res/BPMClock.svg")));

  //SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  //BPM DISPLAY 
  BpmDisplayWidget *display = new BpmDisplayWidget();
  display->box.pos = Vec(23,45);
  display->box.size = Vec(45, 20);
  display->value = &module->tempo;
  addChild(display); 
  //TEMPO KNOB
  addParam(ParamWidget::create<as_KnobBlack>(Vec(8, 69), module, BPMClock::TEMPO_PARAM, 40.0f, 250.0f, 120.0f));
  //OLD/NEW SWITCH FROM 40-250 TO 30-300
	addParam(ParamWidget::create<as_CKSS>(Vec(67, 77), module, BPMClock::MODE_PARAM, 0.0f, 1.0f, 1.0f));
  //SIG TOP DISPLAY 
  SigDisplayWidget *display2 = new SigDisplayWidget();
  display2->box.pos = Vec(54,123);
  display2->box.size = Vec(30, 20);
  display2->value = &module->time_sig_top;
  addChild(display2);
  //SIG TOP KNOB
  addParam(ParamWidget::create<as_Knob>(Vec(8, 110), module, BPMClock::TIMESIGTOP_PARAM,2.0f, 15.0f, 4.0f));
  //SIG BOTTOM DISPLAY    
  SigDisplayWidget *display3 = new SigDisplayWidget();
  display3->box.pos = Vec(54,155);
  display3->box.size = Vec(30, 20);
  display3->value = &module->time_sig_bottom;
  addChild(display3); 
  //SIG BOTTOM KNOB
  addParam(ParamWidget::create<as_Knob>(Vec(8, 150), module, BPMClock::TIMESIGBOTTOM_PARAM,0.0f, 3.0f, 1.0f));
  //RESET & RUN LEDS
  /*
  addParam(ParamWidget::create<LEDBezel>(Vec(60.5, 202), module, BPMClock::RUN_SWITCH , 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(62.7, 204.3), module, BPMClock::RUN_LED));
 */
  addParam(ParamWidget::create<LEDBezel>(Vec(33.5, 202), module, BPMClock::RUN_SWITCH , 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(35.7, 204.3), module, BPMClock::RUN_LED));

  addParam(ParamWidget::create<LEDBezel>(Vec(33.5, 241), module, BPMClock::RESET_SWITCH , 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(35.7, 243.2), module, BPMClock::RESET_LED));
  //RESET INPUT
  addInput(Port::create<as_PJ301MPort>(Vec(6, 240), Port::INPUT, module, BPMClock::RESET_INPUT));
  //RESET OUTPUT
  addOutput(Port::create<as_PJ301MPort>(Vec(59, 240), Port::OUTPUT, module, BPMClock::RESET_OUTPUT));
  //TEMPO OUTPUTS
  addOutput(Port::create<as_PJ301MPort>(Vec(6, 280), Port::OUTPUT, module, BPMClock::BAR_OUT));
  addOutput(Port::create<as_PJ301MPort>(Vec(59, 280), Port::OUTPUT, module, BPMClock::BEAT_OUT));
  addOutput(Port::create<as_PJ301MPort>(Vec(6, 320), Port::OUTPUT, module, BPMClock::EIGHTHS_OUT));
  addOutput(Port::create<as_PJ301MPort>(Vec(59, 320), Port::OUTPUT, module, BPMClock::SIXTEENTHS_OUT));

  //RUN CV
  addInput(Port::create<as_PJ301MPort>(Vec(6, 200), Port::INPUT, module, BPMClock::RUN_CV));
  //RUN TRIGGER OUTPUT
  addOutput(Port::create<as_PJ301MPort>(Vec(59, 200), Port::OUTPUT, module, BPMClock::RUN_OUTPUT));

}

RACK_PLUGIN_MODEL_INIT(AS, BPMClock) {
   Model *modelBPMClock = Model::create<BPMClock, BPMClockWidget>("AS", "BPMClock", "BPM Clock", CLOCK_TAG);
   return modelBPMClock;
}
