#include "../dsp/DSPMath.hpp"
#include "../LindenbergResearch.hpp"
#include "../LRModel.hpp"

using namespace rack;
using namespace lrt;

namespace rack_plugin_LindenbergResearch {

struct ShapedVCA {

    float shapeExp(float x) {
        float y = powf(x, 7);

        if (x > 1) {
            y = tanh(y) * 5; // limit exp curve
        }

        return y;
    }


    float shapeLog(float x) {
        return tanh(6 * x);
    }


    float shapeLin(float x) {
        return x;
    }


    /**
     * @brief Returns the correct weighted gain value
     * @param gain Input gain 0 <= x < 1
     * @param shape Mixing value of the given shapes (LOG->LIN->EXP) -1 <= x < 1
     * @return Weighted gain coefficent
     */
    float getWeightedGain(float gain, float shape) {
        float y = 0;

        if (shape < 0 && shape >= -1) {
           y = dsp::fade2(shapeLog(gain), shapeLin(gain), shape + 1);
        } else if (shape <= 1 && shape >= 0) {
           y = dsp::fade2(shapeLin(gain), shapeExp(gain), shape);
        }

        return y;
    }


    float compute(float in, float gain, float shape) {
        return in * getWeightedGain(gain, shape);
    }

};


struct QuickMix : LRModule {
    enum ParamIds {
        LEVEL1_PARAM,
        LEVEL2_PARAM,
        LEVEL3_PARAM,
        LEVEL4_PARAM,
        LEVEL5_PARAM,
        SHAPE_PARAM,
        LEVELM_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        M1_INPUT,
        M2_INPUT,
        M3_INPUT,
        M4_INPUT,
        M5_INPUT,
        CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        MASTER_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        LEVEL1_LIGHT,
        LEVEL2_LIGHT,
        LEVEL3_LIGHT,
        LEVEL4_LIGHT,
        LEVEL5_LIGHT,
        NUM_LIGHTS
    };


    float lightVals[NUM_LIGHTS];
    ShapedVCA vca;


    QuickMix() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    void step() override;
};


void QuickMix::step() {
    float out = 0;

    /* lights */
    for (int i = 0; i < NUM_LIGHTS - 1; i++) {
        lightVals[i] = clamp(inputs[i].value * abs(quadraticBipolar(params[i].value)) / 6, 0.f, 1.f);

        if (inputs[i].active) {
            lights[i].setBrightnessSmooth(lightVals[i]);
        } else {
            lights[i].value = 0;
        }
    }

    /* mixup all signals */
    for (int i = 0; i < NUM_INPUTS - 1; i++) {
        out += inputs[i].value * quadraticBipolar(params[i].value);
    }

    /* VCA mode active */
    if (inputs[CV_INPUT].active) {
        float cv = inputs[CV_INPUT].value / 5;

        out *= vca.getWeightedGain(cv, params[SHAPE_PARAM].value);
    }

    out *= quadraticBipolar(params[LEVELM_PARAM].value) * 2;

    outputs[MASTER_OUTPUT].value = out;
}


/**
 * @brief Blank Panel Small
 */
struct QuickMixWidget : LRModuleWidget {
    QuickMixWidget(QuickMix *module);
};


QuickMixWidget::QuickMixWidget(QuickMix *module) : LRModuleWidget(module) {
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/QuickMix.svg")));
    panel->addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/panels/QuickMixLight.svg")));
    panel->addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/panels/QuickMixAged.svg")));

    panel->init();
    addChild(panel);
    box.size = panel->box.size;

    // ***** SCREWS **********
    // addChild(Widget::create<ScrewLight>(Vec(15, 1)));
    // addChild(Widget::create<ScrewLight>(Vec(box.size.x - 20, 1)));
    //  addChild(Widget::create<ScrewLight>(Vec(15, 366)));
    //  addChild(Widget::create<ScrewLight>(Vec(box.size.x - 20, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 53.8), module, QuickMix::LEVEL1_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 88.8), module, QuickMix::LEVEL2_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 123.8), module, QuickMix::LEVEL3_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 158.8), module, QuickMix::LEVEL4_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 193.8), module, QuickMix::LEVEL5_PARAM, -1.f, 1.f, 0.f));

    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 242.0), module, QuickMix::SHAPE_PARAM, -1.f, 1.f, 0.f));

    addParam(ParamWidget::create<LRSmallKnob>(Vec(18.8, 305.8), module, QuickMix::LEVELM_PARAM, 0.f, 1.f, 0.5f));
    // ***** MAIN KNOBS ******

    // ***** INPUTS **********
    addInput(Port::create<LRIOPortAudio>(Vec(16.5, 52.6), Port::INPUT, module, QuickMix::M1_INPUT));
    addInput(Port::create<LRIOPortAudio>(Vec(16.5, 87.6), Port::INPUT, module, QuickMix::M2_INPUT));
    addInput(Port::create<LRIOPortAudio>(Vec(16.5, 122.6), Port::INPUT, module, QuickMix::M3_INPUT));
    addInput(Port::create<LRIOPortAudio>(Vec(16.5, 157.6), Port::INPUT, module, QuickMix::M4_INPUT));
    addInput(Port::create<LRIOPortAudio>(Vec(16.5, 192.6), Port::INPUT, module, QuickMix::M5_INPUT));

    addInput(Port::create<LRIOPortCV>(Vec(16.5, 240.8f), Port::INPUT, module, QuickMix::CV_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPortAudio>(Vec(60.9, 304.8), Port::OUTPUT, module, QuickMix::MASTER_OUTPUT));
    // ***** OUTPUTS *********


    // ***** LIGHTS **********
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.f, 61.3), module, QuickMix::LEVEL1_LIGHT));
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.f, 96.3), module, QuickMix::LEVEL2_LIGHT));
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.f, 131.3), module, QuickMix::LEVEL3_LIGHT));
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.f, 166.3), module, QuickMix::LEVEL4_LIGHT));
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.f, 201.3), module, QuickMix::LEVEL5_LIGHT));


    // addChild(ModuleLightWidget::create<LRLight>(Vec(47.5, 304.6), module, QuickMix::LEVEL6_LIGHT));

    // ***** LIGHTS **********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, QuickMix) {
   Model *modelQuickMix = Model::create<QuickMix, QuickMixWidget>("Lindenberg Research", "QuickMixer", "VC Mixer Amp", MIXER_TAG);
   return modelQuickMix;
}
