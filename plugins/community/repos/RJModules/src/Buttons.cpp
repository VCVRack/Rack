#include "RJModules.hpp"
#include "dsp/digital.hpp"
#include <iostream>
#include <cmath>

namespace rack_plugin_RJModules {

struct Buttons: Module {
    enum ParamIds {
        RESET_PARAM,
        RESET_PARAM2,
        RESET_PARAM3,
        RESET_PARAM4,
        RESET_PARAM5,
        RESET_PARAM6,
        RESET_PARAM7,
        RESET_PARAM8,
        RESET_PARAM9,
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        CH3_OUTPUT,
        CH4_OUTPUT,
        CH5_OUTPUT,
        CH6_OUTPUT,
        CH7_OUTPUT,
        CH8_OUTPUT,
        CH9_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        RESET_LIGHT,
        RESET_LIGHT2,
        RESET_LIGHT3,
        RESET_LIGHT4,
        RESET_LIGHT5,
        RESET_LIGHT6,
        RESET_LIGHT7,
        RESET_LIGHT8,
        RESET_LIGHT9,
        NUM_LIGHTS
    };
    float resetLight = 0.0;
    float resetLight2 = 0.0;
    float resetLight3 = 0.0;
    float resetLight4 = 0.0;
    float resetLight5 = 0.0;
    float resetLight6 = 0.0;
    float resetLight7 = 0.0;
    float resetLight8 = 0.0;
    float resetLight9 = 0.0;

    Buttons() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

struct MedLEDButton : SVGSwitch, MomentarySwitch {
        MedLEDButton() {
                addFrame(SVG::load(assetPlugin(plugin, "res/MedLEDButton.svg")));
        }
};

template <typename BASE>
struct MedLight : BASE {
        MedLight() {
                this->box.size = mm2px(Vec(10, 10));
        }
};

void Buttons::step() {

    const float lightLambda = 0.075;
    float output = 0.0;
    float output2 = 0.0;
    float output3 = 0.0;
    float output4 = 0.0;
    float output5 = 0.0;
    float output6 = 0.0;
    float output7 = 0.0;
    float output8 = 0.0;
    float output9 = 0.0;

    // Reset
    if (params[RESET_PARAM].value > 0) {
        resetLight = 1.0;
        output = 12.0;
    }
    if (params[RESET_PARAM2].value > 0) {
        resetLight2 = 1.0;
        output2 = 12.0;
    }
    if (params[RESET_PARAM3].value > 0) {
        resetLight3 = 1.0;
        output3 = 12.0;
    }
    if (params[RESET_PARAM4].value > 0) {
        resetLight4 = 1.0;
        output4 = 12.0;
    }
    if (params[RESET_PARAM5].value > 0) {
        resetLight5 = 1.0;
        output5 = 12.0;
    }
    if (params[RESET_PARAM6].value > 0) {
        resetLight6 = 1.0;
        output6 = 12.0;
    }
    if (params[RESET_PARAM7].value > 0) {
        resetLight7 = 1.0;
        output7 = 12.0;
    }
    if (params[RESET_PARAM8].value > 0) {
        resetLight8 = 1.0;
        output8 = 12.0;
    }
    if (params[RESET_PARAM9].value > 0) {
        resetLight9 = 1.0;
        output9 = 12.0;
    }

    resetLight -= resetLight / lightLambda / engineGetSampleRate();
    resetLight2 -= resetLight2 / lightLambda / engineGetSampleRate();
    resetLight3 -= resetLight3 / lightLambda / engineGetSampleRate();
    resetLight4 -= resetLight4 / lightLambda / engineGetSampleRate();
    resetLight5 -= resetLight5 / lightLambda / engineGetSampleRate();
    resetLight6 -= resetLight6 / lightLambda / engineGetSampleRate();
    resetLight7 -= resetLight7 / lightLambda / engineGetSampleRate();
    resetLight8 -= resetLight8 / lightLambda / engineGetSampleRate();
    resetLight9 -= resetLight9 / lightLambda / engineGetSampleRate();

    outputs[CH1_OUTPUT].value = output;
    outputs[CH2_OUTPUT].value = output2;
    outputs[CH3_OUTPUT].value = output3;
    outputs[CH4_OUTPUT].value = output4;
    outputs[CH5_OUTPUT].value = output5;
    outputs[CH6_OUTPUT].value = output6;
    outputs[CH7_OUTPUT].value = output7;
    outputs[CH8_OUTPUT].value = output8;
    outputs[CH9_OUTPUT].value = output9;

    lights[RESET_LIGHT].value = resetLight;
    lights[RESET_LIGHT2].value = resetLight2;
    lights[RESET_LIGHT3].value = resetLight3;
    lights[RESET_LIGHT4].value = resetLight4;
    lights[RESET_LIGHT5].value = resetLight5;
    lights[RESET_LIGHT6].value = resetLight6;
    lights[RESET_LIGHT7].value = resetLight7;
    lights[RESET_LIGHT8].value = resetLight8;
    lights[RESET_LIGHT9].value = resetLight9;

}


struct ButtonsWidget: ModuleWidget {
    ButtonsWidget(Buttons *module);
};

ButtonsWidget::ButtonsWidget(Buttons *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Buttons.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addOutput(Port::create<PJ301MPort>(Vec(24, 223), Port::OUTPUT, module, Buttons::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(65, 223), Port::OUTPUT, module, Buttons::CH2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(105, 223), Port::OUTPUT, module, Buttons::CH3_OUTPUT));

    addOutput(Port::create<PJ301MPort>(Vec(24, 274), Port::OUTPUT, module, Buttons::CH4_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(65, 274), Port::OUTPUT, module, Buttons::CH5_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(106, 274), Port::OUTPUT, module, Buttons::CH6_OUTPUT));

    addOutput(Port::create<PJ301MPort>(Vec(24, 324), Port::OUTPUT, module, Buttons::CH7_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(65, 324), Port::OUTPUT, module, Buttons::CH8_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(106, 324), Port::OUTPUT, module, Buttons::CH9_OUTPUT));

    addParam(ParamWidget::create<MedLEDButton>(Vec(15, 60), module, Buttons::RESET_PARAM, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MedLight<GreenLight>>(Vec(20, 65), module, Buttons::RESET_LIGHT));

    addParam(ParamWidget::create<MedLEDButton>(Vec(55, 60), module, Buttons::RESET_PARAM2, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MedLight<GreenLight>>(Vec(60, 65), module, Buttons::RESET_LIGHT2));

    addParam(ParamWidget::create<MedLEDButton>(Vec(95, 60), module, Buttons::RESET_PARAM3, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MedLight<GreenLight>>(Vec(100, 65), module, Buttons::RESET_LIGHT3));

    addParam(ParamWidget::create<MedLEDButton>(Vec(15, 100), module, Buttons::RESET_PARAM4, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MedLight<GreenLight>>(Vec(20, 105), module, Buttons::RESET_LIGHT4));

    addParam(ParamWidget::create<MedLEDButton>(Vec(55, 100), module, Buttons::RESET_PARAM5, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MedLight<GreenLight>>(Vec(60, 105), module, Buttons::RESET_LIGHT5));

    addParam(ParamWidget::create<MedLEDButton>(Vec(95, 100), module, Buttons::RESET_PARAM6, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MedLight<GreenLight>>(Vec(100, 105), module, Buttons::RESET_LIGHT6));

    addParam(ParamWidget::create<MedLEDButton>(Vec(15, 140), module, Buttons::RESET_PARAM7, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MedLight<GreenLight>>(Vec(20, 145), module, Buttons::RESET_LIGHT7));

    addParam(ParamWidget::create<MedLEDButton>(Vec(55, 140), module, Buttons::RESET_PARAM8, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MedLight<GreenLight>>(Vec(60, 145), module, Buttons::RESET_LIGHT8));

    addParam(ParamWidget::create<MedLEDButton>(Vec(95, 140), module, Buttons::RESET_PARAM9, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<MedLight<GreenLight>>(Vec(100, 145), module, Buttons::RESET_LIGHT9));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Buttons) {
   Model *modelButtons = Model::create<Buttons, ButtonsWidget>("RJModules", "Buttons", "[LIVE] Buttons", UTILITY_TAG);
   return modelButtons;
}
