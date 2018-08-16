#include "Southpole.hpp"

namespace rack_plugin_Southpole {

struct Ftagn : Module {
   enum ParamIds {
      NUM_PARAMS
   };
   enum InputIds {
      IN1_INPUT,
      IN2_INPUT,
      NUM_INPUTS
   };
   enum OutputIds {
      OUT1_OUTPUT,
      OUT2_OUTPUT,
      NUM_OUTPUTS
   };
   enum LightIds {
      // no lights
      NUM_LIGHTS
   };

   Ftagn() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

   void step() override;
};


void Ftagn::step() {

   outputs[OUT1_OUTPUT].value = 0.0;
   outputs[OUT2_OUTPUT].value = 0.0;
}

struct FtagnWidget : ModuleWidget { 
   
   FtagnWidget(Ftagn *module)  : ModuleWidget(module) {
 
      box.size = Vec(15*4, 380);

      {
         SVGPanel *panel = new SVGPanel();
         panel->setBackground(SVG::load(assetPlugin(plugin, "res/Ftagn.svg")));
         panel->box.size = box.size;
         addChild(panel);  
      }

      addInput(Port::create<sp_Port>(Vec( 6.,  380./2.-30.), Port::INPUT, module, Ftagn::IN1_INPUT));
      addInput(Port::create<sp_Port>(Vec( 6.,  380./2.), Port::INPUT, module, Ftagn::IN2_INPUT));

      addOutput(Port::create<sp_Port>(Vec(35.,  380./2.-30.), Port::OUTPUT, module, Ftagn::OUT1_OUTPUT));
      addOutput(Port::create<sp_Port>(Vec(35.,  380./2.), Port::OUTPUT, module, Ftagn::OUT2_OUTPUT));
   }
};

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Ftagn) {
   Model *modelFtagn    = Model::create<Ftagn,FtagnWidget>(  "Southpole", "Ftagn",     "Ftagn - no filter", FILTER_TAG);
   return modelFtagn;
}
