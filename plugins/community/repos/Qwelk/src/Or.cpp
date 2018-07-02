#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 8

namespace rack_plugin_Qwelk {

struct ModuleOr : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_CHANNEL,
        NUM_INPUTS = INPUT_CHANNEL + CHANNELS
    };
    enum OutputIds {
        OUTPUT_OR,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleOr() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleOr::step() {
    int gate_on = 0;
    for (int i = 0; !gate_on && i < CHANNELS; ++i)
        gate_on = inputs[INPUT_CHANNEL + i].value;
    outputs[OUTPUT_OR].value = gate_on ? 10 : 0;
}

struct WidgetOr : ModuleWidget {
    WidgetOr(ModuleOr *module);
};

WidgetOr::WidgetOr(ModuleOr *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/Or.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 12, ytop = 45, ystep = 32.85;
    for (int i = 0; i < CHANNELS; ++i)
        addInput(Port::create<PJ301MPort>(Vec(x, ytop + ystep * i), Port::INPUT, module, ModuleOr::INPUT_CHANNEL + i));
    ytop += 9;
    addOutput(Port::create<PJ301MPort>( Vec(x, ytop + ystep * CHANNELS), Port::OUTPUT, module, ModuleOr::OUTPUT_OR));
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Or) {
   Model *modelOr = Model::create<ModuleOr, WidgetOr>(
      TOSTRING(SLUG), "OR", "OR", UTILITY_TAG, LOGIC_TAG);
   return modelOr;
}
