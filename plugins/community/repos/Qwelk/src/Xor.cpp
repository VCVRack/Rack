#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 3

namespace rack_plugin_Qwelk {

struct ModuleXor : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_A,
        INPUT_B     = INPUT_A + CHANNELS,
        NUM_INPUTS  = INPUT_B + CHANNELS
    };
    enum OutputIds {
        OUTPUT_XOR,
        NUM_OUTPUTS = OUTPUT_XOR + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleXor() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleXor::step() {
    for (int i = 0; i < CHANNELS; ++i)
        outputs[OUTPUT_XOR + i].value = inputs[INPUT_A + i].value == inputs[INPUT_B + i].value ? 0.0 : 10.0;
}

struct WidgetXor : ModuleWidget {
    WidgetXor(ModuleXor *module);
};

WidgetXor::WidgetXor(ModuleXor *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/Xor.svg")));


    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 12, ytop = 45, ystep = 37.5;
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(Port::create<PJ301MPort>(   Vec(x, ytop + ystep * i), Port::INPUT, module, ModuleXor::INPUT_A + i));
        addInput(Port::create<PJ301MPort>(   Vec(x, ytop + ystep*1 + ystep * i), Port::INPUT, module, ModuleXor::INPUT_B + i));
        addOutput(Port::create<PJ301MPort>( Vec(x, ytop + ystep*2 + ystep  * i), Port::OUTPUT, module, ModuleXor::OUTPUT_XOR + i));
        ytop += 3 * ystep - 42.5;
    }
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Xor) {
   Model *modelXor = Model::create<ModuleXor, WidgetXor>(
      TOSTRING(SLUG), "XOR", "XOR", UTILITY_TAG, LOGIC_TAG);
   return modelXor;
}
