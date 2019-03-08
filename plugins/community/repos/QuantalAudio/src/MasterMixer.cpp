#include "QuantalAudio.hpp"

namespace rack_plugin_QuantalAudio {

struct MasterMixer : Module {
    enum ParamIds {
        MIX_LVL_PARAM,
        MONO_PARAM,
        ENUMS(LVL_PARAM, 2),
        NUM_PARAMS
    };
    enum InputIds {
        MIX_CV_INPUT,
        ENUMS(CH_INPUT, 2),
        NUM_INPUTS
    };
    enum OutputIds {
        MIX_OUTPUT,
        MIX_OUTPUT_2,
        ENUMS(CH_OUTPUT, 2),
        NUM_OUTPUTS
    };

    MasterMixer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}

    void step() override {
        float mix = 0.f;
        for (int i = 0; i < 2; i++) {
            float ch = inputs[CH_INPUT + i].value;
            ch *= powf(params[LVL_PARAM + i].value, 2.f);
            outputs[CH_OUTPUT + i].value = ch;
            mix += ch;
        }
        mix *= params[MIX_LVL_PARAM].value;

        bool is_mono = (params[MONO_PARAM].value > 0.0f);

        float mix_cv = 1.f;
        if (inputs[MIX_CV_INPUT].active)
            mix_cv = clamp(inputs[MIX_CV_INPUT].value / 10.f, 0.f, 1.f);

        if (!is_mono && (inputs[CH_INPUT + 0].active && inputs[CH_INPUT + 1].active)) {
            // If the ch 2 jack is active use stereo mode
            float attenuate = params[MIX_LVL_PARAM].value * mix_cv;
            outputs[MIX_OUTPUT].value = outputs[CH_OUTPUT + 0].value * attenuate;
            outputs[MIX_OUTPUT_2].value = outputs[CH_OUTPUT + 1].value * attenuate;
        } else {
            // Otherwise use mono->stereo mode
            mix *= mix_cv;
            outputs[MIX_OUTPUT].value = mix;
            outputs[MIX_OUTPUT_2].value = mix;
        }
    }
};

struct MasterMixerWidget : ModuleWidget {
    MasterMixerWidget(MasterMixer *module) : ModuleWidget(module) {
        setPanel(SVG::load(assetPlugin(plugin, "res/MasterMixer.svg")));

        // Screws
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Level & CV
        addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(RACK_GRID_WIDTH * 2.5 - (38.0/2), 52.0), module, MasterMixer::MIX_LVL_PARAM, 0.0, 2.0, 1.0));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH * 2.5 - (25.0/2), 96.0), Port::INPUT, module, MasterMixer::MIX_CV_INPUT));

        // Mono/stereo switch
        addParam(ParamWidget::create<CKSS>(Vec(RACK_GRID_WIDTH * 2.5 - 7.0, 162.0), module, MasterMixer::MONO_PARAM, 0.0f, 1.0f, 1.0f));

        // LEDs
        addParam(ParamWidget::create<LEDSliderGreen>(Vec(RACK_GRID_WIDTH * 2.5 - (21.0 + 7.0), 130.4), module, MasterMixer::LVL_PARAM + 0, 0.0, 1.0, 1.0));
        addParam(ParamWidget::create<LEDSliderGreen>(Vec(RACK_GRID_WIDTH * 2.5 + 7.0, 130.4), module, MasterMixer::LVL_PARAM + 1, 0.0, 1.0, 1.0));

        // Channel inputs
        addInput(Port::create<PJ301MPort>(Vec((RACK_GRID_WIDTH * 2.5) - (25.0 + 5.0), 232.0), Port::INPUT, module, MasterMixer::CH_INPUT + 0));
        addInput(Port::create<PJ301MPort>(Vec((RACK_GRID_WIDTH * 2.5) + 5.0, 232.0), Port::INPUT, module, MasterMixer::CH_INPUT + 1));

        // Channel outputs
        addOutput(Port::create<PJ301MPort>(Vec((RACK_GRID_WIDTH * 2.5) - (25.0 + 5.0), 276.0), Port::OUTPUT, module, MasterMixer::CH_OUTPUT + 0));
        addOutput(Port::create<PJ301MPort>(Vec((RACK_GRID_WIDTH * 2.5) + 5.0, 276.0), Port::OUTPUT, module, MasterMixer::CH_OUTPUT + 1));

        // Mix outputs
        addOutput(Port::create<PJ301MPort>(Vec((RACK_GRID_WIDTH * 2.5) - (25.0 + 5.0), 320.0), Port::OUTPUT, module, MasterMixer::MIX_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec((RACK_GRID_WIDTH * 2.5) + 5.0, 320.0), Port::OUTPUT, module, MasterMixer::MIX_OUTPUT_2));
    }
};

} // namespace rack_plugin_QuantalAudio

using namespace rack_plugin_QuantalAudio;

RACK_PLUGIN_MODEL_INIT(QuantalAudio, MasterMixer) {
   Model *modelMasterMixer = Model::create<MasterMixer, MasterMixerWidget>("QuantalAudio", "Mixer2", "Mixer 2 | Mono->Stereo | 5HP", MIXER_TAG, AMPLIFIER_TAG);
   return modelMasterMixer;
}
