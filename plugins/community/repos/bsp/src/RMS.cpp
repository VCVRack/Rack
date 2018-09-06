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
#include <stdlib.h>  // memset

#include "bsp.hpp"

namespace rack_plugin_bsp {

typedef union fi_u {
   float f;
   unsigned int u;
   int s;
} fi_t;

struct RMS : Module {
	enum ParamIds {
		IN_AMP_PARAM,
		WIN_SIZE_PARAM,
		SMOOTH_RISE_PARAM,
		SMOOTH_FALL_PARAM,
		OUT_AMP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
      AUDIO_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		RMS_OUTPUT,
		NUM_OUTPUTS
	};

   static const uint32_t MAX_WIN_SIZE = (1024u);

   float buf[MAX_WIN_SIZE];
   uint32_t buf_idx;

   double integrated_val;
   double smoothed_sign;
   double last_smoothed_val;
   double smoothed_val;

   uint32_t last_win_size;

	RMS() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
      last_win_size = 0u;
      buf_idx = 0u;
   }

	void step() override;
};


void RMS::step() {

#if 0
   outputs[RMS_OUTPUT].value = 0.0f;
   return;
#endif

   uint32_t winSize = (1u << uint32_t(params[WIN_SIZE_PARAM].value));
   uint32_t winSizeMask = (winSize - 1u);

   if(winSize != last_win_size)
   {
      last_win_size = winSize;
      ::memset((void*)buf, 0, winSize * sizeof(float));
      buf_idx = 0u;
      integrated_val = 0.0;
      smoothed_sign = 0.0;
      last_smoothed_val = 0.0;
      smoothed_val = 0.0;
   }

   float inAmp = params[IN_AMP_PARAM].value;
   inAmp *= inAmp;
   inAmp *= inAmp;
   // amp is now in range 0..1000

   // Read new input and calc square
   float inValOrig = inputs[AUDIO_INPUT].value;
   float inVal = inValOrig * inAmp;
   inVal *= inVal;

   // Integrate new input
   integrated_val += inVal;

   // Subtract oldest input
   integrated_val -= buf[(buf_idx - winSize + 1u) & winSizeMask];

   buf[buf_idx] = inVal;
   buf_idx = (buf_idx + 1u) & winSizeMask;

   double outVal = integrated_val / double(winSize);
   if(outVal > 0.0)
      outVal = sqrt(outVal);

   // Smoothing
   double smoothAmt;
   if(smoothed_sign >= 0.0)
   {
      smoothAmt = params[SMOOTH_RISE_PARAM].value;
   }
   else
   {
      smoothAmt = params[SMOOTH_FALL_PARAM].value;
   }

   smoothAmt = (1.0 - smoothAmt);
   smoothAmt *= smoothAmt;
   smoothAmt *= smoothAmt;
   smoothAmt *= smoothAmt;
   smoothed_val = smoothed_val + (outVal - smoothed_val) * smoothAmt;

   smoothed_sign = (smoothed_val - last_smoothed_val);
   last_smoothed_val = smoothed_val;

   // Output
   float outAmp = params[OUT_AMP_PARAM].value;
   outAmp *= outAmp;
   outAmp *= outAmp;
   // out amp is now in range 0..1000
   outputs[RMS_OUTPUT].value = float(smoothed_val * outAmp);

#if 0
   static int xxx = 0;
   if(0 == (++xxx & 32767))
   {
      printf("xxx winSize=%u winSizeMask=%u bufIdx=%u smoothAmt=%f\n", winSize, winSizeMask, buf_idx, smoothAmt);
   }
#endif
}


struct RMSWidget : ModuleWidget {
	RMSWidget(RMS *module);
};

RMSWidget::RMSWidget(RMS *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/RMS.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

   float cx;
   float cy;

#define STY 33.0f
   cx = 12.0f;
   cy = 50.0f;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, RMS::AUDIO_INPUT));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, RMS::IN_AMP_PARAM, 0.0f, 3.0f, 0.562341325191f));
#undef STY

#define STY 50.0f
   cx = 12.0f;
   cy = 140.0f;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, RMS::WIN_SIZE_PARAM, 1.0f, 10.0f, 6.0f));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, RMS::SMOOTH_RISE_PARAM, 0.0f, 1.0f, 0.447f));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, RMS::SMOOTH_FALL_PARAM, 0.0f, 1.0f, 0.52f));
#undef STX
#undef STY

	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(12.0f, 290.0f), module, RMS::OUT_AMP_PARAM, 0.0f, 3.0f, 1.8f));
	addOutput(Port::create<PJ301MPort>(Vec(11, 325), Port::OUTPUT, module, RMS::RMS_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, RMS) {
   Model *modelRMS = Model::create<RMS, RMSWidget>("bsp", "RMS", "RMS", ENVELOPE_FOLLOWER_TAG, UTILITY_TAG);
   return modelRMS;
}
