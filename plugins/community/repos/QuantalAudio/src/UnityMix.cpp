#include "QuantalAudio.hpp"

namespace rack_plugin_QuantalAudio {

struct UnityMix : Module {
    enum ParamIds {
        CONNECT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(CH_INPUT, 6),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(CH_OUTPUT, 2),
        NUM_OUTPUTS
    };
    enum LightsIds {
        NUM_LIGHTS
    };

    UnityMix() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    float mixchannels(int in_start, int in_end) {
        float mix = 0.f;
        float channels = 0.f;

        for (int i = in_start; i <= in_end; i++) {
            if (inputs[CH_INPUT + i].active) {
                mix += inputs[CH_INPUT + i].value;
                channels++;
            }
        }
        if (channels > 0.f)
            mix = mix / channels;

        return mix;
    }

    void step() override {
        bool unconnect = (params[CONNECT_PARAM].value > 0.0f);

        if (unconnect) {
            // Group A : Inputs 0 1 2 -> Output 0
            outputs[CH_OUTPUT + 0].value = mixchannels(0, 2);
            // Group B : Inputs 3 4 5 -> Output 1
            outputs[CH_OUTPUT + 1].value = mixchannels(3, 5);
        } else {
            // Combined : Inputs 0-5 -> Output 1 & 2
            float mix = mixchannels(0, 5);
            outputs[CH_OUTPUT + 0].value = mix;
            outputs[CH_OUTPUT + 1].value = mix;
        }
    }
};

struct UnityMixWidget : ModuleWidget {
    UnityMixWidget(UnityMix *module) : ModuleWidget(module) {
        setPanel(SVG::load(assetPlugin(plugin, "res/UnityMix.svg")));

        // Screws
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Connect switch
        addParam(ParamWidget::create<CKSS>(Vec(RACK_GRID_WIDTH - 7.0, 182.0), module, UnityMix::CONNECT_PARAM, 0.0f, 1.0f, 1.0f));

        // Group A
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 50.0), Port::INPUT, module, UnityMix::CH_INPUT + 0));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 78.0), Port::INPUT, module, UnityMix::CH_INPUT + 1));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 106.0), Port::INPUT, module, UnityMix::CH_INPUT + 2));
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 148.0), Port::OUTPUT, module, UnityMix::CH_OUTPUT + 0));

        // Group B
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 222.0), Port::INPUT, module, UnityMix::CH_INPUT + 3));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 250.0), Port::INPUT, module, UnityMix::CH_INPUT + 4));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 278.0), Port::INPUT, module, UnityMix::CH_INPUT + 5));
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 320.0), Port::OUTPUT, module, UnityMix::CH_OUTPUT + 1));
    }
};

} // namespace rack_plugin_QuantalAudio

using namespace rack_plugin_QuantalAudio;

RACK_PLUGIN_MODEL_INIT(QuantalAudio, UnityMix) {
   Model *modelUnityMix = Model::create<UnityMix, UnityMixWidget>("QuantalAudio", "UnityMix", "Unity Mix | 2HP", MULTIPLE_TAG);
   return modelUnityMix;
}
