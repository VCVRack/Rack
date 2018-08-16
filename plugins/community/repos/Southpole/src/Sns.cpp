#include <array>
#include "Southpole.hpp"
#include "dsp/digital.hpp"
#include "Bjorklund.hpp"

namespace rack_plugin_Southpole {

struct Sns : Module {
   enum ParamIds {
      K_PARAM,
      L_PARAM,
      R_PARAM,
      S_PARAM,
      P_PARAM,
      A_PARAM,
      CLK_PARAM,
      TRIG_PARAM,
      NUM_PARAMS
   };
   enum InputIds {
      K_INPUT,
      L_INPUT,
      R_INPUT,
      S_INPUT,
      A_INPUT,
      P_INPUT,
      CLK_INPUT,
      RESET_INPUT,
      NUM_INPUTS
   };
   enum OutputIds {
      GATE_OUTPUT,
      ACCENT_OUTPUT,
      CLK_OUTPUT,
      RESET_OUTPUT,
      NUM_OUTPUTS
   };
   enum LightIds {      
      NUM_LIGHTS
   };

   Bjorklund euclid;
   Bjorklund euclid2;

   #define MAXLEN 32

   // calculated sequence/accents
   std::vector<bool> seq0;
   std::vector<bool> acc0;

   //padded+rotated+distributed
   std::array<bool, MAXLEN> sequence;
   std::array<bool, MAXLEN> accents;

   bool calculate;
   bool from_reset;

   enum patternStyle {
      EUCLIDEAN_PATTERN,
      RANDOM_PATTERN,
      FIBONACCI_PATTERN,
      LINEAR_PATTERN,
      CANTOR_PATTERN
   } style = EUCLIDEAN_PATTERN;

   unsigned int  par_k=4; // fill
   unsigned int  par_l=10; // pattern length    
   unsigned int  par_r=1; // rotation
   unsigned int  par_p=1; // padding
   unsigned int  par_s=1; // shift
   unsigned int  par_a=3; // accent
   
   unsigned int  par_last; // checksum

   unsigned int  par_k_last;
   unsigned int  par_l_last;
   unsigned int  par_a_last; 
   

   SchmittTrigger clockTrigger;
   SchmittTrigger resetTrigger;
   PulseGenerator gatePulse;
   PulseGenerator accentPulse;
   bool gateOn;
   bool accOn;

   enum gateModes {
      TRIGGER_MODE,
      GATE_MODE,
      TURING_MODE    
   } gateMode = TRIGGER_MODE;

   unsigned int  currentStep = 0;
   unsigned int  turing = 0;

   Sns() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
   
      sequence.fill(0);
      accents.fill(0);
      reset();
   }

   void step() override;
   void reset() override;

   unsigned int fib(unsigned int n){
         return (n < 2) ? n : fib(n - 1) + fib(n - 2);
   }

   json_t *toJson() override {
      json_t *rootJ = json_object();
      json_object_set_new(rootJ, "mode", json_integer((int) gateMode));
      json_object_set_new(rootJ, "style", json_integer((int) style));
      return rootJ;
   }

   void fromJson(json_t *rootJ) override {
      json_t *modeJ = json_object_get(rootJ, "mode");
      if (modeJ) {
         gateMode = (gateModes) json_integer_value(modeJ);
      }
      json_t *styleJ = json_object_get(rootJ, "style");
      if (styleJ) {
         style = (patternStyle) json_integer_value(styleJ);
      }
   }  

};

void Sns::reset() {

   if ( par_l_last != par_l ) { 
      std::fill(seq0.begin(), seq0.end(), 0);
      seq0.resize(par_l+par_p); 
   }
   if ( par_k_last != par_k ) { 
      std::fill(acc0.begin(), acc0.end(), 0);
      acc0.resize(par_k); 
   }

   if ( style == RANDOM_PATTERN ) {
      
      if ( par_l_last != par_l || par_k_last != par_k) {
         int n = 0;
         seq0.resize(par_l);
         std::fill(seq0.begin(), seq0.end(), 0);
         unsigned int f = 0;
         while ( f < par_k ) {
            if ( randomUniform() < (float)par_k/(float)par_l ) {
               seq0.at(n % par_l) = 1;
               f++;
            }
            n++;
         }
      }
      if ( par_a && (par_a_last != par_a  || par_k_last != par_k) ) {
         int n = 0;
         acc0.resize(par_k); 
         std::fill(acc0.begin(), acc0.end(), 0);
         unsigned int nacc = 0;
         while ( nacc < par_a ) {
            if ( randomUniform() < (float)par_a/(float)par_k ) {
               acc0.at(n % par_k) = 1;
               nacc++;
            }
            n++;
         }
      }
   }

   if ( style == FIBONACCI_PATTERN ) {

      seq0.resize(par_l);
      std::fill(seq0.begin(), seq0.end(), 0);
      for ( unsigned int k = 0; k < par_k; k++ ) {
         seq0.at(fib(k) % par_l) = 1;
      }

      acc0.resize(par_k);
      std::fill(acc0.begin(), acc0.end(), 0);
      for ( unsigned int a = 0; a < par_a; a++ ) {
         acc0.at(fib(a) % par_k) = 1;
      }
   }  

// if ( style == CANTOR_PATTERN ) {
   if ( style == LINEAR_PATTERN ) {

      seq0.resize(par_l);
      std::fill(seq0.begin(), seq0.end(), 0);
      for ( unsigned int k = 0; k < par_k; k++ ) {
         seq0.at( par_l*k/par_k ) = 1;
      }

      acc0.resize(par_k);
      std::fill(acc0.begin(), acc0.end(), 0);
      for ( unsigned int a = 0; a < par_a; a++ ) {
         acc0.at( par_k*a/par_a ) = 1;
      }
   }  
   
   if ( style == EUCLIDEAN_PATTERN ) {

      euclid.reset();   
      euclid.init(par_l,par_k);
      euclid.iter();
      
      euclid2.reset();  
      if (par_a>0) {
         euclid2.init(par_k, par_a);
         euclid2.iter();
      }
      seq0 = euclid.sequence;
      acc0 = euclid2.sequence;
   }

   // pad sequence
   //if (seq0.size() != par_l+par_p) seq0.resize(par_l+par_p);
   //for (unsigned int i=par_l; i<par_p; i++) { seq0.at(i) = 0; }

   //for (unsigned int i = 0; i != seq0.size(); i++) {   std::cout << seq0[i];   }
   //std::cout << '\n';
   //for (unsigned int i = 0; i != acc0.size(); i++) {   std::cout << acc0[i];   }
   //std::cout << '\n';

   // distribute accents on sequence
   unsigned int j = par_k-par_s;
   for (unsigned int i = 0; i != seq0.size(); i++) {
      unsigned int idx = (i + par_r)%(par_l + par_p);
      sequence[idx] = seq0.at(i);
      accents[idx] = 0;
      if (par_a && seq0.at(i)) {
         accents[idx] = acc0.at( j % par_k );
         j++;
      }
   }

    //for (unsigned int i = 0; i != sequence.size(); i++) { std::cout << sequence[i];    }
    //std::cout << '\n';
    //for (unsigned int i = 0; i != accents.size(); i++) {  std::cout << accents[i];    }
    //std::cout << '\n';

      calculate = false;
   from_reset = true;
}

void Sns::step() {

   bool nextStep = false;

   // reset sequence
   if (inputs[RESET_INPUT].active) {
      if (resetTrigger.process(inputs[RESET_INPUT].value)) {
         currentStep = par_l + par_p;
      }     
       outputs[RESET_OUTPUT].value = inputs[RESET_INPUT].value; 
   }  

   if (inputs[CLK_INPUT].active) {
      if (clockTrigger.process(inputs[CLK_INPUT].value)) {
         nextStep = true;
      }
       outputs[CLK_OUTPUT].value = inputs[CLK_INPUT].value; 
   }  

   if (nextStep) {
      
      currentStep++;
      if (currentStep >= par_l + par_p) {
         currentStep = 0;
      }

      if ( gateMode == TURING_MODE ) {
         turing = 0;
         for (unsigned int i=0; i<par_l; i++) {
            turing |= sequence[(currentStep + i) % par_l+par_p];
            turing <<= 1;
         }
      } else {
         gateOn = false;
         if (sequence[currentStep] ) {
            gatePulse.trigger(1e-3);
            if ( gateMode == GATE_MODE ) {
               gateOn = true;
            }
         }
      }

      accOn = false;
      if (par_a && accents.at( currentStep )) {
         accentPulse.trigger(1e-3);
         if ( gateMode == GATE_MODE ) {
            accOn = true;
         }
      }
   }

   bool gpulse = gatePulse.process(1.0 / engineGetSampleRate());
   bool apulse = accentPulse.process(1.0 / engineGetSampleRate());

   if ( gateMode == TURING_MODE ) { 
      outputs[GATE_OUTPUT].value   = 10.0*(turing / pow(2.,par_l) - 1.);
   } else { 
      outputs[GATE_OUTPUT].value   = gateOn | gpulse ? 10.0 : 0.0;
   }
   outputs[ACCENT_OUTPUT].value = accOn | apulse ? 10.0 : 0.0;

   par_l = (unsigned int) ( 1. +   15.  * clamp( params[L_PARAM].value + inputs[L_INPUT].normalize(0.) / 9., 0.0f, 1.0f));
   par_p = (unsigned int) (32. - par_l) * clamp( params[P_PARAM].value + inputs[P_INPUT].normalize(0.) / 9., 0.0f, 1.0f);

   par_r = (unsigned int) (par_l + par_p - 1.) * clamp( params[R_PARAM].value + inputs[R_INPUT].normalize(0.) / 9., 0.0f, 1.0f);
   par_k = (unsigned int) ( 1. + (par_l-1.) * clamp( params[K_PARAM].value + inputs[K_INPUT].normalize(0.) / 9., 0.0f, 1.0f));

   par_a = (unsigned int) ( par_k ) * clamp( params[A_PARAM].value + inputs[A_INPUT].normalize(0.) / 9., 0.0f, 1.0f);
   if (par_a == 0) { 
      par_s = 0; 
   } else {
      par_s = (unsigned int) ( par_k-1. ) * clamp( params[S_PARAM].value + inputs[S_INPUT].normalize(0.) / 9., 0.0f, 1.0f);
   }

   // new sequence in case of change to parameters
   if (par_l+par_r+par_a+par_k+par_p+par_s != par_last) {
   

//    printf("%d %d",par_k,par_a);  
      par_last = par_l+par_r+par_a+par_k+par_p+par_s;
      calculate = true;
   }
}


struct SnsDisplay : TransparentWidget {

   Sns *module;
   int frame = 0;
   std::shared_ptr<Font> font;

   float y1;
   float yh;

   SnsDisplay( float y1_, float yh_ ) {
     
     y1 = y1_;
     yh = yh_;
     //font = Font::load(assetPlugin(plugin, "res/fonts/Sudo.ttf"));
     font = Font::load(assetPlugin(plugin, "res/hdad-segment14-1.002/Segment14.ttf"));
   }

   void drawPolygon(NVGcontext *vg) {
      
      Rect b = Rect(Vec(2, 2), box.size.minus(Vec(2, 2)));

      float cx = 0.5*b.size.x+1;
      float cy = 0.5*b.size.y-12;
      const float r1 = .45*b.size.x;
      const float r2 = .35*b.size.x;

      // Circles
      nvgBeginPath(vg);
      nvgStrokeColor(vg, nvgRGBA(0x7f, 0x00, 0x00, 0xff));
      nvgFillColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));    
      nvgStrokeWidth(vg, 1.);
       nvgCircle(vg, cx, cy, r1);
       nvgCircle(vg, cx, cy, r2);
      nvgStroke(vg);

      unsigned len = module->par_l + module->par_p;

      nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
      nvgBeginPath(vg);
      bool first = true;

      // inactive Step Rings
      for (unsigned i = 0; i < len; i++) {

         if ( !module->sequence[i] ) {
            float r = module->accents[i] ? r1 : r2;
            float x = cx + r * cosf(2.*M_PI*i/len-.5*M_PI);
            float y = cy + r * sinf(2.*M_PI*i/len-.5*M_PI);

            nvgBeginPath(vg);
            nvgFillColor(vg,  nvgRGBA(0x30, 0x10, 0x10, 0xff));
            nvgStrokeWidth(vg, 1.);
            nvgStrokeColor(vg, nvgRGBA(0x7f, 0x00, 0x00, 0xff));
            nvgCircle(vg, x, y, 3.);
            nvgFill(vg);
            nvgStroke(vg);
         }
      }

      // Path
      nvgBeginPath(vg);
      nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
      nvgStrokeWidth(vg, 1.);
      for (unsigned int  i = 0; i < len; i++) {

         if ( module->sequence[i] ) {
            float a = i/float(len);
            float r = module->accents[i] ? r1 : r2;
            float x = cx + r * cosf(2.*M_PI*a-.5*M_PI);
            float y = cy + r * sinf(2.*M_PI*a-.5*M_PI);

            Vec p(x,y);
            if (module->par_k == 1) nvgCircle(vg, x, y, 3.);            
            if (first) {
               nvgMoveTo(vg, p.x, p.y);
               first = false;
            } else {
               nvgLineTo(vg, p.x, p.y);
            }
         }
      }
      nvgClosePath(vg);
      nvgStroke(vg);

      // Active Step Rings
      for (unsigned i = 0; i < len; i++) {

         if ( module->sequence[i] ) {
            float r = module->accents[i] ? r1 : r2;
            float x = cx + r * cosf(2.*M_PI*i/len-.5*M_PI);
            float y = cy + r * sinf(2.*M_PI*i/len-.5*M_PI);

            nvgBeginPath(vg);
            nvgFillColor(vg,  nvgRGBA(0x30, 0x10, 0x10, 0xff));
            nvgStrokeWidth(vg, 1.);
            nvgStrokeColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
            nvgCircle(vg, x, y, 3.);
            nvgFill(vg);
            nvgStroke(vg);
         }  
      }

      unsigned int i = module->currentStep;
      float r = module->accents[i] ? r1 : r2;
      float x = cx + r * cosf(2.*M_PI*i/len-.5*M_PI);
      float y = cy + r * sinf(2.*M_PI*i/len-.5*M_PI);
      nvgBeginPath(vg);
      nvgStrokeColor(vg,   nvgRGBA(0xff, 0x00, 0x00, 0xff));
      if ( module->sequence[i] ) {
         nvgFillColor(vg,  nvgRGBA(0xff, 0x00, 0x00, 0xff));
      } else {
         nvgFillColor(vg,  nvgRGBA(0x30, 0x10, 0x10, 0xff));
      }
      nvgCircle(vg, x, y, 3.);
      nvgStrokeWidth(vg, 1.5);
      nvgFill(vg);
      nvgStroke(vg);
   }

   void draw(NVGcontext *vg) override {

      // i know ... shouldn't be here at all 
      if (module->calculate) {
         module->reset();
            module->par_k_last = module->par_k;
         module->par_l_last = module->par_l; 
         module->par_a_last = module->par_a;
      }

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

      drawPolygon(vg);

      nvgFontSize(vg, 8);
      nvgFontFaceId(vg, font->handle);

      Vec textPos = Vec(15, 105);
      NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
      nvgFillColor(vg, textColor);
      char str[20];
      snprintf(str,sizeof(str),"%2d %2d %2d",int(module->par_k),int(module->par_l),int(module->par_r));
      nvgText(vg, textPos.x, textPos.y-11, str, NULL);

      snprintf(str,sizeof(str),"%2d %2d %2d",int(module->par_p),int(module->par_a),int(module->par_s));
      nvgText(vg, textPos.x, textPos.y, str, NULL);
   }
};

struct SnsWidget : ModuleWidget { 
   SnsWidget();
   Menu *createContextMenu() override;
   
   SnsWidget(Sns *module) : ModuleWidget(module) {

      box.size = Vec(15*6, 380);

      {
         SVGPanel *panel = new SVGPanel();
         panel->box.size = box.size;
         panel->setBackground(SVG::load(assetPlugin(plugin, "res/Sns.svg")));
         addChild(panel);
      }

      {
         SnsDisplay *display = new SnsDisplay(180,30);
         display->module = module;
         display->box.pos = Vec( 3., 30);
         display->box.size = Vec(box.size.x-6., 110. );
         addChild(display);
      }

      const float y1 = 160;
      const float yh = 30;

      float x1 = 4.;
      float x2 = 4.+30;
      float x3 = 4.+60;

      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1   ), module, Sns::K_PARAM, 0., 1., .25));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1   ), module, Sns::L_PARAM, 0., 1., 1.));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x3, y1   ), module, Sns::R_PARAM, 0., 1., 0.));
      addInput(Port::create<sp_Port>(Vec(x1, y1+1*yh), Port::INPUT, module, Sns::K_INPUT));
      addInput(Port::create<sp_Port>(Vec(x2, y1+1*yh), Port::INPUT, module, Sns::L_INPUT));
      addInput(Port::create<sp_Port>(Vec(x3, y1+1*yh), Port::INPUT, module, Sns::R_INPUT));

      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1+2.5*yh), module, Sns::P_PARAM, 0., 1., 0.));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+2.5*yh), module, Sns::A_PARAM, 0., 1., 0.));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x3, y1+2.5*yh), module, Sns::S_PARAM, 0., 1., 0.));

      addInput(Port::create<sp_Port>(Vec(x1, y1+3.5*yh), Port::INPUT, module, Sns::P_INPUT));
      addInput(Port::create<sp_Port>(Vec(x2, y1+3.5*yh), Port::INPUT, module, Sns::A_INPUT));
      addInput(Port::create<sp_Port>(Vec(x3, y1+3.5*yh), Port::INPUT, module, Sns::S_INPUT));

      addInput(Port::create<sp_Port>(Vec(x1, y1+4.65*yh), Port::INPUT, module, Sns::CLK_INPUT));
      addInput(Port::create<sp_Port>(Vec(x1, y1+5.4*yh), Port::INPUT, module, Sns::RESET_INPUT));
      addOutput(Port::create<sp_Port>(Vec(x3, y1+4.65*yh), Port::OUTPUT, module, Sns::CLK_OUTPUT));
      addOutput(Port::create<sp_Port>(Vec(x3, y1+5.4*yh), Port::OUTPUT, module, Sns::RESET_OUTPUT));

      addOutput(Port::create<sp_Port>(Vec(x2, y1+4.65*yh), Port::OUTPUT, module, Sns::GATE_OUTPUT));
      addOutput(Port::create<sp_Port>(Vec(x2, y1+5.4*yh), Port::OUTPUT, module, Sns::ACCENT_OUTPUT));


      //addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(4, 281), module, Sns::CLK_LIGHT));
      //addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(4+25, 281), module, Sns::GATE_LIGHT));
      //addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(4+50, 281), module, Sns::ACCENT_LIGHT));

   }
};

struct SnsGateModeItem : MenuItem {
   Sns *sns;
   Sns::gateModes gm;
   void onAction(EventAction &e) override {
      sns->gateMode = gm;
   }
   void step() override {
      rightText = (sns->gateMode == gm) ? "✔" : "";
      MenuItem::step();
   }
};

struct SnsPatternStyleItem : MenuItem {
   Sns *sns;
   Sns::patternStyle ps;
   void onAction(EventAction &e) override {
      sns->style = ps;
      sns->reset();
   }
   void step() override {
      rightText = (sns->style == ps) ? "✔" : "";
      MenuItem::step();
   }
};

Menu *SnsWidget::createContextMenu() {
   Sns *sns = dynamic_cast<Sns*>(module);
   assert(sns);

   Menu *menu = ModuleWidget::createContextMenu();

   menu->addChild(construct<MenuLabel>());;

   menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Gate Mode"));
   menu->addChild(construct<SnsGateModeItem>(&MenuItem::text, "Trigger", &SnsGateModeItem::sns, sns, &SnsGateModeItem::gm, Sns::TRIGGER_MODE));
    menu->addChild(construct<SnsGateModeItem>(&MenuItem::text, "Gate",    &SnsGateModeItem::sns, sns, &SnsGateModeItem::gm, Sns::GATE_MODE));
   menu->addChild(construct<SnsGateModeItem>(&MenuItem::text, "Turing",  &SnsGateModeItem::sns, sns, &SnsGateModeItem::gm, Sns::TURING_MODE));

   menu->addChild(construct<MenuLabel>());;

   menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Pattern Style"));
   menu->addChild(construct<SnsPatternStyleItem>(&MenuItem::text, "Euclid",    &SnsPatternStyleItem::sns, sns, &SnsPatternStyleItem::ps, Sns::EUCLIDEAN_PATTERN));
   menu->addChild(construct<SnsPatternStyleItem>(&MenuItem::text, "Fibonacci", &SnsPatternStyleItem::sns, sns, &SnsPatternStyleItem::ps, Sns::FIBONACCI_PATTERN));
   menu->addChild(construct<SnsPatternStyleItem>(&MenuItem::text, "Random",    &SnsPatternStyleItem::sns, sns, &SnsPatternStyleItem::ps, Sns::RANDOM_PATTERN));
  //menu->addChild(construct<SnsPatternStyleItem>(&MenuItem::text, "Cantor",   &SnsPatternStyleItem::sns, sns, &SnsPatternStyleItem::ps, Sns::CANTOR_PATTERN));
   menu->addChild(construct<SnsPatternStyleItem>(&MenuItem::text, "Linear",    &SnsPatternStyleItem::sns, sns, &SnsPatternStyleItem::ps, Sns::LINEAR_PATTERN));

   return menu;
}

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Sns) {
   Model *modelSns   = Model::create<Sns,SnsWidget>(      "Southpole", "SNS",       "SNS - euclidean sequencer", SEQUENCER_TAG);
   return modelSns;
}
