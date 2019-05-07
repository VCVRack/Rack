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

namespace rack_plugin_bsp {

struct BeatFreq : Module {
	enum ParamIds {
		OCT_PARAM,
		SEMI_PARAM,
		CENT_PARAM,
		BEATFREQ_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
      CV_INPUT,
      OCT_INPUT,
      SEMI_INPUT,
      CENT_INPUT,
      BEATFREQ_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		NUM_OUTPUTS
	};

	BeatFreq() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
   }

	void step() override;
};


void BeatFreq::step() {

   float cv = inputs[CV_INPUT].value;
   float f;

   // Octave shift (additive modulation)
   int oct = int(params[OCT_PARAM].value);
   if(inputs[OCT_INPUT].active)
   {
      f = inputs[OCT_INPUT].value * (4.0f / 5.0f);
      if(f < 0.0f)
      {
         oct += int(f - 0.5f);
      }
      else
      {
         oct += int(f + 0.5f);
      }
   }
   cv += oct;

   // Semitone shift (additive modulation)
   int semi = int(params[SEMI_PARAM].value);
   if(inputs[SEMI_INPUT].active)
   {
      f = inputs[SEMI_INPUT].value * (12.0f / 5.0f);
      if(f < 0.0f)
      {
         semi += int(f - 0.5f);
      }
      else
      {
         semi += int(f + 0.5f);
      }
   }
   cv += semi * (1.0f / 12.0f);

   // Cent shift (scaled modulation)
   float cent = params[CENT_PARAM].value;
   if(inputs[CENT_INPUT].active)
   {
      f = inputs[CENT_INPUT].value * (1.0f / 5.0f);
      cent *= f;
   }
   cv += cent * (1.0f / 1200.0f);

   // Beat Frequency (scaled modulation)
   sF32 beatFreq = params[BEATFREQ_PARAM].value;
   if(inputs[BEATFREQ_INPUT].active)
   {
      f = inputs[BEATFREQ_INPUT].value * (1.0f / 5.0f);
      beatFreq *= f;
   }

   beatFreq *= (1.0f / 100.0f);  // => -1..1
   beatFreq = beatFreq * beatFreq * beatFreq;  // increase fine tuning precision (around 0)
   beatFreq *= (1.0f / 12.0f);
   cv += beatFreq / powf(2.0f, cv);

   outputs[CV_OUTPUT].value = cv;
}


struct BeatFreqWidget : ModuleWidget {
	BeatFreqWidget(BeatFreq *module);
};

BeatFreqWidget::BeatFreqWidget(BeatFreq *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/BeatFreq.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

   // CV input
	addInput(Port::create<PJ301MPort>(Vec(11.0f, 40.0f), Port::INPUT, module, BeatFreq::CV_INPUT));

#define STY_KNOB 27.0f
#define STY_PORT 35.0f
   float cxk = 5.0f;
   float cxp = 4.0f;
   float cy = 73.0f;

   // Octave
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cxk, cy), module, BeatFreq::OCT_PARAM, -4.0f, 4.0f, 0.0f));
   cy += STY_KNOB;

   // Octave modulation
	addInput(Port::create<PJ301MPort>(Vec(cxp, cy), Port::INPUT, module, BeatFreq::OCT_INPUT));
   cy += STY_PORT;

   // Semitones
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cxk, cy), module, BeatFreq::SEMI_PARAM, -12.0f, 12.0f, 0.0f));
   cy += STY_KNOB;

   // Semitones modulation
	addInput(Port::create<PJ301MPort>(Vec(cxp, cy), Port::INPUT, module, BeatFreq::SEMI_INPUT));
   cy += STY_PORT;

   // Cents
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cxk, cy), module, BeatFreq::CENT_PARAM, -100.0f, 100.0f, 0.0f));
   cy += STY_KNOB;

   // Cents modulation
	addInput(Port::create<PJ301MPort>(Vec(cxp, cy), Port::INPUT, module, BeatFreq::CENT_INPUT));
   cy += STY_PORT;

   // BeatFreq
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cxk, cy), module, BeatFreq::BEATFREQ_PARAM, -100.0f, 100.0f, 0.0f));
   cy += STY_KNOB;

   // BeatFreq modulation
	addInput(Port::create<PJ301MPort>(Vec(cxp, cy), Port::INPUT, module, BeatFreq::BEATFREQ_INPUT));
   cy += STY_PORT;

   // CV Output
   cy = 320.0f;
	addOutput(Port::create<PJ301MPort>(Vec(11, 325), Port::OUTPUT, module, BeatFreq::CV_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, BeatFreq) {
   Model *modelBeatFreq = Model::create<BeatFreq, BeatFreqWidget>("bsp", "BeatFreq", "BeatFreq", TUNER_TAG);
   return modelBeatFreq;
}
