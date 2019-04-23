#include "../dsp/FastTanWF.hpp"
#include "../dsp/Overdrive.hpp"
#include "../dsp/Hardclip.hpp"
#include "../dsp/RShaper.hpp"
#include "../dsp/Serge.hpp"
#include "../dsp/Lockhart.hpp"
#include "../dsp/Saturator.hpp"
#include "../LindenbergResearch.hpp"
#include "../LRModel.hpp"

namespace rack_plugin_LindenbergResearch {
using namespace lrt;


struct Westcoast : LRModule {

    enum RotaryStages {
        SERGE = 1,
        LOCKHART,
        OVERDRIVE,
        SATURATE,
        RESHAPER,
        VALERIE,
        HARDCLIP
    };

    enum ParamIds {
        GAIN_PARAM,
        CV_GAIN_PARAM,
        CV_BIAS_PARAM,
        BIAS_PARAM,
        TYPE_PARAM,
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
        gainBtn = LRKnob::create<LRBigKnob>(Vec(128.7, 63.0), this, Westcoast::GAIN_PARAM, 0.0, 20.f, 1.f);
        biasBtn = LRKnob::create<LRMiddleKnob>(Vec(135.4, 152.3), this, Westcoast::BIAS_PARAM, -6.f, 6.f, 0.f);
    }


    dsp::LockhartWavefolder *hs = new dsp::LockhartWavefolder(engineGetSampleRate());
    dsp::SergeWavefolder *sg = new dsp::SergeWavefolder(engineGetSampleRate());
    dsp::Saturator *saturator = new dsp::Saturator(engineGetSampleRate());
    dsp::Hardclip *hardclip = new dsp::Hardclip(engineGetSampleRate());
    dsp::ReShaper *reshaper = new dsp::ReShaper(engineGetSampleRate());
    dsp::Overdrive *overdrive = new dsp::Overdrive(engineGetSampleRate());
    dsp::FastTan *fastTan = new dsp::FastTan(engineGetSampleRate());

    LRBigKnob *gainBtn = NULL;
    LRMiddleKnob *biasBtn = NULL;

    void step() override;
    void onSampleRateChange() override;
};


void Westcoast::step() {
    float gaincv = 0;
    float biascv = 0;

    /* not connected */
    if (!inputs[SHAPER_INPUT].active) {
        outputs[SHAPER_OUTPUT].value = 0.f;

        return;
    }

    if (inputs[CV_GAIN_INPUT].active) {
        gaincv = inputs[CV_GAIN_INPUT].value * quadraticBipolar(params[CV_GAIN_PARAM].value) * 4.0f;
    }

    if (inputs[CV_BIAS_INPUT].active) {
        biascv = inputs[CV_BIAS_INPUT].value * quadraticBipolar(params[CV_BIAS_PARAM].value) * 2.0f;
    }

    if (biasBtn != NULL && gainBtn != NULL) {
        gainBtn->setIndicatorActive(inputs[CV_GAIN_INPUT].active);
        biasBtn->setIndicatorActive(inputs[CV_BIAS_INPUT].active);

        gainBtn->setIndicatorValue((params[GAIN_PARAM].value + gaincv) / 20);
        biasBtn->setIndicatorValue((params[BIAS_PARAM].value + (biascv + 6)) / 12);
    }

    float out;
    float gain = params[GAIN_PARAM].value + gaincv;
    float bias = params[BIAS_PARAM].value + biascv;

    switch (lround(params[TYPE_PARAM].value)) {
        case LOCKHART:  // Lockhart Model
            hs->setGain(gain);
            hs->setBias(bias);
            hs->setIn(inputs[SHAPER_INPUT].value);

            hs->process();
            out = (float) hs->getOut();
            break;

        case SERGE:     // Serge Model
            sg->setGain(gain);
            sg->setBias(bias);
            sg->setIn(inputs[SHAPER_INPUT].value);

            sg->process();
            out = (float) sg->getOut();
            break;

        case SATURATE: // Saturator
            saturator->setGain(gain);
            saturator->setBias(bias);
            saturator->setIn(inputs[SHAPER_INPUT].value);

            saturator->process();
            out = (float) saturator->getOut();
            break;

        case HARDCLIP: // Hardclip
            hardclip->setGain(gain);
            hardclip->setBias(bias);
            hardclip->setIn(inputs[SHAPER_INPUT].value);

            hardclip->process();
            out = (float) hardclip->getOut();
            break;

        case RESHAPER: // ReShaper
            reshaper->setGain(gain);
            reshaper->setBias(bias);
            reshaper->setIn(inputs[SHAPER_INPUT].value);

            reshaper->process();
            out = (float) reshaper->getOut();
            break;

        case OVERDRIVE: // Overdrive
            overdrive->setGain(gain);
            overdrive->setBias(bias);
            overdrive->setIn(inputs[SHAPER_INPUT].value);

            overdrive->process();
            out = (float) overdrive->getOut();
            break;

        case VALERIE: // Overdrive
            fastTan->setGain(gain);
            fastTan->setBias(bias);
            fastTan->setIn(inputs[SHAPER_INPUT].value);

            fastTan->process();
            out = (float) fastTan->getOut();
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
    saturator->setSamplerate(engineGetSampleRate());
}


struct WestcoastWidget : LRModuleWidget {
    WestcoastWidget(Westcoast *module);
};


WestcoastWidget::WestcoastWidget(Westcoast *module) : LRModuleWidget(module) {
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/Westcoast.svg")));
    panel->addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/panels/WestcoastLight.svg")));
    panel->addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/panels/WestcoastAged.svg")));

    panel->init();
    addChild(panel);
    box.size = panel->box.size;

    //panel->gradients[DARK]->setInnerColor(nvgRGBAf(1.5f * .369f, 1.5f * 0.369f, 1.5f * 0.369f, 0.45f));
    //panel->gradients[DARK]->setOuterColor(nvgRGBAf(0.f, 0.f, 0.f, 0.25f));

    // panel->gradients[LIGHT]->setInnerColor(nvgRGBAf(1.5f * .369f, 1.5f * 0.369f, 1.5f * 0.369f, 0.45f));
    // panel->gradients[LIGHT]->setOuterColor(nvgRGBAf(0.f, 0.f, 0.f, 0.25f));

    // panel->gradients[AGED]->setInnerColor(nvgRGBAf(1.5f * .369f, 1.5f * 0.369f, 1.5f * 0.369f, 0.45f));
    // panel->gradients[AGED]->setOuterColor(nvgRGBAf(0.f, 0.f, 0.f, 0.25f));
    // ***** SCREWS **********
    addChild(Widget::create<ScrewLight>(Vec(15, 1)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewLight>(Vec(15, 366)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    addParam(module->gainBtn);
    addParam(module->biasBtn);

    auto *toggleknob = LRKnob::create<LRToggleKnob>(Vec(83.8, 277.8), module, Westcoast::TYPE_PARAM, 1, 7, 1);

    // calibrate toggle knob fpr 7 stages
    // TODO:
    toggleknob->minAngle = -0.5f * M_PI;

    addParam(toggleknob);

    addParam(LRKnob::create<LRSmallKnob>(Vec(83.4, 101.00), module, Westcoast::CV_GAIN_PARAM, -1.f, 1.f, 0.f));
    addParam(LRKnob::create<LRSmallKnob>(Vec(83.4, 183.0), module, Westcoast::CV_BIAS_PARAM, -1.f, 1.f, 0.f));
    // ***** MAIN KNOBS ******

    // ***** CV INPUTS *******
    addInput(Port::create<LRIOPortCV>(Vec(32.4, 99.0), Port::INPUT, module, Westcoast::CV_GAIN_INPUT));
    addInput(Port::create<LRIOPortCV>(Vec(32.4, 179.8), Port::INPUT, module, Westcoast::CV_BIAS_INPUT));
    // ***** CV INPUTS *******

    // ***** INPUTS **********
    addInput(Port::create<LRIOPortAudio>(Vec(22.4, 326.05), Port::INPUT, module, Westcoast::SHAPER_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPortAudio>(Vec(159.4, 326.05), Port::OUTPUT, module, Westcoast::SHAPER_OUTPUT));
    // ***** OUTPUTS *********

    // ***** SWITCH  *********
    //addParam(ParamWidget::create<LRSwitch>(Vec(119, 331), module, Westcoast::DCBLOCK_PARAM, 0.0, 1.0, 1.0));
    // ***** SWITCH  *********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, Westcoast) {
   Model *modelWestcoast = Model::create<Westcoast, WestcoastWidget>("Lindenberg Research", "Westcoast VCS",
                                                                  "Westcoast Complex Shaper", WAVESHAPER_TAG);
   return modelWestcoast;
}
