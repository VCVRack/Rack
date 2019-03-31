/*                                                                     *\
**       __   ___  ______                                              **
**      / /  / _ \/_  __/                                              **
**     / /__/ , _/ / /    Lindenberg                                   **
**    /____/_/|_| /_/  Research Tec.                                   **
**                                                                     **
**                                                                     **
**	  https://github.com/lindenbergresearch/LRTRack	                   **
**    heapdump@icloud.com                                              **
**		                                                               **
**    Sound Modules for VCV Rack                                       **
**    Copyright 2017/2018 by Patrick Lindenberg / LRT                  **
**                                                                     **
**    For Redistribution and use in source and binary forms,           **
**    with or without modification please see LICENSE.                 **
**                                                                     **
\*                                                                     */

#include "../LindenbergResearch.hpp"
#include "../LRModel.hpp"
#include "../dsp/Korg35Filter.hpp"

namespace rack_plugin_LindenbergResearch {

using namespace rack;
using namespace lrt;

using dsp::Korg35Filter;

struct Korg35 : LRModule {
    enum ParamIds {
        FREQ_PARAM,
        PEAK_PARAM,
        SAT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        FILTER_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        LP_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    LRKnob *frqKnob, *peakKnob, *saturateKnob;
    Korg35Filter *filter = new Korg35Filter(engineGetSampleRate(), Korg35Filter::LPF);

    Korg35() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    void step() override {
        filter->fc = params[FREQ_PARAM].value;
        filter->peak = params[PEAK_PARAM].value;
        filter->sat = quadraticBipolar(params[SAT_PARAM].value);

        filter->in = inputs[FILTER_INPUT].value;
        filter->invalidate();
        filter->process();

        outputs[LP_OUTPUT].value = filter->out;
    }


    void onSampleRateChange() override {
        Module::onSampleRateChange();
        filter->setSamplerate(engineGetSampleRate());
    }
};


/**
 * @brief Blank Panel with Logo
 */
struct Korg35Widget : LRModuleWidget {
    Korg35Widget(Korg35 *module);
};


Korg35Widget::Korg35Widget(Korg35 *module) : LRModuleWidget(module) {
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/Korg35VCF.svg")));
    panel->addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/panels/Korg35VCF.svg")));
    panel->addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/panels/Korg35VCF.svg")));

    panel->init();
    addChild(panel);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewLight>(Vec(15, 1)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewLight>(Vec(15, 366)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    module->frqKnob = LRKnob::create<LRBigKnob>(Vec(32.5, 74.4), module, Korg35::FREQ_PARAM, 0.f, 1.f, 1.f);
    module->peakKnob = LRKnob::create<LRBigKnob>(Vec(32.5, 144.4), module, Korg35::PEAK_PARAM, 0.001f, 2.0, 0.001f);
    module->saturateKnob = LRKnob::create<LRMiddleKnob>(Vec(40, 244.4), module, Korg35::SAT_PARAM, 1.f, 2.5, 1.0f);

    module->frqKnob->setIndicatorColors(nvgRGBAf(0.9f, 0.9f, 0.9f, 1.0f));
    module->peakKnob->setIndicatorColors(nvgRGBAf(0.9f, 0.9f, 0.9f, 1.0f));
    module->saturateKnob->setIndicatorColors(nvgRGBAf(0.9f, 0.9f, 0.9f, 1.0f));

    addParam(module->frqKnob);
    addParam(module->peakKnob);
    addParam(module->saturateKnob);/*

      addParam(ParamWidget::create<LRSmallKnob>(Vec(39.9, 251.4), module, DiodeVCF::FREQUENCY_CV_PARAM, -1.f, 1.0f, 0.f));
      addParam(ParamWidget::create<LRSmallKnob>(Vec(177, 251.4), module, DiodeVCF::RESONANCE_CV_PARAM, -1.f, 1.0f, 0.f));
      addParam(ParamWidget::create<LRSmallKnob>(Vec(108.5, 251.4), module, DiodeVCF::SATURATE_CV_PARAM, -1.f, 1.0f, 0.f));*/
    // ***** MAIN KNOBS ******

    // ***** CV INPUTS *******
    /*  addInput(Port::create<LRIOPortCV>(Vec(37.4, 284.4), Port::INPUT, module, DiodeVCF::FREQUCENCY_CV_INPUT));
      addInput(Port::create<LRIOPortCV>(Vec(175.3, 284.4), Port::INPUT, module, DiodeVCF::RESONANCE_CV_INPUT));
      addInput(Port::create<LRIOPortCV>(Vec(106.4, 284.4), Port::INPUT, module, DiodeVCF::SATURATE_CV_INPUT));*/
    // ***** CV INPUTS *******


    // ***** INPUTS **********
    addInput(Port::create<LRIOPortAudio>(Vec(37.4, 318.5), Port::INPUT, module, Korg35::FILTER_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPortAudio>(Vec(175.3, 318.5), Port::OUTPUT, module, Korg35::LP_OUTPUT));
    // ***** OUTPUTS *********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, Korg35) {
   Model *modelKorg35 = Model::create<Korg35, Korg35Widget>("Lindenberg Research", "KORG35 VCF", "Mrs. Sally Korg35 Type Filter", FILTER_TAG);
   return modelKorg35;
}
