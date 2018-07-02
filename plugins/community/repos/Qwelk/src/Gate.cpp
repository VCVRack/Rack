#include "qwelk.hpp"
#include "qwelk_common.h"

#define CHANNELS 2


namespace rack_plugin_Qwelk {

struct ModuleGate : Module {
    enum ParamIds {
        PARAM_GATEMODE,
        PARAM_THRESHOLD = PARAM_GATEMODE + CHANNELS,
        PARAM_OUTGAIN = PARAM_THRESHOLD + CHANNELS,
        NUM_PARAMS = PARAM_OUTGAIN + CHANNELS
    };
    enum InputIds {
        IN_SIG,
        NUM_INPUTS = IN_SIG + CHANNELS
    };
    enum OutputIds {
        OUT_GATE,
        NUM_OUTPUTS = OUT_GATE + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleGate() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleGate::step() {
    for (int i = 0; i < CHANNELS; ++i) {
        bool gatemode = params[PARAM_GATEMODE + i].value > 0.0;
        float in = inputs[IN_SIG + i].value;
        float threshold = params[PARAM_THRESHOLD + i].value;
        float out_gain = params[PARAM_OUTGAIN + i].value;
        float out = ((threshold >= 0) ? (in > threshold) : (in < threshold))
                    ? (gatemode ? 10.0 : in) : 0.0;
        outputs[OUT_GATE + i].value = out * out_gain;
    }
}

struct WidgetGate : ModuleWidget {
    WidgetGate(ModuleGate *module);
};

WidgetGate::WidgetGate(ModuleGate *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/Gate.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    for (int i = 0; i < CHANNELS; ++i) {
        float x = 2.5, top = 45 + i * 158;
        addParam(ParamWidget::create<CKSS>(         Vec(x + 5.7, top +   8), module, ModuleGate::PARAM_GATEMODE + i, 0.0, 1.0, 1.0));
        addParam(ParamWidget::create<TinyKnob>(Vec(x + 2.5, top +  40), module, ModuleGate::PARAM_THRESHOLD + i, -10.0, 10.0, 0));
        addInput(Port::create<PJ301MPort>(   Vec(x      , top +  63), Port::INPUT, module, ModuleGate::IN_SIG + i));
        addParam(ParamWidget::create<TinyKnob>(Vec(x + 2.5, top + 102), module, ModuleGate::PARAM_OUTGAIN + i, -1.0, 1.0, 1.0));
        addOutput(Port::create<PJ301MPort>( Vec(x      , top + 125), Port::OUTPUT, module, ModuleGate::OUT_GATE + i));
    }
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Gate) {
   Model *modelGate = Model::create<ModuleGate, WidgetGate>(
      TOSTRING(SLUG), "Gate", "Gate", UTILITY_TAG, ATTENUATOR_TAG);
}
