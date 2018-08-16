#include "dsp/Serge.hpp"
#include "dsp/Lockhart.hpp"
#include "LindenbergResearch.hpp"

namespace rack_plugin_LindenbergResearch {
using namespace lrt;

struct Westcoast : LRModule {

    enum RotaryStages {
        OVERDRIVE = 1,
        LOCKHART,
        SERGE,
        SATURATE,
        POLYNOM,
        SOFTCLIP,
        HARDCLIP
    };

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
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };


    Westcoast() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
       hs = new dsp::LockhartWavefolder(engineGetSampleRate());
       sg = new dsp::SergeWavefolder(engineGetSampleRate());
    }

    dsp::LockhartWavefolder *hs;
    dsp::SergeWavefolder *sg;
    LRAlternateBigKnob *gain;
    LRAlternateMiddleKnob *bias;

    void step() override;
    void onSampleRateChange() override;
};


void Westcoast::step() {

    outputs[SHAPER_OUTPUT].value = 0.0f;

    float gaincv = 0;
    float biascv = 0;

    if (inputs[CV_GAIN_INPUT].active) {
        gaincv = inputs[CV_GAIN_INPUT].value * quadraticBipolar(params[CV_GAIN_PARAM].value) * 4.0f;
    }

    if (inputs[CV_BIAS_INPUT].active) {
        biascv = inputs[CV_BIAS_INPUT].value * quadraticBipolar(params[CV_BIAS_PARAM].value) * 2.0f;
    }

    if (bias != nullptr && gain != nullptr) {
        gain->setIndicatorActive(inputs[CV_GAIN_INPUT].active);
        bias->setIndicatorActive(inputs[CV_BIAS_INPUT].active);

        gain->setIndicatorValue((params[GAIN_PARAM].value + gaincv) / 20);
        bias->setIndicatorValue((params[BIAS_PARAM].value + (biascv + 3)) / 6);
    }

    float out;

    switch (lround(params[TYPE_PARAM].value)) {
        case LOCKHART: // Lockhart Model
            hs->setGain((params[GAIN_PARAM].value + gaincv));
            hs->setBias(params[BIAS_PARAM].value + biascv);
            hs->setIn(inputs[SHAPER_INPUT].value);
            hs->setBlockDC(params[DCBLOCK_PARAM].value == 1);

            hs->process();
            out = (float) hs->getOut();
            break;
        case SERGE: // Serge Model
            sg->setGain((params[GAIN_PARAM].value + gaincv));
            sg->setBias(params[BIAS_PARAM].value + biascv);
            sg->setIn(inputs[SHAPER_INPUT].value);
            sg->setBlockDC(params[DCBLOCK_PARAM].value == 1);

            sg->process();
            out = (float) sg->getOut();
            break;
        default: // invalid state, should not happen
            out = 0;
            break;
    }

    outputs[SHAPER_OUTPUT].value = out;
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
    module->gain = LRKnob::create<LRAlternateBigKnob>(Vec(128.7, 63.0), module, Westcoast::GAIN_PARAM, 0.0, 20.f, 1.f);
    module->bias = LRKnob::create<LRAlternateMiddleKnob>(Vec(136.4, 153.3), module, Westcoast::BIAS_PARAM, -6.f, 6.f, 0.f);

    addParam(module->gain);
    addParam(module->bias);

    addParam(LRKnob::create<LRMiddleIncremental>(Vec(85, 274.3), module, Westcoast::TYPE_PARAM, 1, 6, 1));

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
    // ***** OUTPUTS *********

    // ***** SWITCH  *********
    addParam(ParamWidget::create<LRSwitch>(Vec(119, 331), module, Westcoast::DCBLOCK_PARAM, 0.0, 1.0, 1.0));
    // ***** SWITCH  *********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, Westcoast) {
   Model *modelWestcoast = Model::create<Westcoast, WestcoastWidget>("Lindenberg Research", "Westcoast VCS",
                                                                  "Westcoast Complex Shaper", WAVESHAPER_TAG);
   return modelWestcoast;
}
