//**************************************************************************************
//
//BPM Calculator module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//### BPM detect portions of code by Tomasz Sosnowski - KoralFX
//**************************************************************************************

#include "AS.hpp"

#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>


struct BPMCalc2 : Module {
	enum ParamIds {    
    TEMPO_PARAM,      
		NUM_PARAMS
	};  
	enum InputIds {
    CLOCK_INPUT,  
		NUM_INPUTS
	};
	enum OutputIds {  
    ENUMS(MS_OUTPUT, 16),   
		NUM_OUTPUTS
	};
  enum LightIds {
    CLOCK_LOCK_LIGHT,
    CLOCK_LIGHT,
		NUM_LIGHTS
	};

  //bpm detector variables
  bool inMemory = false;
  bool beatLock = false;
  float beatTime = 0.0f;
  int beatCount = 0;
  int beatCountMemory = 0;
  float beatOld = 0.0f;

  std::string tempo = "---";

  SchmittTrigger clockTrigger;
  PulseGenerator LightPulse;
  bool pulse = false;

  //calculator variables
  float bpm = 120;
  float last_bpm = 0;
  float millisecs = 60000;
  float mult = 1000;
  float millisecondsPerBeat;
  float millisecondsPerMeasure;
  float bar = 1.0f;

  float secondsPerBeat = 0.0f;
	float secondsPerMeasure = 0.0f;
  //ms variables
  float half_note_d = 1.0f;
  float half_note = 1.0f;
  float half_note_t =1.0f;

  float qt_note_d = 1.0f;
  float qt_note = 1.0f;
  float qt_note_t = 1.0f;

  float eight_note_d = 1.0f;
  float eight_note =1.0f;
  float eight_note_t = 1.0f;

  float sixth_note_d =1.0f;
  float sixth_note = 1.0f;
  float sixth_note_t = 1.0f;

  float trth_note_d = 1.0f;
  float trth_note = 1.0f;
  float trth_note_t = 1.0f;
  //hz variables
  float hz_bar = 1.0f;
  float half_hz_d = 1.0f;
  float half_hz = 1.0f;
  float half_hz_t = 1.0f;

  float qt_hz_d = 1.0f;
  float qt_hz = 1.0f;
  float qt_hz_t = 1.0f;

  float eight_hz_d = 1.0f;
  float eight_hz = 1.0f;
  float eight_hz_t = 1.0f;

  float sixth_hz_d = 1.0f;
  float sixth_hz = 1.0f;
  float sixth_hz_t = 1.0f;

  float trth_hz_d = 1.0f;
  float trth_hz = 1.0f;
  float trth_hz_t = 1.0f;


  void calculateValues(float bpm){ 

        millisecondsPerBeat = millisecs/bpm;
        millisecondsPerMeasure = millisecondsPerBeat * 4;

        secondsPerBeat = 60 / bpm;
        secondsPerMeasure = secondsPerBeat * 4;

        bar = (millisecondsPerMeasure);

        half_note_d = ( millisecondsPerBeat * 3 );
        half_note = ( millisecondsPerBeat * 2 );
        half_note_t = ( millisecondsPerBeat * 2 * 2 / 3 );

        qt_note_d = ( millisecondsPerBeat / 2 ) * 3;
        qt_note = millisecondsPerBeat;
        qt_note_t = ( millisecondsPerBeat * 2 ) / 3;

        eight_note_d = ( millisecondsPerBeat / 4 ) * 3;
        eight_note = millisecondsPerBeat / 2;
        eight_note_t = millisecondsPerBeat / 3;

        sixth_note_d = ( millisecondsPerBeat / 4 ) * 1.5;
        sixth_note = millisecondsPerBeat / 4;
        sixth_note_t = millisecondsPerBeat / 6;

        trth_note_d = ( millisecondsPerBeat / 8 ) * 1.5;
        trth_note = millisecondsPerBeat / 8;
        trth_note_t = millisecondsPerBeat / 8 * 2 / 3;
        //hz measures
        hz_bar = (1/secondsPerMeasure);

        half_hz_d = mult / half_note_d;
        half_hz = mult / half_note;
        half_hz_t = mult / half_note_t;

        qt_hz_d = mult / qt_note_d;
        qt_hz = mult / qt_note;
        qt_hz_t = mult / qt_note_t;

        eight_hz_d = mult / eight_note_d;
        eight_hz = mult / eight_note;
        eight_hz_t = mult / eight_note_t;

        sixth_hz_d = mult / sixth_note_d;
        sixth_hz = mult / sixth_note;
        sixth_hz_t = mult / sixth_note_t;

        trth_hz_d = mult / trth_note_d;
        trth_hz = mult / trth_note;
        trth_hz_t = mult / trth_note_t;

         last_bpm = bpm;
        //seems like round calcs are not really needed:
        /*
        half_note_d = std::round(millisecondsPerBeat * 3 * mult)/mult;
        half_note = std::round(millisecondsPerBeat * 2 * mult)/mult;
        half_note_t = std::round(millisecondsPerBeat * 2 * 2 / 3 * mult)/mult;
        */
  }

  void refreshDetector() {

      inMemory = false;
      beatLock = false;
      beatTime = 0.0f;
      beatCount = 0;
      beatCountMemory = 0;
      beatOld = 0.0f;
      tempo = "---";
      pulse = false;

	}

  
	BPMCalc2() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

  void onReset() override {

    refreshDetector();

	}

	void onInitialize() {

    refreshDetector();

	}

};

void BPMCalc2::step() {

  //BPM detection code
  float deltaTime = engineGetSampleTime();

  if ( inputs[CLOCK_INPUT].active ) {

    float clockInput = inputs[CLOCK_INPUT].value;
    //A rising slope
    if ( ( clockTrigger.process( inputs[CLOCK_INPUT].value ) ) && !inMemory ) {
      beatCount++;
      if(!beatLock){
        lights[CLOCK_LIGHT].value = 1.0f;
         LightPulse.trigger( 0.1f );
      }
      inMemory = true;
     
      //BPM is locked
      if ( beatCount == 2 ) {
        lights[CLOCK_LOCK_LIGHT].value = 1.0f;
        beatLock = true;
        beatOld = beatTime;
      }
      //BPM is lost
      if ( beatCount > 2 ) {

        if ( fabs( beatOld - beatTime ) > 0.0005f ) {
          beatLock = false;
          beatCount = 0;
          lights[CLOCK_LOCK_LIGHT].value = 0.0f;
          tempo = "---";
        }

      }

      beatTime = 0;

    }

    //Falling slope
    if ( clockInput <= 0 && inMemory ) {
      inMemory = false;
    }
    //When BPM is locked
    if ( beatLock ) {
      bpm = (int)round( 60 / beatOld );
      tempo = std::to_string( (int)round(bpm) );
      if(bpm!=last_bpm){
        if(bpm<999){
          calculateValues(bpm);
        }else{
          tempo = "OOR";
        }
      }
   
    } //end of beatLock routine

    beatTime += deltaTime;

    //when beat is lost
    if ( beatTime > 2 ) {
      beatLock = false;
      beatCount = 0;
      lights[CLOCK_LOCK_LIGHT].value = 0.0f;
      tempo = "---";
    }
    beatCountMemory = beatCount;

  } else {
    beatLock = false;
    beatCount = 0;
    //tempo = "OFF";
    lights[CLOCK_LOCK_LIGHT].value = 0.0f;
    //caluculate with knob value instead of bmp detector value
    bpm = params[TEMPO_PARAM].value;
    if (bpm<30){
      bpm = 30;
    }
    bpm = (int)round(bpm);
    tempo = std::to_string( (int)round(bpm) );
    if(bpm!=last_bpm){
        calculateValues(bpm);
    }

  }

  pulse = LightPulse.process( 1.0 / engineGetSampleRate() );
  lights[CLOCK_LIGHT].value = (pulse ? 1.0f : 0.0f);

  //OUTPUTS: MS to 10V scaled values
  outputs[MS_OUTPUT+0].value = rescale(bar,0.0f,10000.0f,0.0f,10.0f);
  outputs[MS_OUTPUT+1].value = rescale(half_note_d,0.0f,10000.0f,0.0f,10.0f);

  outputs[MS_OUTPUT+2].value = rescale(half_note,0.0f,10000.0f,0.0f,10.0f);
  outputs[MS_OUTPUT+3].value = rescale(half_note_t,0.0f,10000.0f,0.0f,10.0f);

  outputs[MS_OUTPUT+4].value = rescale(qt_note_d,0.0f,10000.0f,0.0f,10.0f);
  outputs[MS_OUTPUT+5].value = rescale(qt_note,0.0f,10000.0f,0.0f,10.0f);

  outputs[MS_OUTPUT+6].value = rescale(qt_note_t,0.0f,10000.0f,0.0f,10.0f);
  outputs[MS_OUTPUT+7].value = rescale(eight_note_d,0.0f,10000.0f,0.0f,10.0f);

  outputs[MS_OUTPUT+8].value = rescale(eight_note,0.0f,10000.0f,0.0f,10.0f);
  outputs[MS_OUTPUT+9].value = rescale(eight_note_t,0.0f,10000.0f,0.0f,10.0f);

  outputs[MS_OUTPUT+10].value = rescale(sixth_note_d,0.0f,10000.0f,0.0f,10.0f);
  outputs[MS_OUTPUT+11].value = rescale(sixth_note,0.0f,10000.0f,0.0f,10.0f);

  outputs[MS_OUTPUT+12].value = rescale(sixth_note_t,0.0f,10000.0f,0.0f,10.0f);
  outputs[MS_OUTPUT+13].value = rescale(trth_note_d,0.0f,10000.0f,0.0f,10.0f);

  outputs[MS_OUTPUT+14].value = rescale(trth_note,0.0f,10000.0f,0.0f,10.0f);
  
  outputs[MS_OUTPUT+15].value = rescale(trth_note_t,0.0f,10000.0f,0.0f,10.0f);


}

////////////////////////////////////
struct TempodisplayWidget : TransparentWidget {
 std::string *value;
  std::shared_ptr<Font> font;

  TempodisplayWidget() {
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
    to_display << std::setw(3) << *value;

    Vec textPos = Vec(14.0f, 17.0f); 

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


//////////////////////////////////

struct BPMCalc2Widget : ModuleWidget { 
    BPMCalc2Widget(BPMCalc2 *module);
};

BPMCalc2Widget::BPMCalc2Widget(BPMCalc2 *module) : ModuleWidget(module) {

  setPanel(SVG::load(assetPlugin(plugin, "res/BPMCalc2.svg")));

  //SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  //BPM DETECTOR PORT
  addInput(Port::create<as_PJ301MPort>(Vec(7, 53), Port::INPUT, module, BPMCalc2::CLOCK_INPUT));

  //BPM DISPLAY 
  TempodisplayWidget *display = new TempodisplayWidget();
  display->box.pos = Vec(55,54);
  display->box.size = Vec(55, 20);
  display->value = &module->tempo;
  addChild(display);
  //DETECTOR LEDS
  addChild(ModuleLightWidget::create<DisplayLedLight<RedLight>>(Vec(57, 56), module, BPMCalc2::CLOCK_LOCK_LIGHT));
  addChild(ModuleLightWidget::create<DisplayLedLight<RedLight>>(Vec(57, 66), module, BPMCalc2::CLOCK_LIGHT)); 
  //TEMPO KNOB
  addParam(ParamWidget::create<as_KnobBlack>(Vec(45, 84), module, BPMCalc2::TEMPO_PARAM, 30.0f, 300.0f, 120.0f));

  //MS outputs
  int const out_offset = 40;
  // 1 
  addOutput(Port::create<as_PJ301MPort>(Vec(84, 126), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 0));
  //·1/2 - 1/2 - t1/2
  addOutput(Port::create<as_PJ301MPort>(Vec(8, 126+out_offset*1), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 1));
  addOutput(Port::create<as_PJ301MPort>(Vec(48, 126+out_offset*1), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 2));
  addOutput(Port::create<as_PJ301MPort>(Vec(84, 126+out_offset*1), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 3));
  // ·1/4 - 1/4 -  t1/4
  addOutput(Port::create<as_PJ301MPort>(Vec(8, 126+out_offset*2), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 4));
  addOutput(Port::create<as_PJ301MPort>(Vec(48, 126+out_offset*2), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 5));
  addOutput(Port::create<as_PJ301MPort>(Vec(84, 126+out_offset*2), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 6));

  // ·1/8 - 1/8 - t1/8
 addOutput(Port::create<as_PJ301MPort>(Vec(8, 126+out_offset*3), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 7));
  addOutput(Port::create<as_PJ301MPort>(Vec(48, 126+out_offset*3), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 8));
  addOutput(Port::create<as_PJ301MPort>(Vec(84, 126+out_offset*3), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 9));
  // ·1/16 - 1/16
  addOutput(Port::create<as_PJ301MPort>(Vec(8, 126+out_offset*4), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 10));
  addOutput(Port::create<as_PJ301MPort>(Vec(48, 126+out_offset*4), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 11));

  // t1/16 - ·1/32
  addOutput(Port::create<as_PJ301MPort>(Vec(84, 126+out_offset*4), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 12));
  addOutput(Port::create<as_PJ301MPort>(Vec(8, 126+out_offset*5), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 13));
  
  // 1/32 - t1/32
  addOutput(Port::create<as_PJ301MPort>(Vec(48, 126+out_offset*5), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 14));
  addOutput(Port::create<as_PJ301MPort>(Vec(84, 126+out_offset*5), Port::OUTPUT, module, BPMCalc2::MS_OUTPUT + 15));
  

}

RACK_PLUGIN_MODEL_INIT(AS, BPMCalc2) {
   Model *modelBPMCalc2 = Model::create<BPMCalc2, BPMCalc2Widget>("AS", "BPMCalc2", "BPM to Delay MS Calculator Compact", UTILITY_TAG);
   return modelBPMCalc2;
}
