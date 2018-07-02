#include "dsp/digital.hpp"
#include "qwelk.hpp"

namespace rack_plugin_Qwelk {

struct ModuleScaler : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_SUB_5,
        INPUT_MUL_2,
        INPUT_DIV_2,
        INPUT_ADD_5,
        NUM_INPUTS
    };
    enum OutputIds {
        OUTPUT_SUB_5,
        OUTPUT_MUL_2,
        OUTPUT_DIV_2,
        OUTPUT_ADD_5,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleScaler() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleScaler::step() {
    outputs[OUTPUT_SUB_5].value = inputs[INPUT_SUB_5].value - 5.0;
    outputs[OUTPUT_MUL_2].value = inputs[INPUT_MUL_2].normalize(outputs[OUTPUT_SUB_5].value) * 2.0;
    outputs[OUTPUT_DIV_2].value = inputs[INPUT_DIV_2].normalize(outputs[OUTPUT_MUL_2].value) * 0.5;
    outputs[OUTPUT_ADD_5].value = inputs[INPUT_ADD_5].normalize(outputs[OUTPUT_DIV_2].value) + 5.0;
}

struct WidgetScaler : ModuleWidget {
    WidgetScaler(ModuleScaler *module);
};

WidgetScaler::WidgetScaler(ModuleScaler *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/Scaler.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 12, y = 0, ytop = 30, ystep = 30, mstep = 16;
    addInput(Port::create<PJ301MPort>(   Vec(x, ytop + (y+=ystep)), Port::INPUT, module, ModuleScaler::INPUT_SUB_5));
    addOutput(Port::create<PJ301MPort>( Vec(x, ytop + (y+=ystep)), Port::OUTPUT, module, ModuleScaler::OUTPUT_SUB_5));
    ytop += mstep;
    addInput(Port::create<PJ301MPort>(   Vec(x, ytop + (y+=ystep)), Port::INPUT, module, ModuleScaler::INPUT_MUL_2));
    addOutput(Port::create<PJ301MPort>( Vec(x, ytop + (y+=ystep)), Port::OUTPUT, module, ModuleScaler::OUTPUT_MUL_2));
    ytop += mstep;
    addInput(Port::create<PJ301MPort>(   Vec(x, ytop + (y+=ystep)), Port::INPUT, module, ModuleScaler::INPUT_DIV_2));
    addOutput(Port::create<PJ301MPort>( Vec(x, ytop + (y+=ystep)), Port::OUTPUT, module, ModuleScaler::OUTPUT_DIV_2));
    ytop += mstep;
    addInput(Port::create<PJ301MPort>(   Vec(x, ytop + (y+=ystep)), Port::INPUT, module, ModuleScaler::INPUT_ADD_5));
    addOutput(Port::create<PJ301MPort>( Vec(x, ytop + (y+=ystep)), Port::OUTPUT, module, ModuleScaler::OUTPUT_ADD_5));
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Scaler) {
   Model *modelScaler = Model::create<ModuleScaler, WidgetScaler>(
      TOSTRING(SLUG), "Scaler", "Scaler", UTILITY_TAG);
   return modelScaler;
}
