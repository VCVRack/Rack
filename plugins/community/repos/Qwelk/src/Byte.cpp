#include "dsp/digital.hpp"
#include "util/math.hpp"
#include "qwelk.hpp"


#define CHANNELS 8

namespace rack_plugin_Qwelk {

struct ModuleByte : Module {
    enum ParamIds {
        PARAM_SCAN,
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_SCAN,
        INPUT_GATE,
        NUM_INPUTS = INPUT_GATE + CHANNELS
    };
    enum OutputIds {
        OUTPUT_COUNT,
        OUTPUT_NUMBER,
        NUM_OUTPUTS
    };
    enum LightIds {
        LIGHT_POS_SCAN,
        LIGHT_NEG_SCAN,
        NUM_LIGHTS,
    };

    int             scan = 1;
    int             scan_sign = 0;
    SchmittTrigger  trig_scan_input;
    SchmittTrigger  trig_scan_manual;

    const float output_volt_uni = 10.0;

    ModuleByte() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;
};

void ModuleByte::step()
{
    // determine scan direction
    int scan_input_sign = (int)sgn(inputs[INPUT_SCAN].normalize(scan));
    if (scan_input_sign != scan_sign) 
        scan = scan_sign = scan_input_sign;
    // manual tinkering with step?
    if (trig_scan_manual.process(params[PARAM_SCAN].value))
        scan *= -1;
    
    int active_count = 0, count = 0, number = 0;
    for (int i = 0; i < CHANNELS; ++i) {
        int bit = scan >= 0 ? i : (CHANNELS - 1 - i);
        int above0 = inputs[INPUT_GATE + i].value > 0 ? 1 : 0;
        active_count += inputs[INPUT_GATE + i].active ? 1 : 0;
        count += above0;
        if (above0)
            number |= 1 << bit;
    }

    outputs[OUTPUT_COUNT].value  = active_count ? ((float)count  / active_count) * output_volt_uni : 0.0f;
    outputs[OUTPUT_NUMBER].value = ((float)number / (float)(1 << (CHANNELS - 1))) * output_volt_uni;

    // indicate step direction
    lights[LIGHT_POS_SCAN].setBrightness(scan < 0 ? 0.0 : 0.9);
    lights[LIGHT_NEG_SCAN].setBrightness(scan < 0 ? 0.9 : 0.0);
}



template <typename _BASE>
struct MuteLight : _BASE {
    MuteLight()
    {
        this->box.size = mm2px(Vec(6, 6));
    }
};

struct WidgetByte : ModuleWidget {
    WidgetByte(ModuleByte *module);
};


WidgetByte::WidgetByte(ModuleByte *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/Byte.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
 
    const float ypad = 27.5;
    float x = box.size.x / 2.0 - 25.0 / 2.0;
    float ytop = 90.5;

    addParam(ParamWidget::create<LEDBezel>(Vec(x + 1.5, ytop - ypad * 2 - 3.5), module, ModuleByte::PARAM_SCAN, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MuteLight<GreenRedLight>>(Vec(x + 3.75, ytop - ypad * 2 + - 3.5 + 2), module, ModuleByte::LIGHT_POS_SCAN));
    addInput(Port::create<PJ301MPort>(Vec(x, ytop - ypad + 1), Port::INPUT, module, ModuleByte::INPUT_SCAN));
    //ytop += ypad * 0.25;
    
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(Port::create<PJ301MPort>(Vec(x, ytop + ypad * i), Port::INPUT, module, ModuleByte::INPUT_GATE + i));
    }
    //ytop += ypad * 0.25;
    
    const float output_y = ytop + ypad * CHANNELS;

    addOutput(Port::create<PJ301MPort>(Vec(x, output_y        ), Port::OUTPUT, module, ModuleByte::OUTPUT_NUMBER));
    addOutput(Port::create<PJ301MPort>(Vec(x, output_y + ypad ), Port::OUTPUT, module, ModuleByte::OUTPUT_COUNT));
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Byte) {
   Model *modelByte = Model::create<ModuleByte, WidgetByte>(
      TOSTRING(SLUG), "Byte", "Byte", UTILITY_TAG, LOGIC_TAG);
   return modelByte;
}
