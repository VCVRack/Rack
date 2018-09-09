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
#include "dsp/digital.hpp"

#include "bsp.hpp"

namespace rack_plugin_bsp {

struct DownSampler : Module {
	enum ParamIds {
		RATE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
      AUDIO_INPUT,
      TRIG_INPUT,
      RATE_MOD_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};

   static const uint32_t BUFFER_SIZE = (512*1024u);  //  ~11.8sec @ 44.1kHz
   static const uint32_t BUFFER_SIZE_MASK = (BUFFER_SIZE - 1u);

   float *buf;
   uint32_t buf_write_idx;
   int32_t rate_read_left;
   uint32_t buf_read_idx;
	SchmittTrigger trigger;
   float sample_rate;

	DownSampler() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
      buf_read_idx = ~0u;
      rate_read_left = 0;
      buf_write_idx = 0u;
      buf = new float[BUFFER_SIZE];
      handleSampleRateChanged();
   }
   
   ~DownSampler() {
      delete [] buf;
      buf = NULL;
   }

   void handleSampleRateChanged(void) {
      sample_rate = engineGetSampleRate();
   }

   void onSampleRateChange() override {
      Module::onSampleRateChange();

      handleSampleRateChanged();
   }

	void step() override;
};


void DownSampler::step() {

	if(trigger.process(inputs[TRIG_INPUT].value))
   {
      // printf("xxx DownSampler: trig\n");
      buf_read_idx = ~0u;
      buf_write_idx = 0u;
      rate_read_left = 0;
   }

   // Append new input to ring buffer
   float inVal = inputs[AUDIO_INPUT].value;
   buf[buf_write_idx] = inVal;
   buf_write_idx = (buf_write_idx + 1u) & BUFFER_SIZE_MASK;

   if(--rate_read_left < 0)
   {
      buf_read_idx = (buf_read_idx + 1u) & BUFFER_SIZE_MASK;

      float rateF = params[RATE_PARAM].value;

      if(inputs[RATE_MOD_INPUT].active)
      {
         rateF += inputs[RATE_MOD_INPUT].value * (7.0f / 5.0f);
      }

      rate_read_left = int32_t(rateF);
   }

   float outVal = buf[buf_read_idx];

   outputs[AUDIO_OUTPUT].value = outVal;

#if 0
   static int xxx = 0;
   if(0 == (++xxx & 32767))
   {
      printf("xxx readIdx=%u writeIdx=%u readLeft=%d\n", buf_read_idx, buf_write_idx, rate_read_left);
   }
#endif
}


struct DownSamplerWidget : ModuleWidget {
	DownSamplerWidget(DownSampler *module);
};

DownSamplerWidget::DownSamplerWidget(DownSampler *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/DownSampler.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

   float cx;
   float cy;

#define STY 60.0f
   cx = 12.0f;
   cy = 66.0f;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, DownSampler::AUDIO_INPUT));
   cy += STY;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, DownSampler::TRIG_INPUT));
#undef STY

#define STY 32.0f
   cx = 12.0f;
   cy = 200.0f;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, DownSampler::RATE_PARAM, 0.0f, 7.0f, 1.0f));
   cy += STY;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, DownSampler::RATE_MOD_INPUT));
#undef STX
#undef STY

	addOutput(Port::create<PJ301MPort>(Vec(11, 325), Port::OUTPUT, module, DownSampler::AUDIO_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, DownSampler) {
   Model *modelDownSampler = Model::create<DownSampler, DownSamplerWidget>("bsp", "DownSampler", "DownSampler", SAMPLER_TAG);
   return modelDownSampler;
}
