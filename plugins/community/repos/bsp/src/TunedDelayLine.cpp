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

/// When defined, use linear interpolation when reading samples from delay line
// #define USE_FRAC defined

#include <math.h>
#include <stdlib.h>  // memset

#include "bsp.hpp"

namespace rack_plugin_bsp {

struct TunedDelayLine : Module {
	enum ParamIds {
		DRYWET_PARAM,
		FB_AMT_PARAM,
		FINETUNE_PARAM,  // or delaytime in seconds when no V/OCT input is connected
      POSTFB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
      VOCT_INPUT,
      AUDIO_INPUT,
      FB_RET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		FB_SEND_OUTPUT,
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};

#define BUF_SIZE (256u*1024u)
#define BUF_SIZE_MASK (BUF_SIZE - 1u)
   float delay_buf[BUF_SIZE];
   uint32_t delay_buf_idx;
   float last_dly_val;

	float sample_rate;

	TunedDelayLine() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
      delay_buf_idx = 0u;
      ::memset((void*)delay_buf, 0, sizeof(delay_buf));
      handleSampleRateChanged();
      last_dly_val = 0.0f;
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


void TunedDelayLine::step() {

   // Calculate delay length
   float dlySmpOff;

   if(inputs[VOCT_INPUT].active)
   {
      // (note) Freq calculation borrowed from Fundamental.VCO
      float pitch = inputs[VOCT_INPUT].value + params[FINETUNE_PARAM].value * (1.0f / 12.0f);
      // Note C4
      float freq = 261.626f * powf(2.0f, pitch);

      dlySmpOff = (1.0f * sample_rate) / freq;
   }
   else
   {
      // No input connected, set delay time in the range 0..1 seconds
      dlySmpOff = sample_rate * (0.5f + 0.5f * params[FINETUNE_PARAM].value);
   }
   
   // Read delayed sample from ring buffer
#ifdef USE_FRAC
   uint32_t dlySmpOffI = uint32_t(dlySmpOff);
   float dlySmpFrac = dlySmpOff - dlySmpOffI;
   dlySmpOffI = (delay_buf_idx - dlySmpOffI) & BUF_SIZE_MASK;
   float dlyVal = delay_buf[dlySmpOffI] + (delay_buf[(dlySmpOffI+1u) & BUF_SIZE_MASK] - delay_buf[dlySmpOffI]) * dlySmpFrac;
#else
   uint32_t dlySmpOffI = uint32_t(delay_buf_idx - dlySmpOff) & BUF_SIZE_MASK;
   float dlyVal = delay_buf[dlySmpOffI];
#endif

   bool bPostFBOnly = (params[POSTFB_PARAM].value >= 0.5f);

   // Add input signal
   float inSmp = inputs[AUDIO_INPUT].value;

   if(bPostFBOnly)
   {
      dlyVal += inSmp;
   }

   // Send it to external module(s)
   outputs[FB_SEND_OUTPUT].value = dlyVal;

   float fbVal;

   // Read back processed feedback value
   if(inputs[FB_RET_INPUT].active)
   {
      // Use externally processed feedback sample
      // (note) this is actually shifted / delayed by one sample
      fbVal = inputs[FB_RET_INPUT].value;
   }
   else
   {
      // Fallback: feedback send+return not connected, use builtin filter instead
      fbVal = (last_dly_val + dlyVal) * 0.5f;
      last_dly_val = dlyVal;
   }

   // Apply feedback amount
   float fbAmt = params[FB_AMT_PARAM].value;
   fbAmt = 1.0f - fbAmt;
   fbAmt *= fbAmt;
   fbAmt *= fbAmt;
   fbAmt = 1.0f - fbAmt;
   fbVal *= fbAmt;

   if(!bPostFBOnly)
   {
      // Add input signal
      fbVal += inSmp;
   }

   // Write new delay sample to ring buffer
   delay_buf[delay_buf_idx] = fbVal;
   delay_buf_idx = (delay_buf_idx + 1u) & BUF_SIZE_MASK;

   // Final output
   float outVal;
   if(bPostFBOnly)
   {
      outVal = inSmp + (fbVal - inSmp) * params[DRYWET_PARAM].value;
   }
   else
   {
      outVal = inSmp + (dlyVal - inSmp) * params[DRYWET_PARAM].value;
   }
	outputs[AUDIO_OUTPUT].value = outVal;

#if 0
   static int xxx = 0;
   if(0 == (++xxx & 32767))
   {
      printf("xxx V/OCT=%f freq=%f inSmp=%f dlySmpOff=%f dlyVal=%f fbVal=%f outVal=%f fbAmt=%f\n", inputs[VOCT_INPUT].value, freq, inSmp, dlySmpOff, dlyVal, fbVal, outVal, fbAmt);
   }
#endif

}


struct TunedDelayLineWidget : ModuleWidget {
	TunedDelayLineWidget(TunedDelayLine *module);
};

TunedDelayLineWidget::TunedDelayLineWidget(TunedDelayLine *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/TunedDelayLine.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

   float cx;
   float cy;

   cx = 9.0f;
   cy = 37.0f;
	addInput(Port::create<PJ301MPort>(Vec(cx+2.0f, cy), Port::INPUT, module, TunedDelayLine::VOCT_INPUT));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy + 32), module, TunedDelayLine::FINETUNE_PARAM, -1.0f, 1.0f, 0.0f));

#define STY 32.0f
   cx = 11.0f;
   cy = 120.0f;
	addOutput(Port::create<PJ301MPort>(Vec(cx, cy), Port::OUTPUT, module, TunedDelayLine::FB_SEND_OUTPUT));
   cy += STY;
	addInput(Port::create<PJ301MPort>(Vec(cx, cy), Port::INPUT, module, TunedDelayLine::FB_RET_INPUT));
   cy += STY;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx-2.0f, cy), module, TunedDelayLine::FB_AMT_PARAM, 0.0f, 1.0f, 0.3f));
#undef STY

   cx = 16.0f;
   cy = 218.0f;
	addParam(ParamWidget::create<CKSS>(Vec(cx, cy), module, TunedDelayLine::POSTFB_PARAM, 0.0f, 1.0f, 1.0f));

   cx = 9.0f;
   cy = 245.0f;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, TunedDelayLine::DRYWET_PARAM, 0.0f, 1.0f, 1.0f));

#define STY 40.0f
   cx = 11.0f;
   cy = 325.0f;
	addInput(Port::create<PJ301MPort>(Vec(cx, cy - STY), Port::INPUT, module, TunedDelayLine::AUDIO_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(cx, 325), Port::OUTPUT, module, TunedDelayLine::AUDIO_OUTPUT));
#undef STY
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, TunedDelayLine) {
   Model *modelTunedDelayLine = Model::create<TunedDelayLine, TunedDelayLineWidget>("bsp", "TunedDelayLine", "Tuned Delay Line", ATTENUATOR_TAG, MIXER_TAG);
   return modelTunedDelayLine;
}
