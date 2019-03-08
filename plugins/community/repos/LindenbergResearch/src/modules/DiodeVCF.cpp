#include "../LindenbergResearch.hpp"
#include "../dsp/DiodeLadder.hpp"
#include "../dsp/Hardclip.hpp"
#include "../LRModel.hpp"

namespace rack_plugin_LindenbergResearch {

using namespace rack;
using namespace lrt;

using dsp::DiodeLadderFilter;


struct DiodeVCF : LRModule {
    enum ParamIds {
        FREQUENCY_PARAM,
        RES_PARAM,
        SATURATE_PARAM,
        FREQUENCY_CV_PARAM,
        RESONANCE_CV_PARAM,
        SATURATE_CV_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        FILTER_INPUT,
        FREQUCENCY_CV_INPUT,
        RESONANCE_CV_INPUT,
        SATURATE_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        LP_OUTPUT,
        HP_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };


    DiodeVCF() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    void onRandomize() override;
    void updateComponents();

    LRLCDWidget *lcd = new LRLCDWidget(12, "%00004.3f Hz", LRLCDWidget::NUMERIC);
    DiodeLadderFilter *lpf = new DiodeLadderFilter(engineGetSampleRate());

    LRBigKnob *frqKnob = NULL;
    LRBigKnob *resKnob = NULL;
    LRMiddleKnob *saturateKnob = NULL;

    LRPanel *panel;

    bool aged = false;
    bool hidef = false;


    /* json_t *toJson() override {
         json_t *rootJ = LRModule::toJson();

         json_object_set_new(rootJ, "AGED", json_boolean(aged));
         json_object_set_new(rootJ, "hidef", json_boolean(hidef));
         return rootJ;
     }


     void fromJson(json_t *rootJ) override {
         LRModule::fromJson(rootJ);

         json_t *agedJ = json_object_get(rootJ, "AGED");
         if (agedJ)
             aged = json_boolean_value(agedJ);

         json_t *hidefJ = json_object_get(rootJ, "hidef");
         if (agedJ)
             hidef = json_boolean_value(hidefJ);

         updateComponents();
     }*/


    void step() override;
    void onSampleRateChange() override;
};


void DiodeVCF::step() {
    float freqcv = 0, rescv = 0, satcv = 0;

    if (inputs[FREQUCENCY_CV_INPUT].active) {
        freqcv = inputs[FREQUCENCY_CV_INPUT].value / 10 * quadraticBipolar(params[FREQUENCY_CV_PARAM].value);
    }


    if (inputs[RESONANCE_CV_INPUT].active) {
        rescv = inputs[RESONANCE_CV_INPUT].value / 10 * quadraticBipolar(params[RESONANCE_CV_PARAM].value);
    }

    if (inputs[SATURATE_CV_INPUT].active) {
        satcv = inputs[SATURATE_CV_INPUT].value / 10 * quadraticBipolar(params[SATURATE_CV_PARAM].value);
    }

    float frq = clamp(params[FREQUENCY_PARAM].value + freqcv, 0.f, 1.f);
    float res = clamp((params[RES_PARAM].value + rescv) * DiodeLadderFilter::MAX_RESONANCE, 0.f, DiodeLadderFilter::MAX_RESONANCE);
    float sat = clamp(quarticBipolar((params[SATURATE_PARAM].value) + satcv) * 14 + 1, 0.f, 15.f);

    if (frqKnob != nullptr && resKnob != nullptr && saturateKnob != nullptr) {
        frqKnob->setIndicatorActive(inputs[FREQUCENCY_CV_INPUT].active);
        resKnob->setIndicatorActive(inputs[RESONANCE_CV_INPUT].active);
        saturateKnob->setIndicatorActive(inputs[SATURATE_CV_INPUT].active);

        frqKnob->setIndicatorValue(params[FREQUENCY_PARAM].value + freqcv);
        resKnob->setIndicatorValue(params[RES_PARAM].value + rescv);
        saturateKnob->setIndicatorValue(params[SATURATE_PARAM].value + satcv);
    }

    lpf->setFrequency(frq);
    lpf->setResonance(res);
    lpf->setSaturation(sat);

    lpf->low = !hidef;

    lcd->value = lpf->getFreqHz();

    lpf->setIn(inputs[FILTER_INPUT].value / 10.f);
    lpf->invalidate();
    lpf->process();

    /* compensate gain drop on resonance inc.
    float q = params[RES_PARAM].value * 1.8f + 1;*/

    outputs[HP_OUTPUT].value = lpf->getOut2() * 6.5f;  // hipass
    outputs[LP_OUTPUT].value = lpf->getOut() * 10.f;   // lowpass
}


void DiodeVCF::updateComponents() {

}


void DiodeVCF::onSampleRateChange() {
    Module::onSampleRateChange();
    lpf->setSamplerate(engineGetSampleRate());
}


void DiodeVCF::onRandomize() {
    Module::randomize();
    // patina->box.pos = Vec(-randomUniform() * 1000, -randomUniform() * 200);

    updateComponents();
}


/**
 * @brief Blank Panel with Logo
 */
struct DiodeVCFWidget : LRModuleWidget {
    DiodeVCFWidget(DiodeVCF *module);
    void appendContextMenu(Menu *menu) override;
};


DiodeVCFWidget::DiodeVCFWidget(DiodeVCF *module) : LRModuleWidget(module) {
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/DiodeLadderVCFClassic.svg")));
    panel->addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/panels/DiodeLadderVCF.svg")));
    panel->addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/panels/DiodeLadderVCFAged.svg")));

    panel->init();
    addChild(panel);

    module->panel = panel;
    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewLight>(Vec(15, 1)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewLight>(Vec(15, 366)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    module->frqKnob = LRKnob::create<LRBigKnob>(Vec(32.5, 74.4), module, DiodeVCF::FREQUENCY_PARAM, 0.f, 1.f, 1.f);
    module->resKnob = LRKnob::create<LRBigKnob>(Vec(151.5, 74.4), module, DiodeVCF::RES_PARAM, 0.0f, 1.0, 0.0f);
    module->saturateKnob = LRKnob::create<LRMiddleKnob>(Vec(99.5, 164.4), module, DiodeVCF::SATURATE_PARAM, 0.f, 1.0,
                                                                  0.0f);

    module->frqKnob->setIndicatorColors(nvgRGBAf(0.9f, 0.9f, 0.9f, 1.0f));
    module->resKnob->setIndicatorColors(nvgRGBAf(0.9f, 0.9f, 0.9f, 1.0f));
    module->saturateKnob->setIndicatorColors(nvgRGBAf(0.9f, 0.9f, 0.9f, 1.0f));

    addParam(module->frqKnob);
    addParam(module->resKnob);
    addParam(module->saturateKnob);

    addParam(ParamWidget::create<LRSmallKnob>(Vec(39.9, 251.4), module, DiodeVCF::FREQUENCY_CV_PARAM, -1.f, 1.0f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(177, 251.4), module, DiodeVCF::RESONANCE_CV_PARAM, -1.f, 1.0f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(108.5, 251.4), module, DiodeVCF::SATURATE_CV_PARAM, -1.f, 1.0f, 0.f));
    // ***** MAIN KNOBS ******

    // ***** CV INPUTS *******
    addInput(Port::create<LRIOPortCV>(Vec(37.4, 284.4), Port::INPUT, module, DiodeVCF::FREQUCENCY_CV_INPUT));
    addInput(Port::create<LRIOPortCV>(Vec(175.3, 284.4), Port::INPUT, module, DiodeVCF::RESONANCE_CV_INPUT));
    addInput(Port::create<LRIOPortCV>(Vec(106.4, 284.4), Port::INPUT, module, DiodeVCF::SATURATE_CV_INPUT));
    // ***** CV INPUTS *******


    // ***** INPUTS **********
    addInput(Port::create<LRIOPortAudio>(Vec(37.4, 318.5), Port::INPUT, module, DiodeVCF::FILTER_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPortAudio>(Vec(175.3, 318.5), Port::OUTPUT, module, DiodeVCF::LP_OUTPUT));
    addOutput(Port::create<LRIOPortAudio>(Vec(106.4, 318.5), Port::OUTPUT, module, DiodeVCF::HP_OUTPUT));
    // ***** OUTPUTS *********
}


/*
struct DiodeVCFAged : MenuItem {
    DiodeVCF *diodeVCF;


    void onAction(EventAction &e) override {
        if (diodeVCF->aged) {
            diodeVCF->aged = false;
        } else {
            diodeVCF->aged = true;
        }

        diodeVCF->updateComponents();
    }


    void step() override {
        rightText = CHECKMARK(diodeVCF->aged);
    }
};
*/

struct DiodeVCFHiDef : MenuItem {
    DiodeVCF *diodeVCF;


    void onAction(EventAction &e) override {
        if (diodeVCF->hidef) {
            diodeVCF->hidef = false;
        } else {
            diodeVCF->hidef = true;
        }

        diodeVCF->updateComponents();
    }


    void step() override {
        rightText = CHECKMARK(diodeVCF->hidef);
    }
};


void DiodeVCFWidget::appendContextMenu(Menu *menu) {
    menu->addChild(MenuEntry::create());

    DiodeVCF *diodeVCF = dynamic_cast<DiodeVCF *>(module);
    assert(diodeVCF);


    DiodeVCFHiDef *mergeItemHiDef = MenuItem::create<DiodeVCFHiDef>("Use 4x oversampling");
    mergeItemHiDef->diodeVCF = diodeVCF;
    menu->addChild(mergeItemHiDef);
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, DiodeVCF) {
   Model *modelDiodeVCF = Model::create<DiodeVCF, DiodeVCFWidget>("Lindenberg Research", "DIODE VCF", "Laika Diode-Ladder Filter", FILTER_TAG);
   return modelDiodeVCF;
}
