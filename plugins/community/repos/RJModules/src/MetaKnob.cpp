#include "RJModules.hpp"
#include "dsp/digital.hpp"
#include <iostream>
#include <cmath>

namespace rack_plugin_RJModules {

struct MetaKnob: Module {
    enum ParamIds {
        BIG_PARAM,
        RANGE_PARAM_1,
        RANGE_PARAM_2,
        NUM_PARAMS
    };
    enum InputIds {
        BIG_CV_INPUT,
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
        NUM_LIGHTS
    };
    float resetLight = 0.0;

    MetaKnob() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


struct RoundGiantBlackKnob : RoundKnob {
    RoundGiantBlackKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/RoundGiantBlackKnob.svg")));
    }
};

template <typename BASE>
struct GiantLight : BASE {
        GiantLight() {
                this->box.size = mm2px(Vec(34, 34));
        }
};

void MetaKnob::step() {

    float param = params[BIG_PARAM].value * clamp(inputs[BIG_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 10.0f);

    // BI
    outputs[CH1_OUTPUT].value = param;
    outputs[CH4_OUTPUT].value = param;
    outputs[CH7_OUTPUT].value = param;

    // UNI
    float uni_output = ( (param - (-5)) / (5 - (-5)) ) * (5 - 0) + 0;
    outputs[CH2_OUTPUT].value = uni_output;
    outputs[CH5_OUTPUT].value = uni_output;
    outputs[CH8_OUTPUT].value = uni_output;

    // RANGE
    float range_output = ( (param - (-5)) / (5 - (-5)) ) * (params[RANGE_PARAM_2].value - params[RANGE_PARAM_1].value) + params[RANGE_PARAM_1].value;
    outputs[CH9_OUTPUT].value = range_output;

}

struct MetaKnobWidget: ModuleWidget {
    MetaKnobWidget(MetaKnob *module);
};

MetaKnobWidget::MetaKnobWidget(MetaKnob *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/MetaKnob.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addInput(Port::create<PJ301MPort>(Vec(24, 160), Port::INPUT, module, MetaKnob::BIG_CV_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(24, 223), Port::OUTPUT, module, MetaKnob::CH1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(65, 223), Port::OUTPUT, module, MetaKnob::CH2_OUTPUT));
    //addOutput(Port::create<PJ301MPort>(Vec(105, 223), Port::OUTPUT, module, MetaKnob::CH3_OUTPUT));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(105, 223), module, MetaKnob::RANGE_PARAM_1, -5.0, 5.0, -5.0));


    addOutput(Port::create<PJ301MPort>(Vec(24, 274), Port::OUTPUT, module, MetaKnob::CH4_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(65, 274), Port::OUTPUT, module, MetaKnob::CH5_OUTPUT));
    //addOutput(Port::create<PJ301MPort>(Vec(106, 274), Port::OUTPUT, module, MetaKnob::CH6_OUTPUT));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(105, 274), module, MetaKnob::RANGE_PARAM_2, -5.0, 5.0, 5.0));

    addOutput(Port::create<PJ301MPort>(Vec(24, 324), Port::OUTPUT, module, MetaKnob::CH7_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(65, 324), Port::OUTPUT, module, MetaKnob::CH8_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(106, 324), Port::OUTPUT, module, MetaKnob::CH9_OUTPUT));

    addParam(ParamWidget::create<RoundGiantBlackKnob>(Vec(20, 55), module, MetaKnob::BIG_PARAM, -5.0, 5.0, 0.0));

}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, MetaKnob) {
   Model *modelMetaKnob = Model::create<MetaKnob, MetaKnobWidget>("RJModules", "MetaKnob", "[LIVE] MetaKnob", UTILITY_TAG);
   return modelMetaKnob;
}
