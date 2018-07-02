/*
Copyright (c) 2017 Leonardo Laguna Ruiz (modlfo@gmail.com), All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1.- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2.- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.- Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
4.- Commercial use requires explicit permission of the author.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "VultModules.hpp"
#include "util/math.hpp"
#include "VultEngine.h"

struct Splie : Module
{
   enum ParamIds
   {
      NUM_PARAMS
   };
   enum InputIds
   {
      IN_A,
      IN_B,
      NUM_INPUTS
   };
   enum OutputIds
   {
      OUT1_A,
      OUT2_A,
      OUT3_A,
      OUT1_B,
      OUT2_B,
      OUT3_B,
      NUM_OUTPUTS
   };

   Splie();
   void step() override;
};

Splie::Splie() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
{
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
}

void Splie::step()
{

   float in_a = inputs[IN_A].value;
   float in_b = inputs[IN_B].value;

   outputs[OUT1_A].value = in_a;
   outputs[OUT2_A].value = in_a;
   outputs[OUT3_A].value = in_a;

   outputs[OUT1_B].value = in_b;
   outputs[OUT2_B].value = in_b;
   outputs[OUT3_B].value = in_b;
}

struct SplieWidget : ModuleWidget
{
   SplieWidget(Splie *module);
};

SplieWidget::SplieWidget(Splie *module) : ModuleWidget(module)
{
   // Splie *module = new Splie();
   // setModule__deprecated__(module);
   box.size = Vec(45, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Splie.svg")));
      addChild(panel);
   }
   addChild(createScrew<VultScrew>(Vec(15, 0)));
   addChild(createScrew<VultScrew>(Vec(15, 365)));

   addInput(createInput<VultJack>(Vec(11, 43), module, Splie::IN_A));
   addOutput(createOutput<VultJack>(Vec(11, 80), module, Splie::OUT1_A));
   addOutput(createOutput<VultJack>(Vec(11, 117), module, Splie::OUT2_A));
   addOutput(createOutput<VultJack>(Vec(11, 154), module, Splie::OUT3_A));

   addInput(createInput<VultJack>(Vec(11, 186), module, Splie::IN_B));
   addOutput(createOutput<VultJack>(Vec(11, 223), module, Splie::OUT1_B));
   addOutput(createOutput<VultJack>(Vec(11, 260), module, Splie::OUT2_B));
   addOutput(createOutput<VultJack>(Vec(11, 298), module, Splie::OUT3_B));
}

RACK_PLUGIN_MODEL_INIT(VultModules, Splie) {
   Model *model = Model::create<Splie, SplieWidget>("VultModules", "Splie", "Splie", MULTIPLE_TAG);
   return model;
}
