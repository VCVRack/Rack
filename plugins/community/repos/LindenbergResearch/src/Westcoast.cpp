#include "dsp/SergeWavefolder.hpp"
#include "dsp/Lockhart.hpp"
#include "LindenbergResearch.hpp"

namespace rack_plugin_LindenbergResearch {

struct Westcoast : LRModule {

    enum ParamIds {
        GAIN_PARAM,
        CV_GAIN_PARAM,
        CV_BIAS_PARAM,
        BIAS_PARAM,
        TYPE_PARAM,
        DCBLOCK_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        SHAPER_INPUT,
        CV_GAIN_INPUT,
        CV_BIAS_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        SHAPER_OUTPUT,
        SG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };


    Westcoast() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    dsp::LockhartWavefolder *hs = new dsp::LockhartWavefolder(engineGetSampleRate());
    dsp::SergeWavefolder *sg = new dsp::SergeWavefolder(engineGetSampleRate());

    void step() override;
    void onSampleRateChange() override;
};


void Westcoast::step() {
    hs->setGain((params[GAIN_PARAM].value));
    hs->setBias(params[BIAS_PARAM].value);
    hs->setIn(inputs[SHAPER_INPUT].value);

    sg->setGain((params[GAIN_PARAM].value));
    sg->setBias(params[BIAS_PARAM].value);
    sg->setIn(inputs[SHAPER_INPUT].value);


    if (params[DCBLOCK_PARAM].value == 1)
        hs->setBlockDC(true);
    else
        hs->setBlockDC(false);

    hs->process();
    sg->process();

    // [bsp] let's hear what this sounds like :-)
    //  (note) too much gain makes all following modules go quiet permanently
    float hsOut = hs->getOut();
    float sgOut = sg->getOut();

    switch((int)params[TYPE_PARAM].value) {
       default:
       case -2:
          if(params[GAIN_PARAM].value < 9.8f)
             outputs[SHAPER_OUTPUT].value = hsOut;
          else
             outputs[SHAPER_OUTPUT].value = 0.0f;
          break;

       case -1:
          if(params[GAIN_PARAM].value < 15.8f)
             outputs[SHAPER_OUTPUT].value = sgOut;
          else
             outputs[SHAPER_OUTPUT].value = 0.0f;
          break;
    }

    outputs[SG_OUTPUT].value = sgOut;
}


void Westcoast::onSampleRateChange() {
    Module::onSampleRateChange();
    hs->setSamplerate(engineGetSampleRate());
    sg->setSamplerate(engineGetSampleRate());
}


struct WestcoastWidget : LRModuleWidget {
    WestcoastWidget(Westcoast *module);
};


WestcoastWidget::WestcoastWidget(Westcoast *module) : LRModuleWidget(module) {
    panel = new LRPanel();
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Westcoast.svg")));
    addChild(panel);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewLight>(Vec(15, 1)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewLight>(Vec(15, 366)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    addParam(LRKnob::create<LRAlternateBigKnob>(Vec(128.7, 63.0), module, Westcoast::GAIN_PARAM, 0.25, 20.f, 1.f));
    addParam(LRKnob::create<LRAlternateMiddleKnob>(Vec(136.4, 153.3), module, Westcoast::BIAS_PARAM, -0.5f, 0.5f, 0.f));
    addParam(LRKnob::create<LRMiddleIncremental>(Vec(85, 274.3), module, Westcoast::TYPE_PARAM, -3, 3, 0));

    addParam(LRKnob::create<LRAlternateSmallKnob>(Vec(83.4, 101.00), module, Westcoast::CV_GAIN_PARAM, -1.f, 1.f, 0.f));
    addParam(LRKnob::create<LRAlternateSmallKnob>(Vec(83.4, 183.0), module, Westcoast::CV_BIAS_PARAM, -1.f, 1.f, 0.f));
    // ***** MAIN KNOBS ******

    // ***** CV INPUTS *******
    addInput(Port::create<LRIOPortC>(Vec(32.4, 99.0), Port::INPUT, module, Westcoast::CV_GAIN_INPUT));
    addInput(Port::create<LRIOPortC>(Vec(32.4, 179.8), Port::INPUT, module, Westcoast::CV_BIAS_INPUT));
    // ***** CV INPUTS *******

    // ***** INPUTS **********
    addInput(Port::create<LRIOPortC>(Vec(22.4, 324.6), Port::INPUT, module, Westcoast::SHAPER_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPortC>(Vec(159.4, 324.6), Port::OUTPUT, module, Westcoast::SHAPER_OUTPUT));
    //  addOutput(Port::create<LRIOPortC>(Vec(159.4, 300.7), Port::OUTPUT, module, Westcoast::SG_OUTPUT));
    // ***** OUTPUTS *********

    // ***** SWITCH  *********
    // addParam(ParamWidget::create<LRSwitch>(Vec(119, 331), module, Westcoast::DCBLOCK_PARAM, 0.0, 1.0, 1.0));
    // ***** SWITCH  *********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, Westcoast) {
   Model *modelWestcoast = Model::create<Westcoast, WestcoastWidget>("Lindenberg Research", "Westcoast VCS",
                                                                  "Westcoast Complex Shaper", WAVESHAPER_TAG);
   return modelWestcoast;
}
