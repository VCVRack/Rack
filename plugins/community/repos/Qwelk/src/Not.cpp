#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 8

namespace rack_plugin_Qwelk {

struct ModuleNot : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_SIG,
        NUM_INPUTS = INPUT_SIG + CHANNELS
    };
    enum OutputIds {
        OUTPUT_NOT,
        NUM_OUTPUTS = OUTPUT_NOT + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleNot() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleNot::step() {
    for (int i = 0; i < CHANNELS; ++i) {
        outputs[OUTPUT_NOT + i].value = inputs[INPUT_SIG + i].value != 0.0 ? 0.0 : 10.0;
    }
}

struct WidgetNot : ModuleWidget {
    WidgetNot(ModuleNot *module);
};

WidgetNot::WidgetNot(ModuleNot *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/Not.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 25, ytop = 45, ystep = 39;
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(Port::create<PJ301MPort>(   Vec(x       , ytop + ystep * i), Port::INPUT, module, ModuleNot::INPUT_SIG  + i));
        addOutput(Port::create<PJ301MPort>( Vec(x + 26  , ytop + ystep * i), Port::OUTPUT, module, ModuleNot::OUTPUT_NOT + i));
    }
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Not) {
   Model *modelNot = Model::create<ModuleNot, WidgetNot>(
      TOSTRING(SLUG), "NOT", "NOT", UTILITY_TAG, LOGIC_TAG);
   return modelNot;
}
