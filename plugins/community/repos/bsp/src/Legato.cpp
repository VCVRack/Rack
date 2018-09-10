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
#include "dsp/digital.hpp"

namespace rack_plugin_bsp {

struct Legato : Module {
	enum ParamIds {
		DECAY_RATE_PARAM,
		SMOOTH_MIN_RISE_PARAM,
		SMOOTH_MIN_FALL_PARAM,
		SMOOTH_MAX_RISE_PARAM,
		SMOOTH_MAX_FALL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
      CTL_INPUT,
      TRIG_INPUT,
      RATE_MOD_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CTL_OUTPUT,
		NUM_OUTPUTS
	};

   double smoothed_sign;
   double last_smoothed_val;
   double smoothed_val;
   double decay_t;

	SchmittTrigger trigger;

   double rcp_sample_rate;

	Legato() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
      smoothed_sign = 0.0;
      last_smoothed_val = 0.0;
      smoothed_val = 0.0;
      decay_t = 0.0;
      handleSampleRateChanged();
   }

   void handleSampleRateChanged(void) {
      rcp_sample_rate = 1.0 / double(engineGetSampleRate());
   }

   void onSampleRateChange() override {
      Module::onSampleRateChange();

      handleSampleRateChanged();
   }

	void step() override;
};


void Legato::step() {

   // Read ctl input
   float inVal = inputs[CTL_INPUT].value;

	if(trigger.process(inputs[TRIG_INPUT].value))
   {
      decay_t = 0.0;
      smoothed_sign = 0.0f;
   }
   else
   {
      double dcyR = params[DECAY_RATE_PARAM].value;

      if(inputs[RATE_MOD_INPUT].active)
      {
         dcyR += inputs[RATE_MOD_INPUT].value * (1.0 / 5);
         if(dcyR < 0.0)
            dcyR = 0.0;
         else if(dcyR > 1.0)
            dcyR = 1.0;
      }

      dcyR *= dcyR;
      dcyR *= dcyR;
      dcyR *= 4000.0f;
      dcyR += 1.0f;
      dcyR *= rcp_sample_rate; // divide by sample rate
      decay_t += dcyR;
      if(decay_t >= 1.0)
         decay_t = 1.0;
   }

   double smoothAmt;

   if(smoothed_sign >= 0.0f)
   {
      smoothAmt = params[SMOOTH_MIN_RISE_PARAM].value + (params[SMOOTH_MAX_RISE_PARAM].value - params[SMOOTH_MIN_RISE_PARAM].value) * decay_t;
   }
   else
   {
      smoothAmt = params[SMOOTH_MIN_FALL_PARAM].value + (params[SMOOTH_MAX_FALL_PARAM].value - params[SMOOTH_MIN_FALL_PARAM].value) * decay_t;
   }

   smoothAmt = (1.0 - smoothAmt);
   smoothAmt *= smoothAmt;
   smoothAmt *= smoothAmt;
   smoothAmt *= smoothAmt;

   smoothed_val = smoothed_val + (inVal - smoothed_val) * smoothAmt;

   smoothed_sign = (smoothed_val - last_smoothed_val);
   last_smoothed_val = smoothed_val;

   outputs[CTL_OUTPUT].value = float(smoothed_val);

#if 0
   static int xxx = 0;
   if(0 == (++xxx & 32767))
   {
      printf("xxx smoothAmt=%g decay_t=%g\n", smoothAmt, decay_t);
   }
#endif
}


struct LegatoWidget : ModuleWidget {
	LegatoWidget(Legato *module);
};

LegatoWidget::LegatoWidget(Legato *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Legato.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

   float cx;
   float cy;

#define STY 30.0f
   cx = 12.0f;
   cy = 50.0f;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, Legato::CTL_INPUT));
   cy += STY;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, Legato::TRIG_INPUT));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Legato::DECAY_RATE_PARAM, 0.0f, 1.0f, 0.2f));
   cy += STY;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, Legato::RATE_MOD_INPUT));
#undef STY

#define STY 30.0f
   cx = 12.0f;
   cy = 185.0f;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Legato::SMOOTH_MIN_RISE_PARAM, 0.0f, 1.0f, 0.0f));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Legato::SMOOTH_MIN_FALL_PARAM, 0.0f, 1.0f, 0.0f));

   cy += 10.0f;
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Legato::SMOOTH_MAX_RISE_PARAM, 0.0f, 1.0f, 0.6f));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Legato::SMOOTH_MAX_FALL_PARAM, 0.0f, 1.0f, 0.6f));
#undef STX
#undef STY

	addOutput(Port::create<PJ301MPort>(Vec(11, 325), Port::OUTPUT, module, Legato::CTL_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, Legato) {
   Model *modelLegato = Model::create<Legato, LegatoWidget>("bsp", "Legato", "Legato", SLEW_LIMITER_TAG, UTILITY_TAG);
   return modelLegato;
}
