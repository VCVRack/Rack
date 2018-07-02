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

struct Trummor : Module
{
   enum ParamIds
   {
      LEVEL1_PARAM,
      LEVEL2_PARAM,
      ENV1_A_PARAM,
      ENV1_H_PARAM,
      ENV1_R_PARAM,
      ENV2_A_PARAM,
      ENV2_H_PARAM,
      ENV2_R_PARAM,
      PITCH_PARAM,
      BEND_PARAM,
      DRIVE_PARAM,
      TONE_PARAM,
      OSC_BLEND_PARAM,
      NOISE_BLEND_PARAM,
      SEL_ENV1_PARAM,
      SEL_ENV2_PARAM,
      DECIMATE_PARAM,
      OSC_MOD_PARAM,
      NOISE_MOD_PARAM,
      OSC_SEL_PARAM,
      NOISE_SEL_PARAM,
      NUM_PARAMS
   };
   enum InputIds
   {
      GATE_INPUT,
      OSC_INPUT,
      NOISE_INPUT,
      OSC_MOD_INPUT,
      NOISE_MOD_INPUT,
      NUM_INPUTS
   };
   enum OutputIds
   {
      AUDIO_OUTPUT,
      PITCH_OUTPUT,
      ENV1_OUTPUT,
      ENV2_OUTPUT,
      NUM_OUTPUTS
   };

   Trummor_do_type processor;

   Trummor();
   void step();
};

Trummor::Trummor() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
{
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
   Trummor_do_init(processor);
}

void Trummor::step()
{

   Trummor_setLevel1(processor, params[LEVEL1_PARAM].value);
   Trummor_setLevel2(processor, params[LEVEL2_PARAM].value);

   Trummor_setEnv1A(processor, params[ENV1_A_PARAM].value);
   Trummor_setEnv1H(processor, params[ENV1_H_PARAM].value);
   Trummor_setEnv1R(processor, params[ENV1_R_PARAM].value);

   Trummor_setEnv2A(processor, params[ENV2_A_PARAM].value);
   Trummor_setEnv2H(processor, params[ENV2_H_PARAM].value);
   Trummor_setEnv2R(processor, params[ENV2_R_PARAM].value);

   Trummor_setPitch(processor, params[PITCH_PARAM].value);
   Trummor_setBend(processor, params[BEND_PARAM].value);
   Trummor_setDrive(processor, params[DRIVE_PARAM].value);

   Trummor_setTone(processor, params[TONE_PARAM].value);

   Trummor_setOscBlend(processor, params[OSC_BLEND_PARAM].value);
   Trummor_setNoiseBlend(processor, params[NOISE_BLEND_PARAM].value);

   Trummor_setEnv1Scale(processor, params[SEL_ENV1_PARAM].value);
   Trummor_setEnv2Scale(processor, params[SEL_ENV2_PARAM].value);

   Trummor_setDecimate(processor, params[DECIMATE_PARAM].value);

   int osc_sel = round(params[OSC_SEL_PARAM].value);
   float osc_mod = params[OSC_MOD_PARAM].value * inputs[OSC_MOD_INPUT].value / 5.0;
   switch (osc_sel)
   {
   case 0:
      Trummor_setPitch(processor, osc_mod / 10.0f + params[PITCH_PARAM].value);
      break;
   case 1:
      Trummor_setBend(processor, (osc_mod + params[BEND_PARAM].value));
      break;
   case 2:
      Trummor_setDrive(processor, (osc_mod + params[DRIVE_PARAM].value));
      break;
   case 3:
      Trummor_setEnv1A(processor, (osc_mod + params[ENV1_A_PARAM].value));
      break;
   case 4:
      Trummor_setEnv1H(processor, (osc_mod + params[ENV1_H_PARAM].value));
      break;
   case 5:
      Trummor_setEnv1R(processor, (osc_mod + params[ENV1_R_PARAM].value));
      break;
   case 6:
      Trummor_setEnv1Scale(processor, (osc_mod + params[SEL_ENV1_PARAM].value));
      break;
   case 7:
      Trummor_setOscBlend(processor, (osc_mod + params[OSC_BLEND_PARAM].value));
      break;
   case 8:
      Trummor_setLevel1(processor, (osc_mod + params[LEVEL1_PARAM].value));
      break;
   }

   int noise_sel = round(params[NOISE_SEL_PARAM].value);
   float noise_mod = params[NOISE_MOD_PARAM].value * inputs[NOISE_MOD_INPUT].value / 5.0;

   switch (noise_sel)
   {
   case 0:
      Trummor_setTone(processor, (noise_mod + params[TONE_PARAM].value));
      break;
   case 1:
      Trummor_setDecimate(processor, (noise_mod + params[DECIMATE_PARAM].value));
      break;
   case 2:
      Trummor_setEnv2A(processor, (noise_mod + params[ENV2_A_PARAM].value));
      break;
   case 3:
      Trummor_setEnv2H(processor, (noise_mod + params[ENV2_H_PARAM].value));
      break;
   case 4:
      Trummor_setEnv2R(processor, (noise_mod + params[ENV2_R_PARAM].value));
      break;
   case 5:
      Trummor_setEnv2Scale(processor, (noise_mod + params[SEL_ENV2_PARAM].value));
      break;
   case 6:
      Trummor_setNoiseBlend(processor, (noise_mod + params[NOISE_BLEND_PARAM].value));
      break;
   case 7:
      Trummor_setLevel2(processor, (noise_mod + params[LEVEL2_PARAM].value));
      break;
   }

   _tuple___real_real_real_real__ out;
   Trummor_do(processor, inputs[GATE_INPUT].value / 10.0f, inputs[OSC_INPUT].value / 10.0f, inputs[NOISE_INPUT].value / 10.0f, out);

   outputs[AUDIO_OUTPUT].value = out.field_0 * 10.0f;
   outputs[PITCH_OUTPUT].value = (out.field_1 - 0.3f) * 10.0f;
   outputs[ENV1_OUTPUT].value = out.field_2 * 10.0f;
   outputs[ENV2_OUTPUT].value = out.field_3 * 10.0f;
}

struct TrummorWidget : ModuleWidget
{
   TrummorWidget(Trummor *module);
};

TrummorWidget::TrummorWidget(Trummor *module) : ModuleWidget(module)
{
   // Trummor *module = new Trummor();
   // setModule__deprecated__(module);
   box.size = Vec(300, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Trummor.svg")));
      addChild(panel);
   }
   addChild(createScrew<VultScrew>(Vec(15, 0)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 0)));
   addChild(createScrew<VultScrew>(Vec(15, 365)));
   addChild(createScrew<VultScrew>(Vec(box.size.x - 30, 365)));

   addParam(createParam<VultKnob>(Vec(101, 196), module, Trummor::LEVEL1_PARAM, 0.0, 1.0, 0.7));
   addParam(createParam<VultKnob>(Vec(244, 196), module, Trummor::LEVEL2_PARAM, 0.0, 1.0, 0.1));

   addParam(createParam<VultKnobAlt>(Vec(17, 140), module, Trummor::ENV1_A_PARAM, 0.0, 1.0, 0.0));
   addParam(createParam<VultKnobAlt>(Vec(63, 140), module, Trummor::ENV1_H_PARAM, 0.0, 1.0, 0.2));
   addParam(createParam<VultKnobAlt>(Vec(110, 140), module, Trummor::ENV1_R_PARAM, 0.0, 1.0, 0.2));

   addParam(createParam<VultKnobAlt>(Vec(160, 140), module, Trummor::ENV2_A_PARAM, 0.0, 1.0, 0.0));
   addParam(createParam<VultKnobAlt>(Vec(206, 140), module, Trummor::ENV2_H_PARAM, 0.0, 1.0, 0.05));
   addParam(createParam<VultKnobAlt>(Vec(252, 140), module, Trummor::ENV2_R_PARAM, 0.0, 1.0, 0.05));

   addParam(createParam<VultKnobAlt>(Vec(17, 85), module, Trummor::PITCH_PARAM, -0.1, 0.3, 0.0));
   addParam(createParam<VultKnobAlt>(Vec(63, 85), module, Trummor::BEND_PARAM, -1.0, 1.0, 0.5));
   addParam(createParam<VultKnobAlt>(Vec(110, 85), module, Trummor::DRIVE_PARAM, 0.0, 4.0, 0.0));

   addParam(createParam<VultKnobAlt>(Vec(177, 85), module, Trummor::TONE_PARAM, -1.0, 1.0, -0.7));

   addParam(createParam<VultKnobAlt>(Vec(53, 205), module, Trummor::OSC_BLEND_PARAM, 0.0, 1.0, 0.0));
   addParam(createParam<VultKnobAlt>(Vec(195, 205), module, Trummor::NOISE_BLEND_PARAM, 0.0, 1.0, 0.0));

   addParam(createParam<VultSelector2>(Vec(12, 201), module, Trummor::SEL_ENV1_PARAM, 0.0, 1.0, 0.0));
   addParam(createParam<VultSelector2>(Vec(155, 201), module, Trummor::SEL_ENV2_PARAM, 0.0, 1.0, 0.0));

   addInput(createInput<VultJack>(Vec(63, 336), module, Trummor::GATE_INPUT));

   addInput(createInput<VultJack>(Vec(20, 295), module, Trummor::OSC_INPUT));
   addInput(createInput<VultJack>(Vec(188, 295), module, Trummor::NOISE_INPUT));

   addOutput(createOutput<VultJack>(Vec(213, 336), module, Trummor::AUDIO_OUTPUT));

   addOutput(createOutput<VultJack>(Vec(112, 295), module, Trummor::PITCH_OUTPUT));

   addOutput(createOutput<VultJack>(Vec(67, 295), module, Trummor::ENV1_OUTPUT));
   addOutput(createOutput<VultJack>(Vec(238, 295), module, Trummor::ENV2_OUTPUT));

   addInput(createInput<VultJack>(Vec(14, 257), module, Trummor::OSC_MOD_INPUT));
   addInput(createInput<VultJack>(Vec(157, 257), module, Trummor::NOISE_MOD_INPUT));

   addParam(createParam<VultKnobSmall>(Vec(50, 260), module, Trummor::OSC_MOD_PARAM, -1.0, 1.0, 0.0));
   addParam(createParam<VultKnobSmall>(Vec(193, 260), module, Trummor::NOISE_MOD_PARAM, -1.0, 1.0, 0.0));

   addParam(createParam<TrummodNoiseSelector>(Vec(218, 262), module, Trummor::NOISE_SEL_PARAM, 0.0, 7.0, 0.0));
   addParam(createParam<TrummodOscSelector>(Vec(75, 262), module, Trummor::OSC_SEL_PARAM, 0.0, 8.0, 0.0));

   addParam(createParam<VultKnobAlt>(Vec(237, 85), module, Trummor::DECIMATE_PARAM, 0.0, 1.0, 0.0));
}

RACK_PLUGIN_MODEL_INIT(VultModules, Trummor) {
   Model *model = Model::create<Trummor, TrummorWidget>("VultModules", "Trummor", "Trummor", DRUM_TAG);
   return model;
}
