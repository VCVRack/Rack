#include "../LindenbergResearch.hpp"
#include "../LRModel.hpp"

namespace rack_plugin_LindenbergResearch {
using namespace lrt;


struct ReShaper : LRModule {
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


    ReShaper() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


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
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/ReShaper.svg")));
    // panel->addSVGVariant(SVG::load(assetPlugin(plugin, "res/panels/ReShaper.svg")));
    // panel->addSVGVariant(SVG::load(assetPlugin(plugin, "res/panels/ReShaper.svg")));

    noVariants = true;
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
    addParam(ParamWidget::create<LRBigKnob>(Vec(32.7, 228), module, ReShaper::RESHAPER_AMOUNT, 1.f, 50.f, 1.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(48.9, 126), module, ReShaper::RESHAPER_CV_AMOUNT, 0.f, 5.f, 0.f));
    // ***** MAIN KNOBS ******


    // ***** INPUTS **********
    addInput(Port::create<LRIOPortAudio>(Vec(21.5, 52.3), Port::INPUT, module, ReShaper::RESHAPER_INPUT));
    addInput(Port::create<LRIOPortCV>(Vec(71.2, 52.3), Port::INPUT, module, ReShaper::RESHAPER_CV_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPortAudio>(Vec(46.2, 311.6), Port::OUTPUT, module, ReShaper::RESHAPER_OUTPUT));
    // ***** OUTPUTS *********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, ReShaper) {
   Model *modelReShaper = Model::create<ReShaper, ReShaperWidget>("Lindenberg Research", "ReShaper", "ReShaper Wavefolder", WAVESHAPER_TAG);
   return modelReShaper;
}
