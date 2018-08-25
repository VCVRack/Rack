#include "dsp/Overdrive.hpp"
#include "dsp/Hardclip.hpp"
#include "dsp/RShaper.hpp"
#include "dsp/Serge.hpp"
#include "dsp/Lockhart.hpp"
#include "dsp/Saturator.hpp"
#include "LindenbergResearch.hpp"

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
       hs = new dsp::LockhartWavefolder(engineGetSampleRate());
       sg = new dsp::SergeWavefolder(engineGetSampleRate());
       saturator = new dsp::Saturator(engineGetSampleRate());
       hardclip = new dsp::Hardclip(engineGetSampleRate());
       reshaper = new dsp::ReShaper(engineGetSampleRate());
       overdrive = new dsp::Overdrive(engineGetSampleRate());
    }

    dsp::LockhartWavefolder *hs;
    dsp::SergeWavefolder *sg;
    dsp::Saturator *saturator;
    dsp::Hardclip *hardclip;
    dsp::ReShaper *reshaper;
    dsp::Overdrive *overdrive;
    LRAlternateBigKnob *gainBtn = NULL;
    LRAlternateMiddleKnob *biasBtn = NULL;

    LRPanel *patina;

    void step() override;
    void onSampleRateChange() override;


    json_t *toJson() override {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "agedmode", json_boolean(patina->visible));
        return rootJ;
    }


    void fromJson(json_t *rootJ) override {
        json_t *agedmodeJ = json_object_get(rootJ, "agedmode");
        if (agedmodeJ)
            patina->visible = json_boolean_value(agedmodeJ);
    }
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

        case OVERDRIVE: // ReShaper
            overdrive->setGain(gain);
            overdrive->setBias(bias);
            overdrive->setIn(inputs[SHAPER_INPUT].value);

            overdrive->process();
            out = (float) overdrive->getOut();
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
    void appendContextMenu(Menu *menu) override;
};


WestcoastWidget::WestcoastWidget(Westcoast *module) : LRModuleWidget(module) {
    panel = new LRPanel();
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Westcoast.svg")));
    addChild(panel);

    module->patina = new LRPanel();
    module->patina->setBackground(SVG::load(assetPlugin(plugin, "res/WestcoastAged.svg")));
    module->patina->visible = false;
    addChild(module->patina);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewLight>(Vec(15, 1)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewLight>(Vec(15, 366)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    module->gainBtn = LRKnob::create<LRAlternateBigKnob>(Vec(128.7, 63.0), module, Westcoast::GAIN_PARAM, 0.0, 20.f, 1.f);
    module->biasBtn = LRKnob::create<LRAlternateMiddleKnob>(Vec(136.4, 153.3), module, Westcoast::BIAS_PARAM, -6.f, 6.f, 0.f);

    addParam(module->gainBtn);
    addParam(module->biasBtn);

    addParam(LRKnob::create<LRMiddleIncremental>(Vec(85, 279.3), module, Westcoast::TYPE_PARAM, 1, 7, 1));

    addParam(LRKnob::create<LRAlternateSmallKnob>(Vec(83.4, 101.00), module, Westcoast::CV_GAIN_PARAM, -1.f, 1.f, 0.f));
    addParam(LRKnob::create<LRAlternateSmallKnob>(Vec(83.4, 183.0), module, Westcoast::CV_BIAS_PARAM, -1.f, 1.f, 0.f));
    // ***** MAIN KNOBS ******

    // ***** CV INPUTS *******
    addInput(Port::create<LRIOPortC>(Vec(32.4, 99.0), Port::INPUT, module, Westcoast::CV_GAIN_INPUT));
    addInput(Port::create<LRIOPortC>(Vec(32.4, 179.8), Port::INPUT, module, Westcoast::CV_BIAS_INPUT));
    // ***** CV INPUTS *******

    // ***** INPUTS **********
    addInput(Port::create<LRIOPortC>(Vec(22.4, 332.6), Port::INPUT, module, Westcoast::SHAPER_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPortC>(Vec(159.4, 332.6), Port::OUTPUT, module, Westcoast::SHAPER_OUTPUT));
    // ***** OUTPUTS *********

    // ***** SWITCH  *********
    //addParam(ParamWidget::create<LRSwitch>(Vec(119, 331), module, Westcoast::DCBLOCK_PARAM, 0.0, 1.0, 1.0));
    // ***** SWITCH  *********
}


struct WestcoastShowPatina : MenuItem {
    Westcoast *westcoast;


    void onAction(EventAction &e) override {
        westcoast->patina->visible ^= true;
    }


    void step() override {
        rightText = CHECKMARK(westcoast->patina->visible);
    }
};


void WestcoastWidget::appendContextMenu(Menu *menu) {
    menu->addChild(MenuEntry::create());

    auto *westcoast = dynamic_cast<Westcoast *>(module);
    assert(westcoast);

    auto *mergeItem = MenuItem::create<WestcoastShowPatina>("Aged Look");
    mergeItem->westcoast = westcoast;
    menu->addChild(mergeItem);
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, Westcoast) {
   Model *modelWestcoast = Model::create<Westcoast, WestcoastWidget>("Lindenberg Research", "Westcoast VCS",
                                                                  "Westcoast Complex Shaper", WAVESHAPER_TAG);
   return modelWestcoast;
}
