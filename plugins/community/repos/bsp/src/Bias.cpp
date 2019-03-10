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

struct Bias : Module {
	enum ParamIds {
		CTR_PARAM,
		NEG_PARAM,
		POS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
      CTL_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CTL_OUTPUT,
		NUM_OUTPUTS
	};

	Bias() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
   }

	void step() override;
};


void Bias::step() {

   // Read ctl input
   float inVal = inputs[CTL_INPUT].value;
   float ctrVal = params[CTR_PARAM].value;
   float outVal;

   if(inVal < ctrVal)
   {
      outVal = inVal - ctrVal;
      outVal *= params[NEG_PARAM].value;
      outVal += ctrVal;
   }
   else
   {
      outVal = inVal - ctrVal;
      outVal *= params[POS_PARAM].value;
      outVal += ctrVal;
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


struct BiasWidget : ModuleWidget {
	BiasWidget(Bias *module);
};

BiasWidget::BiasWidget(Bias *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Bias.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

   float cx;
   float cy;

#define STY 30.0f
   cx = 12.0f;
   cy = 50.0f;
	addInput(Port::create<PJ301MPort>(Vec(11.0f, cy), Port::INPUT, module, Bias::CTL_INPUT));
   cy += STY;
#undef STY

   cy = 115.0f;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Bias::CTR_PARAM, -10.0f, 10.0f, 0.0f));

   cy = 175.0f;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Bias::NEG_PARAM, -4.0f, 4.0f, 1.0f));

   cy = 235.0f;
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cy), module, Bias::POS_PARAM, -4.0f, 4.0f, 1.0f));

	addOutput(Port::create<PJ301MPort>(Vec(11, 325), Port::OUTPUT, module, Bias::CTL_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, Bias) {
   Model *modelBias = Model::create<Bias, BiasWidget>("bsp", "Bias", "Bias", UTILITY_TAG);
   return modelBias;
}
