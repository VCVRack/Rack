#include "../dsp/Oscillator.hpp"
#include "../LindenbergResearch.hpp"
#include "../LRModel.hpp"

namespace rack_plugin_LindenbergResearch {

using namespace lrt;

using dsp::DSPBLOscillator;


struct VCO : LRModule {
    enum ParamIds {
        FREQUENCY_PARAM,
        OCTAVE_PARAM,
        FM_CV_PARAM,
        PW_CV_PARAM,
        SAW_PARAM,
        PULSE_PARAM,
        SINE_PARAM,
        TRI_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        VOCT1_INPUT,
        FM_CV_INPUT,
        PW_CV_INPUT,
        VOCT2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        SAW_OUTPUT,
        PULSE_OUTPUT,
        SINE_OUTPUT,
        TRI_OUTPUT,
        NOISE_OUTPUT,
        MIX_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        LFO_LIGHT,
        NUM_LIGHTS
    };

    dsp::DSPBLOscillator *osc = new dsp::DSPBLOscillator(engineGetSampleRate());
    LRLCDWidget *lcd = new LRLCDWidget(10, "%00004.3f Hz", LRLCDWidget::NUMERIC, LCD_FONTSIZE);
    LRBigKnob *frqKnob = NULL;


    VCO() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        frqKnob = LRKnob::create<LRBigKnob>(Vec(126.0, 64.7), this, VCO::FREQUENCY_PARAM, -1.f, 1.f, 0.f);
    }


    /*
    json_t *toJson() override {
        json_t *rootJ = LRModule::toJson();
        json_object_set_new(rootJ, "AGED", json_boolean(AGED));
        return rootJ;
    }


    void fromJson(json_t *rootJ) override {
        LRModule::fromJson(rootJ);

        json_t *agedJ = json_object_get(rootJ, "AGED");
        if (agedJ)
            AGED = json_boolean_value(agedJ);

        updateComponents();
    }*/


    void onRandomize() override;

    void step() override;
    void onSampleRateChange() override;
};


void VCO::step() {
    Module::step();

    float fm = clamp(inputs[FM_CV_INPUT].value, -CV_BOUNDS, CV_BOUNDS) * 0.4f * quadraticBipolar(params[FM_CV_PARAM].value);
    float tune = params[FREQUENCY_PARAM].value;
    float pw;

    if (inputs[PW_CV_INPUT].active) {
        pw = clamp(inputs[PW_CV_INPUT].value, -CV_BOUNDS, CV_BOUNDS) * 0.6f * quadraticBipolar(params[PW_CV_PARAM].value / 2.f) + 1;
        pw = clamp(pw, 0.01, 1.99);
    } else {
        pw = params[PW_CV_PARAM].value * 0.99f + 1;
    }

    if (frqKnob != NULL) {
        frqKnob->setIndicatorActive(inputs[FM_CV_INPUT].active);
        frqKnob->setIndicatorValue((params[FREQUENCY_PARAM].value + 1) / 2 + (fm / 2));
    }

    osc->setInputs(inputs[VOCT1_INPUT].value, inputs[VOCT2_INPUT].value, fm, tune, params[OCTAVE_PARAM].value);
    osc->setPulseWidth(pw);

    osc->process();

    outputs[SAW_OUTPUT].value = osc->getSawWave();
    outputs[PULSE_OUTPUT].value = osc->getPulseWave();
    outputs[SINE_OUTPUT].value = osc->getSineWave();
    outputs[TRI_OUTPUT].value = osc->getTriWave();
    outputs[NOISE_OUTPUT].value = osc->getNoise();


    if (outputs[MIX_OUTPUT].active) {
        float mix = 0.f;

        mix += osc->getSawWave() * params[SAW_PARAM].value;
        mix += osc->getPulseWave() * params[PULSE_PARAM].value;
        mix += osc->getSineWave() * params[SINE_PARAM].value;
        mix += osc->getTriWave() * params[TRI_PARAM].value;

        outputs[MIX_OUTPUT].value = mix;
    }

    /* for LFO mode */
    if (osc->isLFO())
        lights[LFO_LIGHT].setBrightnessSmooth(osc->getSineWave() / 10.f + 0.3f);
    else lights[LFO_LIGHT].value = 0.f;

    lcd->active = osc->isLFO();
    lcd->value = osc->getFrequency();
}


void VCO::onSampleRateChange() {
    Module::onSampleRateChange();
    osc->updateSampleRate(engineGetSampleRate());
}


void VCO::onRandomize() {
    Module::randomize();
}


/**
 * @brief Woldemar VCO Widget
 */
struct VCOWidget : LRModuleWidget {
    VCOWidget(VCO *module);
};


VCOWidget::VCOWidget(VCO *module) : LRModuleWidget(module) {
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/VCO.svg")));
    panel->addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/panels/Woldemar.svg")));
    panel->addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/panels/WoldemarAged.svg")));

    panel->init();
    box.size = panel->box.size;

    addChild(panel);

    panel->visible = true;
    panel->dirty = true;

    /* panel->setInner(nvgRGBAf(0.3, 0.3, 0.f, 0.09f));
     panel->setOuter(nvgRGBAf(0.f, 0.f, 0.f, 0.7f));

     module->panelAged->setInner(nvgRGBAf(0.5, 0.5, 0.f, 0.1f));
     module->panelAged->setOuter(nvgRGBAf(0.f, 0.f, 0.f, 0.73f));*/


    // **** SETUP LCD ********
    module->lcd->box.pos = Vec(22, 222);
    module->lcd->format = "%00004.3f Hz";
    addChild(module->lcd);
    // **** SETUP LCD ********


    // ***** SCREWS **********
    addChild(Widget::create<ScrewLight>(Vec(15, 1)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewLight>(Vec(15, 366)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********


    // ***** MAIN KNOBS ******
    addParam(module->frqKnob);
    addParam(LRKnob::create<LRToggleKnob>(Vec(133, 170.5), module, VCO::OCTAVE_PARAM, -4.f, 3.f, 0.f));

    addParam(LRKnob::create<LRSmallKnob>(Vec(69.5, 122), module, VCO::FM_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(LRKnob::create<LRSmallKnob>(Vec(69.5, 175), module, VCO::PW_CV_PARAM, -1, 1, 0.f));


    addParam(LRKnob::create<LRSmallKnob>(Vec(22.8, 270.1), module, VCO::SAW_PARAM, -1.f, 1.f, 0.f));
    addParam(LRKnob::create<LRSmallKnob>(Vec(58.3, 270.1), module, VCO::PULSE_PARAM, -1.f, 1.f, 0.f));
    addParam(LRKnob::create<LRSmallKnob>(Vec(93.1, 270.1), module, VCO::SINE_PARAM, -1.f, 1.f, 0.f));
    addParam(LRKnob::create<LRSmallKnob>(Vec(128.1, 270.1), module, VCO::TRI_PARAM, -1.f, 1.f, 0.f));
    // ***** MAIN KNOBS ******


    // ***** INPUTS **********
    addInput(Port::create<LRIOPortCV>(Vec(20.8, 67.9), Port::INPUT, module, VCO::VOCT1_INPUT));
    addInput(Port::create<LRIOPortCV>(Vec(68.0, 67.9), Port::INPUT, module, VCO::VOCT2_INPUT));
    addInput(Port::create<LRIOPortCV>(Vec(20.8, 121.5), Port::INPUT, module, VCO::FM_CV_INPUT));
    addInput(Port::create<LRIOPortCV>(Vec(20.8, 174.8), Port::INPUT, module, VCO::PW_CV_INPUT));
    // ***** INPUTS **********


    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPortAudio>(Vec(21, 305.8), Port::OUTPUT, module, VCO::SAW_OUTPUT));
    addOutput(Port::create<LRIOPortAudio>(Vec(56.8, 305.8), Port::OUTPUT, module, VCO::PULSE_OUTPUT));
    addOutput(Port::create<LRIOPortAudio>(Vec(91.6, 305.8), Port::OUTPUT, module, VCO::SINE_OUTPUT));
    addOutput(Port::create<LRIOPortAudio>(Vec(126.6, 305.8), Port::OUTPUT, module, VCO::TRI_OUTPUT));
    addOutput(Port::create<LRIOPortAudio>(Vec(162.0, 305.8), Port::OUTPUT, module, VCO::NOISE_OUTPUT));
    addOutput(Port::create<LRIOPortAudio>(Vec(162.0, 269.1), Port::OUTPUT, module, VCO::MIX_OUTPUT));
    // ***** OUTPUTS *********


    // ***** LIGHTS **********
    addChild(ModuleLightWidget::create<LRLight>(Vec(181.8, 210), module, VCO::LFO_LIGHT));
    // ***** LIGHTS **********
}


/*
struct VCOAged : MenuItem {
    VCO *vco;


    void onAction(EventAction &e) override {
        if (vco->AGED) {
            vco->AGED = false;
        } else {
            vco->AGED = true;
        }

        vco->updateComponents();
    }


    void step() override {
        rightText = CHECKMARK(vco->AGED);
    }
};


void VCOWidget::appendContextMenu(Menu *menu) {
    menu->addChild(MenuEntry::create());

    VCO *vco = dynamic_cast<VCO *>(module);
    assert(vco);

    VCOAged *mergeItemAged = MenuItem::create<VCOAged>("Use AGED look");
    mergeItemAged->vco = vco;
    menu->addChild(mergeItemAged);
}*/


} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, VCO) {
   Model *modelVCO = Model::create<VCO, VCOWidget>("Lindenberg Research", "VCO", "Woldemar VCO", OSCILLATOR_TAG);
   return modelVCO;
}
