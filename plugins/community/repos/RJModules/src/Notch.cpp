#include "RJModules.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include "VAStateVariableFilter.h"

namespace rack_plugin_RJModules {

struct Notch: Module {
    enum ParamIds {
        FREQ_PARAM,
        VOL_PARAM,
        WIDTH_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        FREQ_CV_INPUT,
        VOL_CV_INPUT,
        WIDTH_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        NUM_OUTPUTS
    };

    VAStateVariableFilter *notchFilter = new VAStateVariableFilter();


    Notch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
    void step() override;
};

void Notch::step() {

    float dry = inputs[CH1_INPUT].value;
    float wet = 0.0;

    dry += 1.0e-6 * (2.0*randomUniform() - 1.0)*1000;

    notchFilter->setFilterType(5);

    notchFilter->setCutoffFreq(params[FREQ_PARAM].value * clamp(inputs[FREQ_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f));
    notchFilter->setShelfGain(params[VOL_PARAM].value * clamp(inputs[VOL_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f));
    notchFilter->setResonance(params[WIDTH_PARAM].value * clamp(inputs[WIDTH_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f));
    notchFilter->setSampleRate(engineGetSampleRate());

    wet = notchFilter->processAudioSample(dry, 1);
    outputs[CH1_OUTPUT].value = wet;
}


struct NotchWidget: ModuleWidget {
    NotchWidget(Notch *module);
};

NotchWidget::NotchWidget(Notch *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Notch.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 61), module, Notch::FREQ_PARAM, 30.0, 6000.0, 1000.0));
    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 143), module, Notch::VOL_PARAM,  0.0, 5.0, 2));
    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 228), module, Notch::WIDTH_PARAM, 0.0, 1.0, 0.5));

    addInput(Port::create<PJ301MPort>(Vec(22, 100), Port::INPUT, module, Notch::FREQ_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 180), Port::INPUT, module, Notch::VOL_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 260), Port::INPUT, module, Notch::WIDTH_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 310), Port::INPUT, module, Notch::CH1_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(110, 310), Port::OUTPUT, module, Notch::CH1_OUTPUT));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Notch) {
   Model *modelNotch = Model::create<Notch, NotchWidget>("RJModules", "Notch", "[FILT] Notch", FILTER_TAG);//UTILITY_TAG);
   return modelNotch;
}
