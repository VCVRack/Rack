#include <iostream>
#include <stdlib.h>
#include <random>
#include <cmath>

#include "dsp/digital.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/filter.hpp"

#include "RJModules.hpp"
#include "VAStateVariableFilter.h"

namespace rack_plugin_RJModules {

#define HISTORY_SIZE (1<<21)

struct Widener : Module {
    enum ParamIds {
        TIME_PARAM,
        MIX_PARAM,
        FILTER_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        TIME_CV_INPUT,
        MIX_CV_INPUT,
        FILTER_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer;
    DoubleRingBuffer<float, 16> outBuffer;
    SampleRateConverter<1> src;

    float low = 99;
    float high = 0;
    float next;
    float mapped_pink = 0.0;
    float white = 0.0;
    float mixed = 0.0;
    float mix_value = 1.0;

    std::random_device rd; // obtain a random number from hardware

    float outLP;
    float outHP;

    VAStateVariableFilter *lpFilter = new VAStateVariableFilter() ; // create a lpFilter;
    VAStateVariableFilter *hpFilter = new VAStateVariableFilter() ; // create a hpFilter;

    Widener() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void Widener::step(){

  float in = inputs[CH1_INPUT].value;

  // Compute delay time in seconds
  float delay = .001 * powf(10.0 / .001, clamp(params[TIME_PARAM].value + inputs[TIME_CV_INPUT].value / 10.0f, 0.0f, 1.0f));
  // Number of delay samples
  float index = delay * engineGetSampleRate();

  if (!historyBuffer.full()) {
      historyBuffer.push(in);
  }

  // How many samples do we need consume to catch up?
  float consume = index - historyBuffer.size();
  if (outBuffer.empty()) {
      double ratio = 1.0;
      if (consume <= -16)
          ratio = 0.5;
      else if (consume >= 16)
          ratio = 2.0;

      float inSR = engineGetSampleRate();
      float outSR = ratio * inSR;

      int inFrames = min(historyBuffer.size(), 16);
      int outFrames = outBuffer.capacity();
      src.setRates(inSR, outSR);
      src.process((const Frame<1>*)historyBuffer.startData(), &inFrames, (Frame<1>*)outBuffer.endData(), &outFrames);
      historyBuffer.startIncr(inFrames);
      outBuffer.endIncr(outFrames);
  }

  float wet = 0.0;
  if (!outBuffer.empty()) {
      wet = outBuffer.shift();
  }


  // filter the wet
  lpFilter->setFilterType(0);
  hpFilter->setFilterType(2);

  lpFilter->setResonance(.7);
  hpFilter->setResonance(.7);

  lpFilter->setSampleRate(engineGetSampleRate());
  hpFilter->setSampleRate(engineGetSampleRate());

  float param = params[FILTER_PARAM].value * clamp(inputs[FILTER_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

  if(param < .5){
      // new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
      float lp_cutoff = ( (param - 0) / (.5 - 0.0)) * (8000.0 - 30.0) + 30.0;
      lpFilter->setCutoffFreq(lp_cutoff);
      wet = lpFilter->processAudioSample(wet, 1);
  }
  if(param > .5){
      // new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
      float hp_cutoff = ( (param - .5) / (1.0 - 0.5)) * (8000.0 - 200.0) + 200.0;
      hpFilter->setCutoffFreq(hp_cutoff);
      wet = hpFilter->processAudioSample(wet, 1);
  }
  // if(param == .5){
  //     wet = wet;
  // }

  //mix
  float mix_percent = params[MIX_PARAM].value * clamp(inputs[MIX_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
  float mixed = ((wet * mix_percent)) + (in * (1-mix_percent));

  outputs[CH1_OUTPUT].value = in;
  outputs[CH2_OUTPUT].value = mixed;

}

struct WidenerWidget: ModuleWidget {
    WidenerWidget(Widener *module);
};

WidenerWidget::WidenerWidget(Widener *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Widener.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    // addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 61), module, Widener::TIME_PARAM, 0.0, 0.7, 0.35));
    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 143), module, Widener::MIX_PARAM, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 228), module, Widener::FILTER_PARAM, 0.0, 1.0, 0.5));

    addInput(Port::create<PJ301MPort>(Vec(22, 100), Port::INPUT, module, Widener::TIME_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 190), Port::INPUT, module, Widener::MIX_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 270), Port::INPUT, module, Widener::FILTER_CV_INPUT));

    addInput(Port::create<PJ301MPort>(Vec(22, 315), Port::INPUT, module, Widener::CH1_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(62, 315), Port::OUTPUT, module, Widener::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(100, 315), Port::OUTPUT, module, Widener::CH2_OUTPUT));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Widener) {
   Model *modelWidener = Model::create<Widener, WidenerWidget>("RJModules", "Widener", "[FX] Widener", UTILITY_TAG);
   return modelWidener;
}
