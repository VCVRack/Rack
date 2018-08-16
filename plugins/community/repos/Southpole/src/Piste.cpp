
#include "Southpole.hpp"
#include "dsp/digital.hpp"
#include "VAStateVariableFilter.h"

namespace rack_plugin_Southpole {

   struct Piste : Module {
      enum ParamIds {
         FREQ_PARAM,
         RESO_PARAM,
         DECAY1_PARAM,
         DECAY2_PARAM,
         SCALE1_PARAM,
         SCALE2_PARAM,
         DRIVE_PARAM,
         NUM_PARAMS
      };
      enum InputIds {
         IN_INPUT,
         DECAY1_INPUT,
         DECAY2_INPUT,
         TRIG1_INPUT,
         TRIG2_INPUT,
         SCALE1_INPUT,
         SCALE2_INPUT,
         MUTE_INPUT,
         NUM_INPUTS
      };
      enum OutputIds {
         ENV1_OUTPUT,
         ENV2_OUTPUT,
         OUT_OUTPUT,
         NUM_OUTPUTS
      };
      enum LightIds {
         DECAY1_LIGHT,
         DECAY2_LIGHT,
         NUM_LIGHTS
      };

      VAStateVariableFilter lpFilter;
      VAStateVariableFilter hpFilter;

      float env1 = 0.0;
      float env2 = 0.0;
      SchmittTrigger trigger1;
      SchmittTrigger trigger2;
      SchmittTrigger mute;

      Piste() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

         params.resize(NUM_PARAMS);
         inputs.resize(NUM_INPUTS);
         outputs.resize(NUM_OUTPUTS);
         lights.resize(NUM_LIGHTS);

         //trigger1.setThresholds(0.0, 2.0);
         //trigger1.setThresholds(0.0, 2.0);

         lpFilter.setFilterType(SVFLowpass);
         hpFilter.setFilterType(SVFHighpass);        
      }
      void step() override;

      unsigned timer;

   };



   void Piste::step() {
    
      float drive = clamp(params[DRIVE_PARAM].value, 0.0f, 1.0f);

      float freq = clamp(params[FREQ_PARAM].value, -1.0f, 1.0f);
      float reso = clamp(params[RESO_PARAM].value, 0.0f, 1.0f);

      float decay1 =          clamp(params[DECAY1_PARAM].value + inputs[DECAY1_INPUT].value / 10.0f, 0.0f, 1.0f);
      float decay2 = decay1 * clamp(params[DECAY2_PARAM].value + inputs[DECAY2_INPUT].value / 10.0f, 0.0f, 1.0f);

      float scale1 =          clamp(params[SCALE1_PARAM].value, 0.0, 1.0);
      float scale2 = scale1 * clamp(params[SCALE2_PARAM].value, 0.0, 1.0);

      bool muted = inputs[MUTE_INPUT].normalize(0.) >= 1.0;
   
      if (!muted) { 
         if (trigger1.process(inputs[TRIG1_INPUT].value)) {
            env1 = 1.;
         }
         if (trigger2.process(inputs[TRIG2_INPUT].value)) {
            env2 = 1.;
         }
      }
      const float base = 20000.0;
      const float maxTime = 1.0;

      if (decay1 < 1e-4) { env1 = 0.;
      } else {
         env1 += powf(base, 1. - decay1) / maxTime * ( - env1) / engineGetSampleRate();
      }

      if (decay2 < 1e-4) { env2 = 0.;
      } else {
         env2 += powf(base, 1. - decay2) / maxTime * ( - env2) / engineGetSampleRate();
      }

      outputs[ENV1_OUTPUT].value = 10.*scale1 * env1;
      outputs[ENV2_OUTPUT].value = 10.*scale2 * env2;

      float v = inputs[IN_INPUT].value;
    
      // DRIVE
      v = (1.-drive)*v + drive * 10.*tanhf(10.*drive*v);

      const float f0 = 261.626;     
      const float rmax = 0.9995; // Qmax = 1000

      float fout = v;

      // FILTER   
      if (freq < 0.) {

         float lp_cutoff  = f0 * powf(2.f, 8.*(freq+1.)-4.);
         lpFilter.setResonance(reso*rmax);
         lpFilter.setSampleRate(engineGetSampleRate());
         lpFilter.setCutoffFreq(lp_cutoff);
         fout  = lpFilter.processAudioSample( v, 1);

      } else if ( freq > 0.) {

         float hp_cutoff  = f0 * powf(2.f, 8.*freq-3.);
         hpFilter.setResonance(reso*rmax);
         hpFilter.setSampleRate(engineGetSampleRate());
         hpFilter.setCutoffFreq(hp_cutoff);
         fout  = hpFilter.processAudioSample( v, 1);
      }

      // VCA   
      v = fout * 10.*scale1 * env1 * (1. + 10* scale2 * env2);

      outputs[OUT_OUTPUT].value = v;

      // Lights
      lights[DECAY1_LIGHT].value = env1;
      lights[DECAY2_LIGHT].value = env2;
   }

   struct PisteWidget : ModuleWidget { 
   
      PisteWidget(Module *module)  : ModuleWidget(module) {

         box.size = Vec(15*4, 380);

         {
            SVGPanel *panel = new SVGPanel();
            panel->box.size = box.size;
            panel->setBackground(SVG::load(assetPlugin(plugin, "res/Piste.svg")));
            addChild(panel);
         }

         const float x1 = 5.; 
         const float x2 = 36.;

         const float y1 = 47.;
         const float yh = 31.;

         addInput(Port::create<sp_Port>(Vec(x1, y1), Port::INPUT, module, Piste::IN_INPUT));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1), module, Piste::DRIVE_PARAM, 0.0, 1.0, 0.0));
      
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1+1*yh), module, Piste::FREQ_PARAM, -1.0, 1.0, 0.));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+1*yh), module, Piste::RESO_PARAM, .0, 1.0, 0.));

         addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(x1+6, y1+2*yh+5), module, Piste::DECAY1_LIGHT));
         addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(x2+6, y1+2*yh+5), module, Piste::DECAY2_LIGHT));

         addInput(Port::create<sp_Port>(Vec(x1, y1+2.5*yh), Port::INPUT, module, Piste::TRIG1_INPUT));
         addInput(Port::create<sp_Port>(Vec(x2, y1+2.5*yh), Port::INPUT, module, Piste::TRIG2_INPUT));

         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1+3.5*yh), module, Piste::SCALE1_PARAM, 0.0, 1.0, .5));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+3.5*yh), module, Piste::SCALE2_PARAM, 0.0, 1.0, 1.));

         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1+4.5*yh), module, Piste::DECAY1_PARAM, 0.0, 1.0, 0.5));
         addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+4.5*yh), module, Piste::DECAY2_PARAM, 0.0, 1.0, 1.));
         addInput(Port::create<sp_Port>(Vec(x1, y1+5.25*yh), Port::INPUT, module, Piste::DECAY1_INPUT));
         addInput(Port::create<sp_Port>(Vec(x2, y1+5.25*yh), Port::INPUT, module, Piste::DECAY2_INPUT));

         addOutput(Port::create<sp_Port>(Vec(x1, y1+6.5*yh), Port::OUTPUT, module, Piste::ENV1_OUTPUT));
         addOutput(Port::create<sp_Port>(Vec(x2, y1+6.5*yh), Port::OUTPUT, module, Piste::ENV2_OUTPUT));

         addInput(Port::create<sp_Port>(Vec(0.5*(x1+x2), 7.75*yh+y1), Port::INPUT, module, Piste::MUTE_INPUT));
         addOutput(Port::create<sp_Port>(Vec(0.5*(x1+x2), y1+9*yh), Port::OUTPUT, module, Piste::OUT_OUTPUT));

      }
   };

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Piste) {
   Model *modelPiste = Model::create<Piste,PisteWidget>(  "Southpole", "Piste",     "Piste - drum processor", ENVELOPE_GENERATOR_TAG, EFFECT_TAG);
}
