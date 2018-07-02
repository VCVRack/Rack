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

struct Tangents : Module
{
   enum ParamIds
   {
      CUTOFF_PARAM,
      RESONANCE_PARAM,
      CUTOFF_AMT_PARAM,
      RESONANCE_AMT_PARAM,
      NUM_PARAMS
   };
   enum InputIds
   {
      LP_INPUT,
      BP_INPUT,
      HP_INPUT,
      CUTOFF_INPUT,
      RESONANCE_INPUT,
      NUM_INPUTS
   };
   enum OutputIds
   {
      AUDIO_OUTPUT,
      NUM_OUTPUTS
   };

   VultEngine_tangents_type processor;

   Tangents();
   void step() override;
};

Tangents::Tangents() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
{
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
   VultEngine_tangents_init(processor);
}

void Tangents::step()
{

   float lp = inputs[LP_INPUT].value / 15.0;
   float bp = inputs[BP_INPUT].value / 15.0;
   float hp = inputs[HP_INPUT].value / 15.0;

   float cutoff_knob = params[CUTOFF_PARAM].value;
   float cutoff_cv = inputs[CUTOFF_INPUT].value / 5.0;
   float cutoff_amt = params[CUTOFF_AMT_PARAM].value;
   float cutoff = cutoff_knob + cutoff_amt * cutoff_cv;

   float resonance_knob = params[RESONANCE_PARAM].value;
   float resonance_cv = inputs[RESONANCE_INPUT].value / 5.0;
   float resonance_amt = params[RESONANCE_AMT_PARAM].value;
   float resonance = resonance_knob + resonance_amt * resonance_cv;

   float out = VultEngine_tangents(processor, lp, bp, hp, cutoff, resonance);

   outputs[AUDIO_OUTPUT].value = out * 15.0;
}

struct TangentsWidget : ModuleWidget
{
   TangentsWidget(Tangents *module);
};

TangentsWidget::TangentsWidget(Tangents *module) : ModuleWidget(module)
{
   // Tangents *module = new Tangents();
   // setModule__deprecated__(module);
   box.size = Vec(15 * 10, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Tangents.svg")));
      addChild(panel);
   }
   addChild(createScrew<VultScrew>(Vec(15, 0)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 0)));
   addChild(createScrew<VultScrew>(Vec(15, 365)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 365)));

   addParam(createParam<VultKnobBig>(Vec(25, 75), module, Tangents::CUTOFF_PARAM, 0.0, 0.85, 0.85 / 2.0));
   addParam(createParam<VultKnob>(Vec(34, 173), module, Tangents::RESONANCE_PARAM, 0.0, 1.0, 0.0));

   addParam(createParam<VultKnobSmall>(Vec(108, 82), module, Tangents::CUTOFF_AMT_PARAM, -1.0, 1.0, 0.0));
   addParam(createParam<VultKnobSmall>(Vec(108, 173), module, Tangents::RESONANCE_AMT_PARAM, -1.0, 1.0, 0.0));

   addInput(createInput<VultJack>(Vec(105, 107), module, Tangents::CUTOFF_INPUT));
   addInput(createInput<VultJack>(Vec(105, 198), module, Tangents::RESONANCE_INPUT));

   addInput(createInput<VultJack>(Vec(21, 313), module, Tangents::LP_INPUT));
   addInput(createInput<VultJack>(Vec(63, 313), module, Tangents::BP_INPUT));
   addInput(createInput<VultJack>(Vec(105, 313), module, Tangents::HP_INPUT));

   addOutput(createOutput<VultJack>(Vec(63, 248), module, Tangents::AUDIO_OUTPUT));
}

RACK_PLUGIN_MODEL_INIT(VultModules, Tangents) {
   Model *model = Model::create<Tangents, TangentsWidget>("VultModules", "Tangents", "Tangents", FILTER_TAG);
   return model;
}
