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

struct Rescomb : Module
{
   enum ParamIds
   {
      TUNE_PARAM,
      COMB_PARAM,
      FEEDBACK_PARAM,
      COMB_AMT_PARAM,
      FEEDBACK_AMT_PARAM,
      NUM_PARAMS
   };
   enum InputIds
   {
      PITCH_INPUT,
      AUDIO_INPUT,
      COMB_INPUT,
      FEEDBACK_INPUT,
      NUM_INPUTS
   };
   enum OutputIds
   {
      AUDIO_OUTPUT,
      NUM_OUTPUTS
   };

   VultEngine_rescomb_type processor;

   Rescomb();
   void step() override;
};

Rescomb::Rescomb() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
{
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
   VultEngine_rescomb_init(processor);
}

void Rescomb::step()
{
   float pitch = inputs[PITCH_INPUT].value / 10.0;
   float tune = params[TUNE_PARAM].value;

   float audio = inputs[AUDIO_INPUT].value / 5.0;

   float comb_cv = inputs[COMB_INPUT].value / 5.0;
   float comb_p = params[COMB_PARAM].value;
   float comb_amt = params[COMB_AMT_PARAM].value;

   float feedback_cv = inputs[FEEDBACK_INPUT].value / 5.0;
   float feedback_p = params[FEEDBACK_PARAM].value;
   float feedback_amt = params[FEEDBACK_AMT_PARAM].value;

   float comb = comb_p + comb_cv * comb_amt;

   float feedback = feedback_p + feedback_cv * feedback_amt;

   float out = VultEngine_rescomb(processor, audio, pitch + tune, comb, feedback);

   outputs[AUDIO_OUTPUT].value = out * 5.0;
}

struct RescombWidget : ModuleWidget
{
   RescombWidget(Rescomb *module);
};

RescombWidget::RescombWidget(Rescomb *module) : ModuleWidget(module)
{
   // Rescomb *module = new Rescomb();
   // setModule__deprecated__(module);
   box.size = Vec(15 * 10, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Rescomb.svg")));
      addChild(panel);
   }

   addChild(createScrew<VultScrew>(Vec(15, 0)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 0)));
   addChild(createScrew<VultScrew>(Vec(15, 365)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 365)));

   addParam(createParam<VultKnob>(Vec(30, 78), module, Rescomb::TUNE_PARAM, -0.4, 0.4, 0.0));
   addParam(createParam<VultKnob>(Vec(30, 158), module, Rescomb::COMB_PARAM, -1.0, 1.0, 0.0));
   addParam(createParam<VultKnob>(Vec(30, 238), module, Rescomb::FEEDBACK_PARAM, 0.0, 1.1, 0.0));

   addParam(createParam<VultKnobSmall>(Vec(103, 158), module, Rescomb::COMB_AMT_PARAM, -1.0, 1.0, 0.0));
   addParam(createParam<VultKnobSmall>(Vec(103, 238), module, Rescomb::FEEDBACK_AMT_PARAM, -1.0, 1.0, 0.0));

   addInput(createInput<VultJack>(Vec(101, 85), module, Rescomb::PITCH_INPUT));
   addInput(createInput<VultJack>(Vec(101, 183), module, Rescomb::COMB_INPUT));
   addInput(createInput<VultJack>(Vec(101, 263), module, Rescomb::FEEDBACK_INPUT));

   addInput(createInput<VultJack>(Vec(27, 318), module, Rescomb::AUDIO_INPUT));
   addOutput(createOutput<VultJack>(Vec(101, 318), module, Rescomb::AUDIO_OUTPUT));
}

RACK_PLUGIN_MODEL_INIT(VultModules, Rescomb) {
   Model *model = Model::create<Rescomb, RescombWidget>("VultModules", "Rescomb", "Rescomb", FILTER_TAG);
   return model;
}
