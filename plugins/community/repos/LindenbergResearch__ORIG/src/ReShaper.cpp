#include "LindenbergResearch.hpp"

namespace rack_plugin_LindenbergResearch {

struct ReShaper : Module {
    enum ParamIds {
        RESHAPER_AMOUNT,
        RESHAPER_CV_AMOUNT,
        NUM_PARAMS
    };

    enum InputIds {
        RESHAPER_INPUT,
        RESHAPER_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        RESHAPER_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };


    ReShaper() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}


    void step() override;
};


void ReShaper::step() {
    // normalize signal input to [-1.0...+1.0]
    float x = clamp(inputs[RESHAPER_INPUT].value * 0.1f, -1.f, 1.f);
    float cv = inputs[RESHAPER_CV_INPUT].value * params[RESHAPER_CV_AMOUNT].value;
    float a = clamp(params[RESHAPER_AMOUNT].value + cv, 1.f, 50.f);

    // do the acid!
    float out = x * (fabs(x) + a) / (x * x + (a - 1) * fabs(x) + 1);

    outputs[RESHAPER_OUTPUT].value = out * 5.0f;
}


/**
 * @brief Reshaper Panel
 */
struct ReShaperWidget : LRModuleWidget {
    ReShaperWidget(ReShaper *module);
};


ReShaperWidget::ReShaperWidget(ReShaper *module) : LRModuleWidget(module) {
    // setPanel(SVG::load(assetPlugin(plugin, "res/ReShaper.svg")));

    panel = new LRPanel();
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/ReShaper.svg")));
    addChild(panel);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewDarkA>(Vec(15, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(15, 366)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********


    // ***** MAIN KNOBS ******
    addParam(ParamWidget::create<LRBigKnob>(Vec(32, 216), module, ReShaper::RESHAPER_AMOUNT, 1.f, 50.f, 1.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(48, 126), module, ReShaper::RESHAPER_CV_AMOUNT, 0.f, 5.f, 0.f));
    // ***** MAIN KNOBS ******


    // ***** INPUTS **********
    addInput(Port::create<IOPort>(Vec(21, 60), Port::INPUT, module, ReShaper::RESHAPER_INPUT));
    addInput(Port::create<IOPort>(Vec(71, 60), Port::INPUT, module, ReShaper::RESHAPER_CV_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<IOPort>(Vec(46, 320), Port::OUTPUT, module, ReShaper::RESHAPER_OUTPUT));
    // ***** OUTPUTS *********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, ReShaper) {
   Model *modelReShaper = Model::create<ReShaper, ReShaperWidget>("Lindenberg Research", "ReShaper", "ReShaper Wavefolder", WAVESHAPER_TAG);
   return modelReShaper;
}
