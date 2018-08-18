#include "Autodafe.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_Autodafe {

#define FONT_FILE  assetPlugin(plugin, "res/Segment7Standard.ttf")

struct BPMClock : Module {
   enum ParamIds {
      BPM,BTNUP, BTNDWN,
      BTNUPDEC, BTNDWNDEC,
      NUM_PARAMS
   };
   enum InputIds {
      NUM_INPUTS
   };
   enum OutputIds {

      OUT_1,
      OUT_2,
      OUT_3,
      OUT_4,
      OUT_8,
      OUT_12,
      OUT_16,
      OUT_24,

      OUT_1_1,
      OUT_1_2,
      OUT_1_3,
      OUT_1_4,
      OUT_1_8,
      OUT_1_12,
      OUT_1_16,
      OUT_1_24,
      NUM_OUTPUTS
   };

   enum LightIds {
      CLOCK_LIGHT,
      
      NUM_LIGHTS
   };

   BPMClock() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
   void step();

   json_t *toJson() override {
      json_t *rootJ = json_object();

      json_t *bpmintJ = json_integer((int) bpmint);
      json_object_set_new(rootJ, "bpmint", bpmintJ);

      json_t *bpmdecJ = json_integer((int) bpmdec);
      json_object_set_new(rootJ, "bpmdec", bpmdecJ);


   
      
      return rootJ;
   }

   void fromJson(json_t *rootJ) override {
      // running
      json_t *bpmintJ = json_object_get(rootJ, "bpmint");
      if (bpmintJ)
         bpmint= json_integer_value(bpmintJ);
      json_t *bpmdecJ = json_object_get(rootJ, "bpmdec");
      if (bpmdecJ)
         bpmdec= json_integer_value(bpmdecJ);
      
   }

   void reset() override {
      bpmint=120;
      bpmdec=0;
   }

   void randomize() override {
      
   }




   float bpm;
   int bpmint=120;
   int bpmdec=0;


   SchmittTrigger btnup;
   SchmittTrigger btndwn;
   SchmittTrigger btnupdec;
   SchmittTrigger btndwndec;

   PulseGenerator pulse;

   float clock_phase = 0.f;
   uint32_t tick = UINT32_MAX;



};


void BPMClock::step() {




   if (btnup.process(params[BTNUP].value))
   { 
      if (bpmint<240.0) {
         bpmint+=1;
            
      }
      else
      {
         bpmint=0;
      }
   }



   if (btndwn.process(params[BTNDWN].value))
   { 
      if (bpmint>0) {
         bpmint-=1;
            
      }
      else
      {
         bpmint=240.0;
      }
   }






   if (btnupdec.process(params[BTNUPDEC].value))
   { 
      if (bpmdec<9) {
         bpmdec+=1;
            
      }
      else
      {
         bpmdec=0;
         bpmint+=1;
      }
   }



   if (btndwndec.process(params[BTNDWNDEC].value))
   { 
      if (bpmdec>0) {
         bpmdec-=1;
            
      }
      else
      {
         bpmdec=9;
         bpmint-=1;
      }
   }


   bpm=bpmint+bpmdec*0.1;










   //const float bpm = params[BPM].value;





   clock_phase += (bpm*4 / 60.f) / engineGetSampleRate() * 12;

   bool ticked = false;

   if(clock_phase >= 1.f) {
      ticked = true;
      if(++tick >= 1152u) tick = 0u;
      clock_phase -= 1.f;
   }

   if(ticked) {
      


      //DIVIDER
      outputs[OUT_1].value = !(tick % 48u)*5;
      outputs[OUT_2].value = !(tick % 96u)*5;
      outputs[OUT_3].value = !(tick % 144u)*5;
      outputs[OUT_4].value = !(tick % 192u)*5;
      outputs[OUT_8].value = !(tick % 384u)*5;
      outputs[OUT_12].value = !(tick % 576u)*5;
      outputs[OUT_16].value = !(tick % 768u)*5;
      outputs[OUT_24].value = !(tick % 1152u)*5;

      



      //MULTIPLIER
      outputs[OUT_1_1].value = !(tick % 48u)*5;
      outputs[OUT_1_2].value = !(tick % 24u)*5;
      outputs[OUT_1_3].value = !(tick % 36u)*5;
      outputs[OUT_1_4].value = !(tick % 12u)*5;
      outputs[OUT_1_8].value = !(tick % 6u)*5;
      outputs[OUT_1_12].value = !(tick % 4u)*5;
      outputs[OUT_1_16].value = !(tick % 3u)*5;
      outputs[OUT_1_24].value = !(tick % 2u)*5;






   } else {
      lights[CLOCK_LIGHT].value=0.0;



      //DIVIDER
      outputs[OUT_1].value = 0.f;
      outputs[OUT_2].value = 0.f;
      outputs[OUT_4].value = 0.f;
      outputs[OUT_8].value = 0.f;
      outputs[OUT_12].value = 0.f;
      outputs[OUT_16].value = 0.f;
      outputs[OUT_24].value = 0.f;





      //MULTIPLIER
      outputs[OUT_1_1].value = 0.f;
      outputs[OUT_1_2].value = 0.f;
      outputs[OUT_1_4].value = 0.f;
      outputs[OUT_1_8].value = 0.f;
      outputs[OUT_1_12].value = 0.f;
      outputs[OUT_1_16].value = 0.f;
      outputs[OUT_1_24].value = 0.f;
   }




}











struct BPMClockModelDisplay : TransparentWidget {
   int *valueint;
   int *valuedec;
   std::shared_ptr<Font> font;

   BPMClockModelDisplay() {
      font = Font::load(FONT_FILE);
   }

   void draw(NVGcontext *vg) {
      // Background
      NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
      NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
      nvgBeginPath(vg);
      nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
      nvgFillColor(vg, backgroundColor);
      nvgFill(vg);
      nvgStrokeWidth(vg, 1.0);
      nvgStrokeColor(vg, borderColor);
      nvgStroke(vg);

      nvgFontSize(vg, 24);
      nvgFontFaceId(vg, font->handle);
      nvgTextLetterSpacing(vg, 2.5);

      std::string to_displayint = std::to_string(*valueint);
      std::string to_displaydec = std::to_string(*valuedec);
      std::string to_display = to_displayint +to_displaydec;
      Vec textPos = Vec(5.0f, 27.0f);

      NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
      nvgFillColor(vg, nvgTransRGBA(textColor, 16));
      nvgText(vg, textPos.x, textPos.y, "~~~~", NULL);

      textColor = nvgRGB(0xda, 0xe9, 0x29);
      nvgFillColor(vg, nvgTransRGBA(textColor, 16));
      nvgText(vg, textPos.x, textPos.y, "\\\\\\\\", NULL);

      textColor = nvgRGB(0xf0, 0x00, 0x00);
      nvgFillColor(vg, textColor);

   
      std::string z;
      if(to_displayint.length()==1){z="00"+to_displayint+to_displaydec;}
      else if(to_displayint.length()==2){z="0"+to_displayint+to_displaydec;}
      else  {z=to_displayint+to_displaydec;}

      nvgText(vg, textPos.x, textPos.y, z.c_str(), NULL);
      //nvgText(vg, textPos.x, textPos.y, to_display.c_str(), NULL);

      nvgText(vg, 41, textPos.y, ".", NULL);
   }
};



struct BPMClockWidget : ModuleWidget {
	BPMClockWidget(BPMClock *module);
};


BPMClockWidget::BPMClockWidget(BPMClock *module) : ModuleWidget(module) {
   box.size = Vec(150, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/BPMClock.svg")));
      addChild(panel);
   }

   addChild(createScrew<ScrewSilver>(Vec(15, 0)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
   addChild(createScrew<ScrewSilver>(Vec(15, 365)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

   {
      BPMClockModelDisplay *display = new BPMClockModelDisplay();
      display->box.pos = Vec(35, 65);
      display->box.size = Vec(78, 36);
      display->valueint = &module->bpmint;
      display->valuedec = &module->bpmdec;
      addChild(display);
   }

   addChild(createLight<MediumLight<RedLight>>(Vec(70, 105), module, BPMClock::CLOCK_LIGHT));

   addParam(createParam<BtnUp>(Vec(15, 70), module, BPMClock::BTNUP, 0.0, 1.0, 0.0));
   addParam(createParam<BtnDwn>(Vec(15, 88), module, BPMClock::BTNDWN, 0.0, 1.0, 0.0));

   addParam(createParam<BtnUp>(Vec(120, 70), module, BPMClock::BTNUPDEC, 0.0, 1.0, 0.0));
   addParam(createParam<BtnDwn>(Vec(120, 88), module, BPMClock::BTNDWNDEC, 0.0, 1.0, 0.0));

   //addParam(createParam<Davies1900hBlackKnob>(Vec(27, 80), module, BPMClock::BPM, 30.0, 240.0, 120.0));

   addOutput(createOutput<PJ301MPort>(Vec(30, 115), module, BPMClock::OUT_1));
   addOutput(createOutput<PJ301MPort>(Vec(30, 145), module, BPMClock::OUT_2));
   addOutput(createOutput<PJ301MPort>(Vec(30, 175), module, BPMClock::OUT_3));
   addOutput(createOutput<PJ301MPort>(Vec(30, 205), module, BPMClock::OUT_4));
   addOutput(createOutput<PJ301MPort>(Vec(30, 235), module, BPMClock::OUT_8));
   addOutput(createOutput<PJ301MPort>(Vec(30, 265), module, BPMClock::OUT_12));
   addOutput(createOutput<PJ301MPort>(Vec(30, 295), module, BPMClock::OUT_16));
   addOutput(createOutput<PJ301MPort>(Vec(30, 325), module, BPMClock::OUT_24));


   addOutput(createOutput<PJ301MPort>(Vec(90, 115), module, BPMClock::OUT_1_1));
   addOutput(createOutput<PJ301MPort>(Vec(90, 145), module, BPMClock::OUT_1_2));
   addOutput(createOutput<PJ301MPort>(Vec(90, 175), module, BPMClock::OUT_1_3));
   addOutput(createOutput<PJ301MPort>(Vec(90, 205), module, BPMClock::OUT_1_4));
   addOutput(createOutput<PJ301MPort>(Vec(90, 235), module, BPMClock::OUT_1_8));
   addOutput(createOutput<PJ301MPort>(Vec(90, 265), module, BPMClock::OUT_1_12));
   addOutput(createOutput<PJ301MPort>(Vec(90, 295), module, BPMClock::OUT_1_16));
   addOutput(createOutput<PJ301MPort>(Vec(90, 325), module, BPMClock::OUT_1_24));
}

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

RACK_PLUGIN_MODEL_INIT(Autodafe, BPMClock) {
   return Model::create<BPMClock, BPMClockWidget>("Autodafe", "BPM Clock", "BPM Clock", UTILITY_TAG, CLOCK_TAG);
}
