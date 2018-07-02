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

struct Tohe : Module
{
   enum ParamIds
   {
      TONE_PARAM,
      TONE_AMT_PARAM,
      NUM_PARAMS
   };
   enum InputIds
   {
      AUDIO_INPUT,
      TONE_INPUT,
      NUM_INPUTS
   };
   enum OutputIds
   {
      AUDIO_OUTPUT,
      NUM_OUTPUTS
   };

   VultEngine_tohe_type processor;

   Tohe();
   void step() override;
};

Tohe::Tohe() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
{
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
   VultEngine_tohe_init(processor);
}

void Tohe::step()
{

   float audio = inputs[AUDIO_INPUT].value;

   float tone_knob = params[TONE_PARAM].value;
   float tone_cv = inputs[TONE_INPUT].value / 5.0;
   float tone_amt = params[TONE_AMT_PARAM].value;
   float tone = tone_knob + tone_amt * tone_cv;

   float out = VultEngine_tohe(processor, audio, tone);

   outputs[AUDIO_OUTPUT].value = out;
}

struct ToheWidget : ModuleWidget
{
   ToheWidget(Tohe *module);
};

ToheWidget::ToheWidget(Tohe *module) : ModuleWidget(module)
{
   // Tohe *module = new Tohe();
   // setModule__deprecated__(module);
   box.size = Vec(15 * 4, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Tohe.svg")));
      addChild(panel);
   }

   addChild(createScrew<VultScrew>(Vec(23, 0)));
   addChild(createScrew<VultScrew>(Vec(23, 365)));

   addParam(createParam<VultKnob>(Vec(10, 88), module, Tohe::TONE_PARAM, -1.0, 1.0, 0.0));
   addParam(createParam<VultKnobSmall>(Vec(21, 158), module, Tohe::TONE_AMT_PARAM, -1.0, 1.0, 0.0));
   addInput(createInput<VultJack>(Vec(18, 183), module, Tohe::TONE_INPUT));
   addInput(createInput<VultJack>(Vec(17, 228), module, Tohe::AUDIO_INPUT));
   addOutput(createOutput<VultJack>(Vec(17, 274), module, Tohe::AUDIO_OUTPUT));
}

RACK_PLUGIN_MODEL_INIT(VultModules, Tohe) {
   Model *model = Model::create<Tohe, ToheWidget>("VultModules", "Tohe", "Tohe", EQUALIZER_TAG);
   return model;
}
