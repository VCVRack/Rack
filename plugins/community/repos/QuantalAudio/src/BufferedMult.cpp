#include "QuantalAudio.hpp"

namespace rack_plugin_QuantalAudio {

struct BufferedMult : Module {
    enum ParamIds {
        CONNECT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(CH_INPUT, 2),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(CH_OUTPUT, 6),
        NUM_OUTPUTS
    };
    enum LightsIds {
        NUM_LIGHTS
    };

    BufferedMult() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override {
        bool unconnect = (params[CONNECT_PARAM].value > 0.0f);

        // Input 0 -> Outputs 0 1 2
        float ch = inputs[CH_INPUT + 0].value;
        for (int i = 0; i <= 2; i++) {
            outputs[CH_OUTPUT + i].value = ch;
        }

        // Input 1 -> Outputs 3 4 5
        // otherwise Outputs 3 4 5 is copy of Input 0
        if (unconnect) {
            ch = inputs[CH_INPUT + 1].value;
        }

        for (int i = 3; i <= 5; i++) {
            outputs[CH_OUTPUT + i].value = ch;
        }
    }
};

struct BufferedMultWidget : ModuleWidget {
    BufferedMultWidget(BufferedMult *module) : ModuleWidget(module) {
        setPanel(SVG::load(assetPlugin(plugin, "res/BufferedMult.svg")));

        // Screws
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Connect switch
        addParam(ParamWidget::create<CKSS>(Vec(RACK_GRID_WIDTH - 7.0, 182.0), module, BufferedMult::CONNECT_PARAM, 0.0f, 1.0f, 1.0f));

        // Group A
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 50.0), Port::INPUT, module, BufferedMult::CH_INPUT + 0));
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 92.0), Port::OUTPUT, module, BufferedMult::CH_OUTPUT + 0));
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 120.0), Port::OUTPUT, module, BufferedMult::CH_OUTPUT + 1));
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 148.0), Port::OUTPUT, module, BufferedMult::CH_OUTPUT + 2));

        // Group B
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 222.0), Port::INPUT, module, BufferedMult::CH_INPUT + 1));
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 264.0), Port::OUTPUT, module, BufferedMult::CH_OUTPUT + 3));
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 292.0), Port::OUTPUT, module, BufferedMult::CH_OUTPUT + 4));
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 12.5, 320.0), Port::OUTPUT, module, BufferedMult::CH_OUTPUT + 5));
    }
};

} // namespace rack_plugin_QuantalAudio

using namespace rack_plugin_QuantalAudio;

RACK_PLUGIN_MODEL_INIT(QuantalAudio, BufferedMult) {
   Model *modelBufferedMult = Model::create<BufferedMult, BufferedMultWidget>("QuantalAudio", "BufferedMult", "Buffered Mult | 2HP", MULTIPLE_TAG);
   return modelBufferedMult;
}
