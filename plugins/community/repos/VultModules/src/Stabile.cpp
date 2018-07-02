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

struct Stabile : Module
{
   enum ParamIds
   {
      CUTOFF_PARAM,
      RESONANCE_PARAM,
      CUTOFF_AMT_PARAM,
      RESONANCE_AMT_PARAM,
      SEMBLANCE_PARAM,
      SEMBLANCE_AMT_PARAM,
      NUM_PARAMS
   };
   enum InputIds
   {
      AUDIO_INPUT,
      CUTOFF_INPUT,
      RESONANCE_INPUT,
      SEMBLANCE_INPUT,
      NUM_INPUTS
   };
   enum OutputIds
   {
      LP_OUTPUT,
      BP_OUTPUT,
      HP_OUTPUT,
      SEM_OUTPUT,
      NUM_OUTPUTS
   };

   VultEngine_stabile_type processor;

   Stabile();
   void step() override;
};

Stabile::Stabile() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
{
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
   VultEngine_stabile_init(processor);
}

void Stabile::step()
{

   float audio = inputs[AUDIO_INPUT].value / 5.0;

   float cutoff_knob = params[CUTOFF_PARAM].value;
   float cutoff_cv = inputs[CUTOFF_INPUT].value / 5.0;
   float cutoff_amt = params[CUTOFF_AMT_PARAM].value;
   float cutoff = cutoff_knob + cutoff_amt * cutoff_cv;

   float resonance_knob = params[RESONANCE_PARAM].value;
   float resonance_cv = inputs[RESONANCE_INPUT].value / 5.0;
   float resonance_amt = params[RESONANCE_AMT_PARAM].value;
   float resonance = resonance_knob + resonance_amt * resonance_cv;

   float semblance_knob = params[SEMBLANCE_PARAM].value;
   float semblance_cv = inputs[SEMBLANCE_INPUT].value / 5.0;
   float semblance_amt = params[SEMBLANCE_AMT_PARAM].value;
   float semblance = semblance_knob + semblance_amt * semblance_cv;

   _tuple___real_real_real_real__ out;
   VultEngine_stabile(processor, audio, cutoff, resonance, semblance, out);

   outputs[LP_OUTPUT].value = out.field_0 * 5.0;

   outputs[BP_OUTPUT].value = out.field_1 * 5.0;

   outputs[HP_OUTPUT].value = out.field_2 * 5.0;

   outputs[SEM_OUTPUT].value = out.field_3 * 5.0;
}

struct StabileWidget : ModuleWidget
{
   StabileWidget(Stabile *module);
};

StabileWidget::StabileWidget(Stabile *module) : ModuleWidget(module)
{
   // Stabile *module = new Stabile();
   // setModule__deprecated__(module);
   box.size = Vec(15 * 10, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Stabile.svg")));
      addChild(panel);
   }

   addChild(createScrew<VultScrew>(Vec(15, 0)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 0)));
   addChild(createScrew<VultScrew>(Vec(15, 365)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 365)));

   addParam(createParam<VultKnobBig>(Vec(25, 53), module, Stabile::CUTOFF_PARAM, 0.0, 1.0, 0.5));
   addParam(createParam<VultKnob>(Vec(34, 134), module, Stabile::RESONANCE_PARAM, 0.0, 4.0, 0.0));

   addParam(createParam<VultKnobSmall>(Vec(108, 60), module, Stabile::CUTOFF_AMT_PARAM, -1.0, 1.0, 0.0));
   addParam(createParam<VultKnobSmall>(Vec(108, 134), module, Stabile::RESONANCE_AMT_PARAM, -1.0, 1.0, 0.0));

   addParam(createParam<VultKnob>(Vec(34, 202), module, Stabile::SEMBLANCE_PARAM, 0.0, 1.0, 0.5));
   addParam(createParam<VultKnobSmall>(Vec(108, 202), module, Stabile::SEMBLANCE_AMT_PARAM, -1.0, 1.0, 0.0));

   addInput(createInput<VultJack>(Vec(105, 85), module, Stabile::CUTOFF_INPUT));
   addInput(createInput<VultJack>(Vec(105, 159), module, Stabile::RESONANCE_INPUT));

   addInput(createInput<VultJack>(Vec(60, 318), module, Stabile::AUDIO_INPUT));

   addOutput(createOutput<VultJack>(Vec(9, 277), module, Stabile::LP_OUTPUT));
   addOutput(createOutput<VultJack>(Vec(45, 277), module, Stabile::BP_OUTPUT));
   addOutput(createOutput<VultJack>(Vec(82, 277), module, Stabile::HP_OUTPUT));

   addInput(createInput<VultJack>(Vec(105, 227), module, Stabile::SEMBLANCE_INPUT));
   addOutput(createOutput<VultJack>(Vec(118, 277), module, Stabile::SEM_OUTPUT));
}

RACK_PLUGIN_MODEL_INIT(VultModules, Stabile) {
   Model *model = Model::create<Stabile, StabileWidget>("VultModules", "Stabile", "Stabile", FILTER_TAG);
   return model;
}
