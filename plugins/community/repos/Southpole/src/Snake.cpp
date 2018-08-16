#include "Southpole.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_Southpole {

#define NSNAKEBUSS  16
#define NSNAKEPORTS 10

struct Snake : Module {

   enum ParamIds {
      PLUS_PARAM,
      MINUS_PARAM,
      NUM_PARAMS
   };
   enum InputIds {      
      IN_INPUT,
      NUM_INPUTS = IN_INPUT + NSNAKEPORTS + 1
   };
   enum OutputIds {
      OUT_OUTPUT,
      NUM_OUTPUTS = OUT_OUTPUT + NSNAKEPORTS + 1
   };
   enum LightIds {
      LOCK_LIGHT,
      NUM_LIGHTS = LOCK_LIGHT + 2*NSNAKEPORTS + 2
   };

   static int   nsnakes;
   static float cable[NSNAKEBUSS][NSNAKEPORTS];
   static int   lockid[NSNAKEBUSS][NSNAKEPORTS];

   int buss = 0;
   int id;

   SchmittTrigger plusTrigger;
   SchmittTrigger minusTrigger;

   Snake() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

      // first Snake module instantiation
      if (nsnakes == 0) {
         //printf("initialize Snake system\n");    
         for (int b=0; b< NSNAKEBUSS; b++) {
            for (int i=0; i< NSNAKEPORTS; i++) {
               cable[b][i] = 0.;
               lockid[b][i] = 0;
            }
         }
      }

      nsnakes++;
      buss = 0;
      id = nsnakes;
      //dump("constructor");
   }

   ~Snake() {
      // clean up
      for (int i=0; i < NSNAKEPORTS; i++) {
         if ( lockid[buss][i] == id ) {
            lockid[buss][i] = 0;
            cable[buss][i]  = 0;
         }
      }           
      nsnakes--;
      //dump("destructor");
   }

   void step() override;

   json_t *toJson() override {
      json_t *rootJ = json_object();
      json_object_set_new(rootJ, "buss", json_integer(buss));
      return rootJ;
   }

   void fromJson(json_t *rootJ) override {
      json_t *bussJ = json_object_get(rootJ, "buss");
      if (bussJ) { buss = json_integer_value(bussJ); }
      //dump("fromJson");
   }

   void dump( const char * where="" ) {
      printf(  "%p [%s] (%d) buss %d: id %d, lockid: [ ", this, where, nsnakes, buss, id );
      for (int i=0; i< NSNAKEPORTS; i++) {
         printf("%d, ", lockid[buss][i]);
      }
      printf(" ] %f %f\n", params[PLUS_PARAM].value, params[MINUS_PARAM].value);
   }

};

int   Snake::nsnakes = 0;
float Snake::cable[NSNAKEBUSS][NSNAKEPORTS];
int   Snake::lockid[NSNAKEBUSS][NSNAKEPORTS];

void Snake::step() {

   // change buss on trigger
    if (plusTrigger.process(params[PLUS_PARAM].value)) {
      if (buss < NSNAKEBUSS-1) {       
         // free and clean up current buss
         for (int i=0; i < NSNAKEPORTS; i++) {
            if ( lockid[buss][i] == id ) {
               lockid[buss][i] = 0;
               cable[buss][i]  = 0;
            }
         }                    
         buss++;     
         //dump("plus");
       } 
    }

    if (minusTrigger.process(params[MINUS_PARAM].value)) {
      if (buss > 0) {
         // free and clean up current buss
         for (int i=0; i < NSNAKEPORTS; i++) {
            if ( lockid[buss][i] == id ) {
               lockid[buss][i] = 0;
               cable[buss][i]  = 0;
            }
         }           
         buss--;        
         //dump("minus");
      }
    }

   for (int i=0; i < NSNAKEPORTS; i++) {

      // if active try to lock input
      if ( inputs[IN_INPUT+i].active ) {
         if ( lockid[buss][i] == 0 ) {
            lockid[buss][i] = id;
            //dump("lock");
         }

         if ( lockid[buss][i] == id ) {
            cable[buss][i] = inputs[IN_INPUT+i].value;
         }
      } else if ( lockid[buss][i] == id ) {
         lockid[buss][i] = 0;
         cable[buss][i]  = 0;
         //dump("release");
      }

      // operate lights                
      if ( lockid[buss][i] == 0 ) {
         lights[LOCK_LIGHT+2*i  ].setBrightness(0);
         lights[LOCK_LIGHT+2*i+1].setBrightness(0);
      } else if ( lockid[buss][i] == id ) {
         lights[LOCK_LIGHT+2*i  ].setBrightness(1.0);
         lights[LOCK_LIGHT+2*i+1].setBrightness(0.);
      } else {
         lights[LOCK_LIGHT+2*i  ].setBrightness(0.);        
         lights[LOCK_LIGHT+2*i+1].setBrightness(1.);        
      }

      // set output
      outputs[OUT_OUTPUT+i].value = cable[buss][i];
   }

}

struct SnakeDisplay : TransparentWidget {
   Snake *module;
   std::shared_ptr<Font> font;

   SnakeDisplay() {
      font = Font::load(assetPlugin(plugin, "res/hdad-segment14-1.002/Segment14.ttf"));
   }

   void draw(NVGcontext *vg) override {

      // Background
      NVGcolor backgroundColor = nvgRGB(0x30, 0x10, 0x10);
      NVGcolor borderColor = nvgRGB(0xd0, 0xd0, 0xd0);
      nvgBeginPath(vg);
      nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
      nvgFillColor(vg, backgroundColor);
      nvgFill(vg);
      nvgStrokeWidth(vg, 1.5);
      nvgStrokeColor(vg, borderColor);
      nvgStroke(vg);

      nvgFontSize(vg, 20.);
      nvgFontFaceId(vg, font->handle);
      nvgTextLetterSpacing(vg, 2.);

      Vec textPos = Vec(5, 28);
      NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
      nvgFillColor(vg, nvgTransRGBA(textColor, 16));
      nvgText(vg, textPos.x, textPos.y, "~~~~", NULL);
      nvgFillColor(vg, textColor);
      char strbuss[4];
      sprintf(strbuss,"%1x",module->buss);
      nvgText(vg, textPos.x, textPos.y, strbuss, NULL);
   }
};

struct SnakeWidget : ModuleWidget {
   
   SnakeWidget(Snake *module) : ModuleWidget(module) {

      box.size = Vec(15*4, 380);

      {
         SVGPanel *panel = new SVGPanel();
         panel->box.size = box.size;
         panel->setBackground(SVG::load(assetPlugin(plugin, "res/Snake.svg")));
         addChild(panel);
      }

      {
         SnakeDisplay *display = new SnakeDisplay();
         display->box.pos = Vec(5., 30.);
         display->box.size = Vec(25., 34.);
         display->module = module;
         addChild(display);
      }

      addParam(ParamWidget::create<TL1105>(Vec( 40, 30 ), module, Snake::PLUS_PARAM, 0.0, 1.0, 0.0));
      addParam(ParamWidget::create<TL1105>(Vec( 40, 50 ), module, Snake::MINUS_PARAM, 0.0, 1.0, 0.0));

      float y1 = 85; 
      float yh = 26;

      for (int i=0; i< NSNAKEPORTS; i++)
      {
         float y = y1+i*yh + floor(i/5)*yh*.4;
         addInput(Port::create<sp_Port>(  Vec( 5, y), Port::INPUT, module, Snake::IN_INPUT + i));
         addOutput(Port::create<sp_Port>(Vec(34, y), Port::OUTPUT, module, Snake::OUT_OUTPUT + i));
         addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(26, y), module, Snake::LOCK_LIGHT + 2*i));
      }

   }

};

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Snake) {
   Model *modelSnake    = Model::create<Snake,SnakeWidget>(  "Southpole", "Snake",     "Snake - multicore", UTILITY_TAG);
   return modelSnake;
}
