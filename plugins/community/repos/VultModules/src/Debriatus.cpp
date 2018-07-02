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

struct Debriatus : Module
{
   enum ParamIds
   {
      FOLD_PARAM,
      CRUSH_PARAM,
      DISTORT_PARAM,
      SATURATE_PARAM,
      FOLD_AMT_PARAM,
      CRUSH_AMT_PARAM,
      DISTORT_AMT_PARAM,
      SATURATE_AMT_PARAM,
      NUM_PARAMS
   };
   enum InputIds
   {
      AUDIO_INPUT,
      FOLD_INPUT,
      CRUSH_INPUT,
      DISTORT_INPUT,
      SATURATE_INPUT,
      NUM_INPUTS
   };
   enum OutputIds
   {
      AUDIO_OUTPUT,
      NUM_OUTPUTS
   };

   Debriatus();
   void step() override;
};

Debriatus::Debriatus() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
{
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
}

void Debriatus::step()
{

   float audio = inputs[AUDIO_INPUT].value / 5.0;

   float fold_knob = params[FOLD_PARAM].value;
   float fold_cv = inputs[FOLD_INPUT].value / 5.0;
   float fold_amt = params[FOLD_AMT_PARAM].value;
   float fold = fold_knob + fold_amt * fold_cv;

   float crush_knob = params[CRUSH_PARAM].value;
   float crush_cv = inputs[CRUSH_INPUT].value / 5.0;
   float crush_amt = params[CRUSH_AMT_PARAM].value;
   float crush = crush_knob + crush_amt * crush_cv;

   float distort_knob = params[DISTORT_PARAM].value;
   float distort_cv = inputs[DISTORT_INPUT].value / 5.0;
   float distort_amt = params[DISTORT_AMT_PARAM].value;
   float distort = distort_knob + distort_amt * distort_cv;

   float saturate_knob = params[SATURATE_PARAM].value;
   float saturate_cv = inputs[SATURATE_INPUT].value / 5.0;
   float saturate_amt = params[SATURATE_AMT_PARAM].value;
   float saturate = saturate_knob + saturate_amt * saturate_cv;

   float out = VultEngine_debriatus(audio, fold, crush, distort, saturate);

   outputs[AUDIO_OUTPUT].value = out * 5.0;
}

struct DebriatusWidget : ModuleWidget
{
   DebriatusWidget(Debriatus *module);
};

DebriatusWidget::DebriatusWidget(Debriatus *module) : ModuleWidget(module)
{
   // Debriatus *module = new Debriatus();
   // setModule__deprecated__(module);
   box.size = Vec(15 * 10, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Debriatus.svg")));
      addChild(panel);
   }
   addChild(createScrew<VultScrew>(Vec(15, 0)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 0)));
   addChild(createScrew<VultScrew>(Vec(15, 365)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 365)));

   addParam(createParam<VultKnob>(Vec(30, 52), module, Debriatus::FOLD_PARAM, 0.0, 1.0, 0.0));
   addParam(createParam<VultKnob>(Vec(30, 118), module, Debriatus::CRUSH_PARAM, 0.0, 1.0, 0.0));
   addParam(createParam<VultKnob>(Vec(30, 185), module, Debriatus::DISTORT_PARAM, 0.0, 1.0, 0.0));
   addParam(createParam<VultKnob>(Vec(30, 252), module, Debriatus::SATURATE_PARAM, 0.0, 1.0, 0.0));

   addParam(createParam<VultKnobSmall>(Vec(104, 52), module, Debriatus::FOLD_AMT_PARAM, -1.0, 1.0, 0.0));
   addParam(createParam<VultKnobSmall>(Vec(104, 118), module, Debriatus::CRUSH_AMT_PARAM, -1.0, 1.0, 0.0));
   addParam(createParam<VultKnobSmall>(Vec(104, 185), module, Debriatus::DISTORT_AMT_PARAM, -1.0, 1.0, 0.0));
   addParam(createParam<VultKnobSmall>(Vec(104, 252), module, Debriatus::SATURATE_AMT_PARAM, -1.0, 1.0, 0.0));

   addInput(createInput<VultJack>(Vec(101, 77), module, Debriatus::FOLD_INPUT));
   addInput(createInput<VultJack>(Vec(101, 143), module, Debriatus::CRUSH_INPUT));
   addInput(createInput<VultJack>(Vec(101, 210), module, Debriatus::DISTORT_INPUT));
   addInput(createInput<VultJack>(Vec(101, 277), module, Debriatus::SATURATE_INPUT));

   addInput(createInput<VultJack>(Vec(27, 318), module, Debriatus::AUDIO_INPUT));
   addOutput(createOutput<VultJack>(Vec(101, 318), module, Debriatus::AUDIO_OUTPUT));
}

RACK_PLUGIN_MODEL_INIT(VultModules, Debriatus) {
   Model *model = Model::create<Debriatus, DebriatusWidget>("VultModules", "Debriatus", "Debriatus", WAVESHAPER_TAG);
   return model;
}
