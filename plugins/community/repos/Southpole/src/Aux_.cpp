
#include "Southpole.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_Southpole {

#define NUMAUX 2

struct Aux : Module {
   enum ParamIds {      
      SEND1_PARAM,
      SEND2_PARAM,
      RETURN1_PARAM,
      RETURN2_PARAM,
      FEEDBACK1_PARAM,
      FEEDBACK2_PARAM,
      MUTE_PARAM,
      BYPASS_PARAM,
      NUM_PARAMS
   };
   enum InputIds {
      INL_INPUT,
      INR_INPUT,
      RETURN1L_INPUT,
      RETURN2L_INPUT,
      RETURN1R_INPUT,
      RETURN2R_INPUT,
      NUM_INPUTS
   };
   enum OutputIds {
      OUTL_OUTPUT,
      OUTR_OUTPUT,
      SEND1L_OUTPUT,
      SEND2L_OUTPUT,
      SEND1R_OUTPUT,
      SEND2R_OUTPUT,
      NUM_OUTPUTS
   };
   enum LightIds {
      MUTE_LIGHT,
      BYPASS_LIGHT,
      NUM_LIGHTS
   };

   SchmittTrigger muteTrigger;
   SchmittTrigger bypassTrigger;
   bool mute;
   bool bypass;

   Aux() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

      mute = 0;
      bypass = 0;
   }
   void step() override;

   json_t *toJson()override {
      json_t *rootJm = json_object();
      json_t *statesJ = json_array();
      
         json_t *muteJ = json_boolean(mute);
         json_array_append_new(statesJ, muteJ);

         json_t *bypassJ = json_boolean(bypass);
         json_array_append_new(statesJ, bypassJ);
      
      json_object_set_new(rootJm, "states", statesJ);

      return rootJm;
   }

   void fromJson(json_t *rootJm)override {
      json_t *statesJ = json_object_get(rootJm, "states");
      
         json_t *muteJ   = json_array_get(statesJ, 0);
         json_t *bypassJ = json_array_get(statesJ, 1);

         mute = !!json_boolean_value(muteJ);
         bypass = !!json_boolean_value(bypassJ);
      
   }

};


void Aux::step() {

   if (muteTrigger.process(params[MUTE_PARAM].value)){
      mute = !mute;
   }
    lights[MUTE_LIGHT].value = mute ? 1.0 : 0.0;

   if (bypassTrigger.process(params[BYPASS_PARAM].value)){
      bypass = !bypass;
   }
    lights[BYPASS_LIGHT].value = bypass ? 1.0 : 0.0;

   float inl = 0.;
   float inr = 0.;

   if (!mute) {
      inl = inputs[INL_INPUT].normalize(0.);
      inr = inputs[INR_INPUT].normalize(inl);
   }

   float outl = inl;
   float outr = inr;

   float sl1 = params[SEND1_PARAM].value * inl;
   float sr1 = params[SEND1_PARAM].value * inr;

   float sl2 = params[SEND2_PARAM].value * inl;
   float sr2 = params[SEND2_PARAM].value * inr;

   float rl1 = inputs[RETURN1L_INPUT].normalize(0.);
   float rr1 = inputs[RETURN1R_INPUT].normalize(rl1);

   float rl2 = inputs[RETURN2L_INPUT].normalize(0.);
   float rr2 = inputs[RETURN2R_INPUT].normalize(rl2);

   float fb1 = params[FEEDBACK1_PARAM].value;
   float fb2 = params[FEEDBACK2_PARAM].value;

   if (fb1 >= 0.) {
      sl1 += fb1 * rl2;
      sr1 += fb1 * rr2;
   } else {
      sr1 -= fb1 * rl2;
      sl1 -= fb1 * rr2;    
   }

   if (fb2 >= 0.) {
      sl2 += fb2 * rl1;
      sr2 += fb2 * rr1;
   } else {
      sr2 -= fb2 * rl1;
      sl2 -= fb2 * rr1;    
   }

   outputs[SEND1L_OUTPUT].value = sl1;
   outputs[SEND1R_OUTPUT].value = sr1;

   outputs[SEND2L_OUTPUT].value = sl2;
   outputs[SEND2R_OUTPUT].value = sr2;

   if (!bypass) {
      outl += params[RETURN1_PARAM].value * rl1;
      outr += params[RETURN1_PARAM].value * rr1;      
      outl += params[RETURN2_PARAM].value * rl2;
      outr += params[RETURN2_PARAM].value * rr2;      
   }

   outputs[OUTL_OUTPUT].value = outl;
   outputs[OUTR_OUTPUT].value = outr;
}

struct AuxWidget : ModuleWidget { 
   AuxWidget(Aux *module) : ModuleWidget(module) {

      box.size = Vec(15*4, 380);

      {
         SVGPanel *panel = new SVGPanel();
         panel->setBackground(SVG::load(assetPlugin(plugin, "res/Aux_.svg")));
         panel->box.size = box.size;
         addChild(panel);  
      }

      const float y1 = 42;
      const float yh = 26;

      const float x1 = 4.;
      //const float x2 = 20.;
      const float x3 = 36.;



      addOutput(Port::create<sp_Port>(Vec(x1,  y1+ 0*yh), Port::OUTPUT, module, Aux::SEND1L_OUTPUT));
      addOutput(Port::create<sp_Port>(Vec(x1,  y1+ 1*yh), Port::OUTPUT, module, Aux::SEND1R_OUTPUT));
      addInput(Port::create<sp_Port>(  Vec(x3,  y1+ 0*yh), Port::INPUT, module, Aux::RETURN1L_INPUT));
      addInput(Port::create<sp_Port>(  Vec(x3,  y1+ 1*yh), Port::INPUT, module, Aux::RETURN1R_INPUT));      
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1+2*yh), module, Aux::SEND1_PARAM, 0.0, 1.0, 0.5));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x3, y1+2*yh), module, Aux::RETURN1_PARAM, 0.0, 1.0, 0.5));

      addParam(ParamWidget::create<sp_Trimpot>(Vec(x1, y1+3.5*yh), module, Aux::FEEDBACK1_PARAM, -1.0, 1.0, 0.0));
      addParam(ParamWidget::create<sp_Trimpot>(Vec(x3, y1+3.5*yh), module, Aux::FEEDBACK2_PARAM, -1.0, 1.0, 0.0));

      addOutput(Port::create<sp_Port>(Vec(x1,  y1+ 5.5*yh), Port::OUTPUT, module, Aux::SEND2L_OUTPUT));
      addOutput(Port::create<sp_Port>(Vec(x1,  y1+ 6.5*yh), Port::OUTPUT, module, Aux::SEND2R_OUTPUT));
      addInput(Port::create<sp_Port>(  Vec(x3,  y1+ 5.5*yh), Port::INPUT, module, Aux::RETURN2L_INPUT));
      addInput(Port::create<sp_Port>(  Vec(x3,  y1+ 6.5*yh), Port::INPUT, module, Aux::RETURN2R_INPUT));    
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1+7.5*yh), module, Aux::SEND2_PARAM, 0.0, 1.0, 0.5));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x3, y1+7.5*yh), module, Aux::RETURN2_PARAM, 0.0, 1.0, 0.5));


      addParam(ParamWidget::create<LEDButton>          (Vec(x1,   y1+ 9*yh    ), module, Aux::MUTE_PARAM, 0.0, 1.0, 0.0));
      addChild(ModuleLightWidget::create<LargeLight<RedLight>>(Vec(x1+2.2, y1+ 9*yh+2), module, Aux::MUTE_LIGHT));

      addParam(ParamWidget::create<LEDButton>            (Vec(x3,   y1+ 9*yh  ), module, Aux::BYPASS_PARAM, 0.0, 1.0, 0.0));
      addChild(ModuleLightWidget::create<LargeLight<RedLight>>(Vec(x3+2.2, y1+ 9*yh+2), module, Aux::BYPASS_LIGHT));

      addInput(Port::create<sp_Port>(  Vec(x1,  y1+10*yh), Port::INPUT, module, Aux::INL_INPUT));
      addInput(Port::create<sp_Port>(  Vec(x1,  y1+11*yh), Port::INPUT, module, Aux::INR_INPUT));

      addOutput(Port::create<sp_Port>(Vec(x3,  y1+10*yh), Port::OUTPUT, module, Aux::OUTL_OUTPUT));
      addOutput(Port::create<sp_Port>(Vec(x3,  y1+11*yh), Port::OUTPUT, module, Aux::OUTR_OUTPUT));
   }
};

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Aux) {
   Model *modelAux   = Model::create<Aux,AuxWidget>(      "Southpole", "Aux",    "Aux - effect loop", AMPLIFIER_TAG, MIXER_TAG);
   return modelAux;
}
