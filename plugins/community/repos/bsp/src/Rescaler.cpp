/*
Copyright (c) 2019 bsp

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

struct Rescaler : Module {
	enum ParamIds {
		IN_MIN_PARAM,
		IN_MAX_PARAM,
		OUT_MIN_PARAM,
		OUT_MAX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
      CTL_INPUT,
      SCALE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CTL_OUTPUT,
		NUM_OUTPUTS
	};

	Rescaler() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
   }

	void step() override;
};


void Rescaler::step() {

   // Read ctl input
   float inVal = inputs[CTL_INPUT].value;

   float t;
   float inMin = params[IN_MIN_PARAM].value;
   float inMax = params[IN_MAX_PARAM].value;
   if(inMin > inMax)
   {
      t = inMin;
      inMin = inMax;
      inMax = t;
   }

   float outMin = params[OUT_MIN_PARAM].value;
   float outMax = params[OUT_MAX_PARAM].value;
   if(outMin > outMax)
   {
      t = outMin;
      outMin = outMax;
      outMax = t;
   }

   // Clip input to min..max range
   if(inVal < inMin)
      inVal = inMin;
   else if(inVal > inMax)
      inVal = inMax;

   float outVal;

   if((inMax > inMin) && (outMax > outMin))
   {
      // Rescale to output range
      outVal = (inVal - inMin) / (inMax - inMin);
      outVal = (outVal * (outMax - outMin)) + outMin;
   }
   else
   {
      // In or out range is zero
      outVal = outMin;
   }

   // Scale by secondary input (if connected)
   if(inputs[SCALE_INPUT].active)
   {
      outVal *= inputs[SCALE_INPUT].value;
   }

   outputs[CTL_OUTPUT].value = outVal;

#if 0
   static int xxx = 0;
   if(0 == (++xxx & 32767))
   {
      printf("xxx outVal=%f\n", outVal);
   }
#endif
}


struct RescalerWidget : ModuleWidget {
	RescalerWidget(Rescaler *module);
};

RescalerWidget::RescalerWidget(Rescaler *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Rescaler.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

   float cx;
   float cy;

#define STY 30.0f
   cx = 12.0f;
   cy = 50.0f;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, Rescaler::CTL_INPUT));
   cy += STY;
#undef STY

#define STY 30.0f
   cx = 12.0f;
   cy = 110.0f;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Rescaler::IN_MIN_PARAM, -10.0f, 10.0f, 0.0f));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Rescaler::IN_MAX_PARAM, -10.0f, 10.0f, 10.0f));

   cy = 195.0f;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Rescaler::OUT_MIN_PARAM, -10.0f, 10.0f, 0.0f));
   cy += STY;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Rescaler::OUT_MAX_PARAM, -10.0f, 10.0f, 10.0f));
#undef STX
#undef STY

   cy = 280.0f;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, Rescaler::SCALE_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(11, 325), Port::OUTPUT, module, Rescaler::CTL_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, Rescaler) {
   Model *modelRescaler = Model::create<Rescaler, RescalerWidget>("bsp", "Rescaler", "Rescaler", UTILITY_TAG);
   return modelRescaler;
}
