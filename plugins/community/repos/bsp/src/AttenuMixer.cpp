/*
Copyright (c) 2018 bsp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <math.h>

#include "bsp.hpp"

namespace rack_plugin_bsp {

typedef union fi_u {
   float f;
   unsigned int u;
   int s;
} fi_t;

struct AttenuMixer : Module {
	enum ParamIds {
		IN_1_SCL_PARAM,
		IN_1_OFF_PARAM,
		IN_2_SCL_PARAM,
		IN_2_OFF_PARAM,
		IN_3_SCL_PARAM,
		IN_3_OFF_PARAM,
		IN_4_SCL_PARAM,
		IN_4_OFF_PARAM,
		BIPOLAR_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
      IN_1_INPUT,
      IN_2_INPUT,
      IN_3_INPUT,
      IN_4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CTL_OUTPUT,
		NUM_OUTPUTS
	};

	AttenuMixer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
   }

	void step() override;
};


void AttenuMixer::step() {

   float outVal = 0.0f;

   if(params[BIPOLAR_PARAM].value >= 0.5f)
   {
      for(int i = 0; i < 4; i++)
      {
         fi_t scl; scl.f = params[IN_1_SCL_PARAM + (i<<1)].value;
         scl.f = (2.0f * scl.f) - 1.0f;
         uint32_t sclSign = scl.u & 0x80000000u;
         scl.f *= scl.f;
         scl.f *= scl.f;
         scl.u |= sclSign;
         outVal += inputs[IN_1_INPUT + i].value * scl.f + params[IN_1_OFF_PARAM + (i<<1)].value;
      }
   }
   else
   {
      for(int i = 0; i < 4; i++)
      {
         float scl = params[IN_1_SCL_PARAM + (i<<1)].value;
         scl *= scl;
         scl *= scl;
         outVal += inputs[IN_1_INPUT + i].value * scl + params[IN_1_OFF_PARAM + (i<<1)].value;
      }
   }

	outputs[CTL_OUTPUT].value = outVal;

#if 0
   static int xxx = 0;
   if(0 == (++xxx & 32767))
   {
      printf("xxx params[IN_2_SCL_PARAM].value=%f\n", params[IN_2_SCL_PARAM].value);
   }
#endif
}


struct AttenuMixerWidget : ModuleWidget {
	AttenuMixerWidget(AttenuMixer *module);
};

AttenuMixerWidget::AttenuMixerWidget(AttenuMixer *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/AttenuMixer.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

	addParam(ParamWidget::create<CKSS>(Vec(box.size.x - 19, 18), module, AttenuMixer::BIPOLAR_PARAM, 0.0f, 1.0f, 0.0f));

#define STY 42
#define OFX 17
#define OFY 20
   float cx = 2.0f;
   float cy = 34.0f;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, AttenuMixer::IN_1_SCL_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx + OFX, cy + OFY), module, AttenuMixer::IN_1_OFF_PARAM, -5.0f, 5.0f, 0.0f));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, AttenuMixer::IN_2_SCL_PARAM, 0.0f, 1.0f, 0.796f));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx + OFX, cy + OFY), module, AttenuMixer::IN_2_OFF_PARAM, -5.0f, 5.0f, 0.0f));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, AttenuMixer::IN_3_SCL_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx + OFX, cy + OFY), module, AttenuMixer::IN_3_OFF_PARAM, -5.0f, 5.0f, 0.0f));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, AttenuMixer::IN_4_SCL_PARAM, 0.0f, 1.0f, 0.25f));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx + OFX, cy + OFY), module, AttenuMixer::IN_4_OFF_PARAM, -5.0f, 5.0f, 0.0f));
#undef STX
#undef STY

#define STY 28.0f
   cx = 11.0f;
   cy = 208.0f;
	addInput(Port::create<PJ301MPort>(Vec(cx, cy), Port::INPUT, module, AttenuMixer::IN_1_INPUT));
   cy += STY;
	addInput(Port::create<PJ301MPort>(Vec(cx, cy), Port::INPUT, module, AttenuMixer::IN_2_INPUT));
   cy += STY;
	addInput(Port::create<PJ301MPort>(Vec(cx, cy), Port::INPUT, module, AttenuMixer::IN_3_INPUT));
   cy += STY;
	addInput(Port::create<PJ301MPort>(Vec(cx, cy), Port::INPUT, module, AttenuMixer::IN_4_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(11, 325), Port::OUTPUT, module, AttenuMixer::CTL_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, AttenuMixer) {
   Model *modelAttenuMixer = Model::create<AttenuMixer, AttenuMixerWidget>("bsp", "AttenuMixer", "AttenuMixer", ATTENUATOR_TAG, MIXER_TAG);
   return modelAttenuMixer;
}
