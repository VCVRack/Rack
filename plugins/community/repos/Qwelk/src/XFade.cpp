#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 2

namespace rack_plugin_Qwelk {

struct ModuleXFade : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_A,
        INPUT_B     = INPUT_A + CHANNELS,
        INPUT_X     = INPUT_B + CHANNELS,
        NUM_INPUTS  = INPUT_X + CHANNELS
    };
    enum OutputIds {
        OUTPUT_BLEND,
        NUM_OUTPUTS = OUTPUT_BLEND + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleXFade() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleXFade::step() {
    for (int i = 0; i < CHANNELS; ++i) {
        float blend = inputs[INPUT_X + i].value / 10.0;
        outputs[OUTPUT_BLEND + i].value = (1.0 - blend) * inputs[INPUT_A + i].value + inputs[INPUT_B + i].value * blend;
    }
}

struct WidgetXFade : ModuleWidget {
    WidgetXFade(ModuleXFade *module);
};

WidgetXFade::WidgetXFade(ModuleXFade *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/XFade.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 12, ytop = 45, ystep = 37.5;
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(Port::create<PJ301MPort>(   Vec(x, ytop + ystep * i), Port::INPUT, module, ModuleXFade::INPUT_A + i));
        addInput(Port::create<PJ301MPort>(   Vec(x, ytop + ystep*1 + ystep * i), Port::INPUT, module, ModuleXFade::INPUT_B + i));
        addInput(Port::create<PJ301MPort>(   Vec(x, ytop + ystep*2 + ystep * i), Port::INPUT, module, ModuleXFade::INPUT_X + i));
        addOutput(Port::create<PJ301MPort>( Vec(x, ytop + ystep*3 + ystep  * i), Port::OUTPUT, module, ModuleXFade::OUTPUT_BLEND + i));
        ytop += 4 * ystep - 17.5;
    }
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, XFade) {
   Model *modelXFade = Model::create<ModuleXFade, WidgetXFade>(
      TOSTRING(SLUG), "XFade", "XFade", UTILITY_TAG);
   return modelXFade;
}
