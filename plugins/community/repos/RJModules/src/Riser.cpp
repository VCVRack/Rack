#include "dsp/digital.hpp"
#include <iostream>
#include "RJModules.hpp"

namespace rack_plugin_RJModules {

struct LowFrequencyOscillator {
    float phase = 0.0;
    float pw = 0.5;
    float freq = 1.0;
    bool offset = false;
    bool invert = false;

    SchmittTrigger resetTrigger;
    LowFrequencyOscillator() {}

    void setPitch(float pitch) {
        pitch = fminf(pitch, 8.0);
        freq = powf(2.0, pitch);
    }

    float getFreq() {
        return freq;
    }
    void setReset(float reset) {
        if (resetTrigger.process(reset)) {
            phase = 0.0;
        }
    }
    void step(float dt) {
        float deltaPhase = fminf(freq * dt, 0.5);
        phase += deltaPhase;
        if (phase >= 1.0)
            phase -= 1.0;
    }
    float sin() {
        if (offset)
            return 1.0 - cosf(2*M_PI * phase) * (invert ? -1.0 : 1.0);
        else
            return sinf(2*M_PI * phase) * (invert ? -1.0 : 1.0);
    }
    float saw(float x) {
        return 2.0 * (x - roundf(x));
    }
    float saw() {
        if (offset)
            return invert ? 2.0 * (1.0 - phase) : 2.0 * phase;
        else
            return saw(phase) * (invert ? -1.0 : 1.0);
    }
    float light() {
        return sinf(2*M_PI * phase);
    }
};

struct Riser : Module {
    enum ParamIds {
        OFFSET_PARAM,
        INVERT_PARAM,
        FREQ_PARAM,
        FREQ_PARAM_2,
        DETUNE_PARAM,
        SHAPE_PARAM,
        THREE_OSC_PARAM,

        NUM_PARAMS
    };
    enum InputIds {
        FREQ_CV_INPUT,
        FREQ_CV_INPUT_2,
        SHAPE_CV_INPUT,
        RESET_INPUT,
        PW_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        SAW_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        PHASE_POS_LIGHT,
        PHASE_NEG_LIGHT,
        PHASE_POS_LIGHT2,
        PHASE_NEG_LIGHT2,
        NUM_LIGHTS
    };

    LowFrequencyOscillator oscillator;
    LowFrequencyOscillator oscillator2;
    float osc0_root = 5.0;

    float max = 10.0;
    float min = 6.0;

    float r_step = .00003;

    Riser() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void Riser::step() {

    if (osc0_root >= max){
        osc0_root = min;
    } else {
        osc0_root = osc0_root + r_step;
    }

    oscillator.setPitch(osc0_root);
    //oscillator.setPitch(-4.0);
    oscillator.step(0.3 / engineGetSampleRate());

    // float root_pitch2 = params[FREQ_PARAM_2].value * clamp(inputs[FREQ_CV_INPUT_2].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
    // oscillator2.setPitch(root_pitch2);
    // oscillator2.offset = (params[OFFSET_PARAM].value > 0.0);
    // oscillator2.invert = (params[INVERT_PARAM].value <= 0.0);
    // oscillator2.step(0.3 / engineGetSampleRate());
    // oscillator2.setReset(inputs[RESET_INPUT].value);

    // float root_pitch = params[FREQ_PARAM].value * clamp(inputs[FREQ_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f) * clamp(oscillator2.sin(), 0.0f, 1.0f);
    // oscillator.setPitch(root_pitch);
    // oscillator.offset = (params[OFFSET_PARAM].value > 0.0);
    // oscillator.invert = (params[INVERT_PARAM].value <= 0.0);
    // oscillator.step(0.3 / engineGetSampleRate());
    // oscillator.setReset(inputs[RESET_INPUT].value);

    //float shape_percent = params[SHAPE_PARAM].value * clamp(inputs[SHAPE_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
    //float mixed = ((oscillator.sin() * shape_percent)) + (oscillator.saw() * (1-shape_percent));
    float mixed = oscillator.saw();

    // This is from 0 to 2V. Should be mapped?
    outputs[SAW_OUTPUT].value = mixed;

    lights[PHASE_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, oscillator.light()));
    lights[PHASE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -oscillator.light()));

    lights[PHASE_POS_LIGHT2].setBrightnessSmooth(fmaxf(0.0, oscillator2.light()));
    lights[PHASE_NEG_LIGHT2].setBrightnessSmooth(fmaxf(0.0, -oscillator2.light()));
}

struct RiserWidget: ModuleWidget {
    RiserWidget(Riser *module);
};

RiserWidget::RiserWidget(Riser *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Riser.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addParam(ParamWidget::create<CKSS>(Vec(119, 100), module, Riser::OFFSET_PARAM, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<CKSS>(Vec(119, 180), module, Riser::INVERT_PARAM, 0.0, 1.0, 1.0));

    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 61), module, Riser::FREQ_PARAM, 0.0, 8.0, 5.0));
    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 143), module, Riser::FREQ_PARAM_2, 0.0, 8.0, 0.5));
    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 228), module, Riser::SHAPE_PARAM, 0.0, 1.0, 1.0));

    addInput(Port::create<PJ301MPort>(Vec(22, 100), Port::INPUT, module, Riser::FREQ_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 190), Port::INPUT, module, Riser::FREQ_CV_INPUT_2));
    addInput(Port::create<PJ301MPort>(Vec(22, 270), Port::INPUT, module, Riser::SHAPE_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(38, 310), Port::INPUT, module, Riser::RESET_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(100, 310), Port::OUTPUT, module, Riser::SAW_OUTPUT));

    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(99, 60), module, Riser::PHASE_POS_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(99, 140), module, Riser::PHASE_POS_LIGHT2));
}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Riser) {
   Model *modelRiser = Model::create<Riser, RiserWidget>("RJModules", "Riser", "[GEN] Riser", LFO_TAG);
   return modelRiser;
}
