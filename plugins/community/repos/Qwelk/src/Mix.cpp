#include "dsp/digital.hpp"
#include "qwelk.hpp"
#include "qwelk_common.h"


namespace rack_plugin_Qwelk {

struct ModuleMix : Module {
    enum ParamIds {
        PARAM_GAIN_M,
        PARAM_GAIN_S,
        PARAM_GAIN_MS,
        PARAM_GAIN_L,
        PARAM_GAIN_R,
        PARAM_GAIN_LR,
        NUM_PARAMS
    };
    enum InputIds {
        IN_L,
        IN_R,
        IN_M,
        IN_S,
        IN_GAIN_M,
        IN_GAIN_S,
        IN_GAIN_L,
        IN_GAIN_R,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT_M,
        OUT_S,
        OUT_L,
        OUT_R,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleMix() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};
static inline float _max(float a, float b) {return a < b ? b : a;}
void ModuleMix::step() {
    if (inputs[IN_L].active && inputs[IN_R].active) {
        float iam = _max(inputs[IN_GAIN_M].value, 0.f) / 10.0;
        float ias = _max(inputs[IN_GAIN_S].value, 0.f) / 10.0;
        float ams = params[PARAM_GAIN_MS].value;
        float am = inputs[IN_GAIN_M].active ? params[PARAM_GAIN_M].value * iam : params[PARAM_GAIN_M].value;
        float as = inputs[IN_GAIN_S].active ? params[PARAM_GAIN_S].value * ias : params[PARAM_GAIN_S].value;
        float l = inputs[IN_L].value;
        float r = inputs[IN_R].value;
        float m = l + r;
        float s = l - r;
        outputs[OUT_M].value = m * ams * am;
        outputs[OUT_S].value = s * ams * as;
    }
    if (inputs[IN_M].active && inputs[IN_S].active) {
        float ial = _max(inputs[IN_GAIN_L].value, 0.f) / 10.0;
        float iar = _max(inputs[IN_GAIN_R].value, 0.f) / 10.0;
        float alr = params[PARAM_GAIN_LR].value;
        float al = inputs[IN_GAIN_L].active ? params[PARAM_GAIN_L].value * ial : params[PARAM_GAIN_L].value;
        float ar = inputs[IN_GAIN_R].active ? params[PARAM_GAIN_R].value * iar : params[PARAM_GAIN_R].value;
        float m = inputs[IN_M].value;
        float s = inputs[IN_S].value;
        float l = (m + s) * 0.5;
        float r = (m - s) * 0.5;
        outputs[OUT_L].value = l * alr * al;
        outputs[OUT_R].value = r * alr * ar;
    }
}

struct WidgetMix : ModuleWidget {
    WidgetMix(ModuleMix *module);
};

WidgetMix::WidgetMix(ModuleMix *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/Mix.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(5, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(5, 365)));

    float x = box.size.x / 2.0 - 27;
    
    addInput(Port::create<PJ301MPort>(Vec(x     ,   25), Port::INPUT, module, ModuleMix::IN_L));
    addInput(Port::create<PJ301MPort>(Vec(x + 30,   25), Port::INPUT, module, ModuleMix::IN_R));
    
    addParam(ParamWidget::create<SmallKnob>(Vec(x + 28,  55), module, ModuleMix::PARAM_GAIN_MS, 0.0, 1.0, 1.0));

    addParam(ParamWidget::create<TinyKnob>(Vec(x    , 90), module, ModuleMix::PARAM_GAIN_M, 0.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(x + 30  , 88), Port::INPUT, module, ModuleMix::IN_GAIN_M));
    addOutput(Port::create<PJ301MPort>(Vec(x + 30, 113), Port::OUTPUT, module, ModuleMix::OUT_M));

    addParam(ParamWidget::create<TinyKnob>(Vec(x    , 147), module, ModuleMix::PARAM_GAIN_S, 0.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(x + 30  , 145), Port::INPUT, module, ModuleMix::IN_GAIN_S));
    addOutput(Port::create<PJ301MPort>(Vec(x + 30, 169), Port::OUTPUT, module, ModuleMix::OUT_S));

    addInput(Port::create<PJ301MPort>(Vec(x     , 210), Port::INPUT, module, ModuleMix::IN_M));
    addInput(Port::create<PJ301MPort>(Vec(x + 30, 210), Port::INPUT, module, ModuleMix::IN_S));

    addParam(ParamWidget::create<SmallKnob>(Vec(x + 28,  240), module, ModuleMix::PARAM_GAIN_LR, 0.0, 1.0, 1.0));

    addParam(ParamWidget::create<TinyKnob>(Vec(x    , 275), module, ModuleMix::PARAM_GAIN_L, 0.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(x + 30  , 273), Port::INPUT, module, ModuleMix::IN_GAIN_L));
    addOutput(Port::create<PJ301MPort>(Vec(x + 30, 298), Port::OUTPUT, module, ModuleMix::OUT_L));

    addParam(ParamWidget::create<TinyKnob>(Vec(x    , 332), module, ModuleMix::PARAM_GAIN_R, 0.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(x + 30  , 330), Port::INPUT, module, ModuleMix::IN_GAIN_R));
    addOutput(Port::create<PJ301MPort>(Vec(x + 30, 355), Port::OUTPUT, module, ModuleMix::OUT_R));
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Mix) {
   Model *modelMix = Model::create<ModuleMix, WidgetMix>(
      TOSTRING(SLUG), "Mix", "Mix", UTILITY_TAG, MIXER_TAG, AMPLIFIER_TAG);
   return modelMix;
}
