#include <iostream>
#include <stdlib.h>

#include "Southpole.hpp"
#include "dsp/digital.hpp"
#include "VAStateVariableFilter.h"

namespace rack_plugin_Southpole {

   struct Etagere : Module {
      enum ParamIds {
         FREQ1_PARAM,
         FREQ2_PARAM,
         FREQ3_PARAM,
         FREQ4_PARAM,
         GAIN1_PARAM,
         GAIN2_PARAM,
         GAIN3_PARAM,
         GAIN4_PARAM,
         Q2_PARAM,
         Q3_PARAM,
         NUM_PARAMS
      };
      enum InputIds {
         FREQ1_INPUT,
         FREQ2_INPUT,
         FREQ3_INPUT,
         FREQ4_INPUT,
         FREQ5_INPUT,
         GAIN1_INPUT,
         GAIN2_INPUT,
         GAIN3_INPUT,
         GAIN4_INPUT,
         GAIN5_INPUT,
         Q2_INPUT,
         Q3_INPUT,
         IN_INPUT,
         NUM_INPUTS
      };
      enum OutputIds {
         LP_OUTPUT,
         BP2_OUTPUT,
         BP3_OUTPUT,
         HP_OUTPUT,
         OUT_OUTPUT,
         NUM_OUTPUTS
      };
      enum LightIds {
         CLIP1_LIGHT,
         CLIP2_LIGHT,
         CLIP3_LIGHT,
         CLIP4_LIGHT,
         CLIP5_LIGHT,
         NUM_LIGHTS
      };

      bool blanc;

      VAStateVariableFilter lpFilter;
      VAStateVariableFilter bp2Filter;
      VAStateVariableFilter bp3Filter;
      VAStateVariableFilter hpFilter;

      Etagere() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {

         params.resize(NUM_PARAMS);
         inputs.resize(NUM_INPUTS);
         outputs.resize(NUM_OUTPUTS);
         lights.resize(NUM_LIGHTS);

         lpFilter.setFilterType(SVFLowpass);
         hpFilter.setFilterType(SVFHighpass);        
         bp2Filter.setFilterType(SVFBandpass);
         bp3Filter.setFilterType(SVFBandpass);

         blanc = true;   
      }

      void step() override;

      json_t *toJson() override {
         json_t *rootJ = json_object();
         json_object_set_new(rootJ, "blanc", json_boolean(blanc));
         return rootJ;
      }

      void fromJson(json_t *rootJ) override {
         json_t *blancJ = json_object_get(rootJ, "blanc");
         if (blancJ) {
            blanc = json_boolean_value(blancJ);
         }
      }

   };

   void Etagere::step() {

      float g_gain   = clamp(inputs[GAIN5_INPUT].normalize(0.), -1.0, 1.0);
      float gain1 = clamp(g_gain + params[GAIN1_PARAM].value + inputs[GAIN1_INPUT].normalize(0.) / 10.0, -1.0f, 1.0f);
      float gain2 = clamp(g_gain + params[GAIN2_PARAM].value + inputs[GAIN2_INPUT].normalize(0.) / 10.0, -1.0f, 1.0f);
      float gain3 = clamp(g_gain + params[GAIN3_PARAM].value + inputs[GAIN3_INPUT].normalize(0.) / 10.0, -1.0f, 1.0f);
      float gain4 = clamp(g_gain + params[GAIN4_PARAM].value + inputs[GAIN4_INPUT].normalize(0.) / 10.0, -1.0f, 1.0f);

      float g_cutoff = clamp(inputs[FREQ5_INPUT].normalize(0.), -4.0, 6.0);
      float freq1 = clamp(g_cutoff + params[FREQ1_PARAM].value + inputs[FREQ1_INPUT].normalize(0.), -4.0f, 6.0f);
      float freq2 = clamp(g_cutoff + params[FREQ2_PARAM].value + inputs[FREQ2_INPUT].normalize(0.), -4.0f, 6.0f);
      float freq3 = clamp(g_cutoff + params[FREQ3_PARAM].value + inputs[FREQ3_INPUT].normalize(0.), -4.0f, 6.0f);
      float freq4 = clamp(g_cutoff + params[FREQ4_PARAM].value + inputs[FREQ4_INPUT].normalize(0.), -4.0f, 6.0f);

      float reso2 = clamp(g_cutoff + params[Q2_PARAM].value + inputs[Q3_INPUT].normalize(0.) / 10.0, 0.0f, 1.0f);
      float reso3 = clamp(g_cutoff + params[Q3_PARAM].value + inputs[Q3_INPUT].normalize(0.) / 10.0, 0.0f, 1.0f);

      lpFilter.setQ(.5); //Resonance(.5);
      hpFilter.setQ(.5); //Resonance(.5);

      lpFilter.setSampleRate(engineGetSampleRate());
      hpFilter.setSampleRate(engineGetSampleRate());
      bp2Filter.setSampleRate(engineGetSampleRate());
      bp3Filter.setSampleRate(engineGetSampleRate());

      // For reference characteristics quoted from Shelves manual:
      //
      // Correction frequency range: 20 Hz to 20kHz.
      // Correction frequency CV scale: 1V/Oct.
      // Cut/boost range: -15dB to 15dB (knob), -40dB to 15dB (CV).
      // Cut/boost CV scale: 3dB/V.
      // Parametric correction Q: 0.5 to 20 (up to 1000 with external CV offset).

      float dry = inputs[IN_INPUT].value;

      //const float fmin = 16.4;
      const float f0 = 261.626;
      //const float fmax = log2f(16000./f0);
      
      const float rmax = 0.9995; // Qmax = 1000
      //const float rmax = 0.975; // Qmax = 20

      float lp_cutoff  = f0 * powf(2.f, freq1);
      float bp2_cutoff = f0 * powf(2.f, freq2);
      float bp3_cutoff = f0 * powf(2.f, freq3);
      float hp_cutoff  = f0 * powf(2.f, freq4);

      lpFilter.setCutoffFreq(lp_cutoff);
      //lpFilter.setResonance( params[Q2_PARAM].value );

      bp2Filter.setCutoffFreq(bp2_cutoff);
      bp2Filter.setResonance( rmax*reso2 );

      bp3Filter.setCutoffFreq(bp3_cutoff);
      bp3Filter.setResonance( rmax*reso3 );

      hpFilter.setCutoffFreq(hp_cutoff);
      //hpFilter.setResonance( params[Q3_PARAM].value );

      float lpout  = lpFilter.processAudioSample(dry, 1);
      float bp2out = bp2Filter.processAudioSample(dry, 1);
      float bp3out = bp3Filter.processAudioSample(dry, 1);
      float hpout  = hpFilter.processAudioSample(dry, 1);

/*
  timer++;
  if (timer > engineGetSampleRate()/2.) {
  timer = 0;
  printf("%f %f %f %f\n", lp_cutoff, bp2_cutoff, bp3_cutoff, hp_cutoff);
  }
*/
      float lpgain  = pow(20.,-gain1); 
      float bp2gain = pow(20.,-gain2); 
      float bp3gain = pow(20.,-gain3); 
      float hpgain  = pow(20.,-gain4);

      outputs[LP_OUTPUT].value  = lpout*lpgain;
      outputs[BP2_OUTPUT].value = bp2out*bp2gain;
      outputs[BP3_OUTPUT].value = bp3out*bp3gain;
      outputs[HP_OUTPUT].value  = hpout*hpgain;
       
      float sumout = lpout*lpgain + hpout*hpgain +  bp2out*bp2gain + bp3out*bp3gain;

      outputs[OUT_OUTPUT].value = sumout;
 
      // Lights
      lights[CLIP5_LIGHT].setBrightnessSmooth( fabs(sumout) > 10. ? 1. : 0. );    
   }

   struct EtagereWidget : ModuleWidget { 

      SVGPanel *blancPanel;
      SVGPanel *noirPanel; 

      void step() override;
      Menu *createContextMenu() override;

      EtagereWidget(Etagere *module)  : ModuleWidget(module) {

         box.size = Vec(15*6, 380);

         {
            noirPanel = new SVGPanel();
            noirPanel->setBackground(SVG::load(assetPlugin(plugin, "res/Etagere.svg")));
            noirPanel->box.size = box.size;
            addChild(noirPanel); 
         }
         {
            blancPanel = new SVGPanel();
            blancPanel->setBackground(SVG::load(assetPlugin(plugin, "res/Etagere_blanc.svg")));
            blancPanel->box.size = box.size;
            addChild(blancPanel);   
         }

         const float x1 = 8;
         const float x15 = 32;
         const float x2 = 65;
        
         const float y1 = 5.;
         const float yh = 25.;

         const float vfmin = -4.;
         const float vfmax =  6.;

         const float gmax = -1.;
         const float gmin =  1.;

         // TO DO possible default freqs: 880, 5000

         addInput(Port::create<sp_Port>(Vec(x1, y1+ 1* yh), Port::INPUT, module, Etagere::FREQ4_INPUT));
         addInput(Port::create<sp_Port>(Vec(x1, y1+ 2* yh), Port::INPUT, module, Etagere::GAIN4_INPUT));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+ 1*yh), module, Etagere::FREQ4_PARAM, vfmin, vfmax, 0.));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+ 2*yh), module, Etagere::GAIN4_PARAM,  gmin,  gmax, 0.));
         addOutput(Port::create<sp_Port>(Vec(x2, y1+0*yh), Port::OUTPUT, module, Etagere::HP_OUTPUT + 0));

         addInput(Port::create<sp_Port>(Vec(x1, y1+ 3* yh), Port::INPUT, module, Etagere::FREQ2_INPUT));
         addInput(Port::create<sp_Port>(Vec(x1, y1+ 4* yh), Port::INPUT, module, Etagere::GAIN2_INPUT));
         addInput(Port::create<sp_Port>(Vec(x1, y1+ 5* yh), Port::INPUT, module, Etagere::Q2_INPUT));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+ 3*yh), module, Etagere::FREQ2_PARAM, vfmin, vfmax, 0.));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+ 4*yh), module, Etagere::GAIN2_PARAM,  gmin,  gmax, 0.));
         addParam(ParamWidget::create<sp_Trimpot>(Vec(x15, y1+ 5*yh), module, Etagere::Q2_PARAM,      0.0,   1.0, 0.));
         addOutput(Port::create<sp_Port>(Vec(x2, y1+5*yh), Port::OUTPUT, module, Etagere::BP2_OUTPUT));
        
         addInput(Port::create<sp_Port>(Vec(x1, y1+ 6* yh), Port::INPUT, module, Etagere::FREQ3_INPUT));
         addInput(Port::create<sp_Port>(Vec(x1, y1+ 7* yh), Port::INPUT, module, Etagere::GAIN3_INPUT));
         addInput(Port::create<sp_Port>(Vec(x1, y1+ 8* yh), Port::INPUT, module, Etagere::Q3_INPUT));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+ 6*yh), module, Etagere::FREQ3_PARAM, vfmin, vfmax, 0.));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+ 7*yh), module, Etagere::GAIN3_PARAM,  gmin,  gmax, 0.));
         addParam(ParamWidget::create<sp_Trimpot>(Vec(x15, y1+ 8*yh), module, Etagere::Q3_PARAM,      0.0,   1.0, 0.));
         addOutput(Port::create<sp_Port>(Vec(x2, y1+8*yh), Port::OUTPUT, module, Etagere::BP3_OUTPUT));
        
         addInput(Port::create<sp_Port>(Vec(x1, y1+ 9* yh), Port::INPUT, module, Etagere::FREQ1_INPUT));
         addInput(Port::create<sp_Port>(Vec(x1, y1+10* yh), Port::INPUT, module, Etagere::GAIN1_INPUT));    
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+ 9*yh), module, Etagere::FREQ1_PARAM, vfmin, vfmax, 0.));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+10*yh), module, Etagere::GAIN1_PARAM,  gmin,  gmax, 0.));
         addOutput(Port::create<sp_Port>(Vec(x2, y1+11*yh), Port::OUTPUT, module, Etagere::LP_OUTPUT + 0));

        
         addInput(Port::create<sp_Port>(Vec(x1, y1+11* yh), Port::INPUT, module, Etagere::FREQ5_INPUT));
         addInput(Port::create<sp_Port>(Vec(x1, y1+12* yh), Port::INPUT, module, Etagere::GAIN5_INPUT));    
        
         addInput(Port::create<sp_Port>(  Vec(x1, y1+13* yh), Port::INPUT, module, Etagere::IN_INPUT));    
         addOutput(Port::create<sp_Port>(Vec(x2, y1+13*yh), Port::OUTPUT, module, Etagere::OUT_OUTPUT));

         addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(x2+10., y1+12.5*yh), module, Etagere::CLIP5_LIGHT));
      }
   };

   void EtagereWidget::step() {
      Etagere *m = dynamic_cast<Etagere*>(module);
      assert(m);
      blancPanel->visible = !m->blanc;
      noirPanel->visible = m->blanc;
      ModuleWidget::step();
   }

   struct EtagereBlancItem : MenuItem {
      Etagere *m;
      void onAction(EventAction &e) override {
         m->blanc ^= true;
      }
      void step() override {
         rightText = (!m->blanc) ? "✔" : "";
         MenuItem::step();
      }
   };

   Menu *EtagereWidget::createContextMenu() {
      Menu *menu = ModuleWidget::createContextMenu();
      Etagere *m = dynamic_cast<Etagere*>(module);
      assert(m);
      menu->addChild(construct<MenuLabel>());
      menu->addChild(construct<EtagereBlancItem>(&MenuItem::text, "blanc", &EtagereBlancItem::m, m));
      return menu;
   }

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Etagere) {
   Model *modelEtagere = Model::create<Etagere,EtagereWidget>(  "Southpole", "Etagere",   "Etagere - EQ", FILTER_TAG);
   return modelEtagere;
}
