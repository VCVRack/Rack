#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 8


namespace rack_plugin_Qwelk {

struct ModuleWrap : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        IN_WRAP,
        IN_SIG,
        NUM_INPUTS = IN_SIG + CHANNELS
    };
    enum OutputIds {
        OUT_WRAPPED,
        NUM_OUTPUTS = OUT_WRAPPED + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    int _wrap = -10;
    
    ModuleWrap() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleWrap::step() {
    int wrap = (/*clampf(*/inputs[IN_WRAP].value/*, -5.0, 5.0)*/ / 5.0) * (CHANNELS - 1);

    for (int i = 0; i < CHANNELS; ++i) {
        int w = i;
        if (wrap > 0)
            w = (i + wrap) % CHANNELS;
        else if (wrap < 0)
            w = (i + CHANNELS - wrap) % CHANNELS;
        outputs[OUT_WRAPPED + i].value = inputs[IN_SIG + w].value;
    }
}

struct WidgetWrap : ModuleWidget {
    WidgetWrap(ModuleWrap *module);
};

WidgetWrap::WidgetWrap(ModuleWrap *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/Wrap.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 25, ytop = 60, ystep = 39;

    addInput(Port::create<PJ301MPort>(Vec(17.5, 30), Port::INPUT, module, ModuleWrap::IN_WRAP));
    
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(Port::create<PJ301MPort>(   Vec(x       , ytop + ystep * i), Port::INPUT, module, ModuleWrap::IN_SIG  + i));
        addOutput(Port::create<PJ301MPort>( Vec(x + 26  , ytop + ystep * i), Port::OUTPUT, module, ModuleWrap::OUT_WRAPPED + i));
    }
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Wrap) {
   Model *modelWrap = Model::create<ModuleWrap, WidgetWrap>(
      TOSTRING(SLUG), "Wrap", "Wrap", UTILITY_TAG);
   return modelWrap;
}
