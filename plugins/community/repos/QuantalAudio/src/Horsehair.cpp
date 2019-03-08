#include "QuantalAudio.hpp"
#include "dsp/resampler.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_QuantalAudio {

template <int OVERSAMPLE, int QUALITY>
struct VoltageControlledOscillator {
    //float lastSyncValue = 0.0f;
    float phase = 0.0f;
    float freq;
    float pw = 0.5f;
    float pitch;

    Decimator<OVERSAMPLE, QUALITY> sinDecimator;
    Decimator<OVERSAMPLE, QUALITY> sawDecimator;
    Decimator<OVERSAMPLE, QUALITY> sqrDecimator;

    float sinBuffer[OVERSAMPLE] = {};
    float sawBuffer[OVERSAMPLE] = {};
    float sqrBuffer[OVERSAMPLE] = {};

    void setPitch(float octave, float pitchKnob, float pitchCv) {
        // Compute frequency
        float pitch = 1.0 + roundf(octave);
        pitch += pitchKnob;
        pitch += pitchCv / 12.0;

        // Note C4
        freq = 261.626f * powf(2.0f, pitch);
        freq = clamp(freq, 0.0f, 20000.0f);
    }

    void setPulseWidth(float pulseWidth) {
        const float pwMin = 0.01f;
        pw = clamp(pulseWidth, pwMin, 1.0f - pwMin);
    }

    void process(float deltaTime) {
        // Advance phase
        float deltaPhase = clamp(freq * deltaTime, 1e-6, 0.5f);

        for (int i = 0; i < OVERSAMPLE; i++) {
            sinBuffer[i] = sinf(2.f*M_PI * phase);

            if (phase < 0.5f)
                sawBuffer[i] = 2.f * phase;
            else
                sawBuffer[i] = -2.f + 2.f * phase;

            sqrBuffer[i] = (phase < pw) ? 1.f : -1.f;

            // Advance phase
            phase += deltaPhase / OVERSAMPLE;
            phase = eucmod(phase, 1.0f);
        }
    }

    float sin() {
        return sinDecimator.process(sinBuffer);
    }
    float saw() {
        return sawDecimator.process(sawBuffer);
    }
    float sqr() {
        return sqrDecimator.process(sqrBuffer);
    }
    float light() {
        return sinf(2*M_PI * phase);
    }
};

struct Horsehair : Module {
    enum ParamIds {
        PITCH_PARAM,
        ENUMS(OCTAVE_PARAM, 2),
        ENUMS(SHAPE_PARAM, 2),
        ENUMS(PW_PARAM, 2),
        MIX_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        PITCH_INPUT,
        ENUMS(SHAPE_CV_INPUT, 2),
        ENUMS(PW_CV_INPUT, 2),
        MIX_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        SIN_OUTPUT,
        MIX_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightsIds {
        OSC_LIGHT,
        NUM_LIGHTS
    };

    VoltageControlledOscillator<16, 16> oscillator;
    VoltageControlledOscillator<16, 16> oscillator2;

    Horsehair() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override {
        float pitchCv = 12.0f * inputs[PITCH_INPUT].value;
        oscillator.setPitch(params[OCTAVE_PARAM + 0].value, params[PITCH_PARAM].value, pitchCv);
        oscillator.setPulseWidth(params[PW_PARAM + 0].value + inputs[PW_CV_INPUT + 0].value / 10.0);
        oscillator.process(engineGetSampleTime());

        oscillator2.setPitch(params[OCTAVE_PARAM + 1].value, params[PITCH_PARAM].value, pitchCv);
        oscillator2.setPulseWidth(params[PW_PARAM + 1].value + inputs[PW_CV_INPUT + 1].value / 10.0);
        oscillator2.process(engineGetSampleTime());

        float shape = clamp(params[SHAPE_PARAM + 0].value, 0.0f, 1.0f);
        float shape2 = clamp(params[SHAPE_PARAM + 1].value, 0.0f, 1.0f);

        if (inputs[SHAPE_CV_INPUT + 0].active) {
            shape += inputs[SHAPE_CV_INPUT + 0].value / 10.0;
            shape = clamp(shape, 0.0f, 1.0f);
        }
        if (inputs[SHAPE_CV_INPUT + 1].active) {
            shape2 += inputs[SHAPE_CV_INPUT + 1].value / 10.0;
            shape2 = clamp(shape2, 0.0f, 1.0f);
        }

        float out = 0.0f;
        float out2 = 0.0f;
        if (outputs[MIX_OUTPUT].active) {
            out = crossfade(oscillator.sqr(), oscillator.saw(), shape);
            out2 = crossfade(oscillator2.sqr(), oscillator2.saw(), shape2);
        }

        float mix = clamp(params[MIX_PARAM].value + inputs[MIX_CV_INPUT].value / 10.0, 0.0f, 1.0f);

        outputs[MIX_OUTPUT].value = 5.0f * crossfade(out, out2, mix);
        outputs[SIN_OUTPUT].value = 5.0f * oscillator.sin();

        //lights[OSC_LIGHT].setBrightnessSmooth(fmaxf(0.0f, oscillator.light()));
        //lights[PHASE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -oscillator.light()));
    }
};

struct HorsehairWidget : ModuleWidget {
    HorsehairWidget(Horsehair *module) : ModuleWidget(module) {
        setPanel(SVG::load(assetPlugin(plugin, "res/Horsehair.svg")));

        // Screws
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Pitch & CV
        addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(RACK_GRID_WIDTH * 4 + 3, 50.0), module, Horsehair::PITCH_PARAM, -2.0f, 2.0f, 0.0f));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH + 3, 50.0), Port::INPUT, module, Horsehair::PITCH_INPUT));

        // Octave
        addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(RACK_GRID_WIDTH, 93.0), module, Horsehair::OCTAVE_PARAM + 0, -5.0f, 4.0f, -2.0f));
        addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(RACK_GRID_WIDTH * 4, 93.0), module, Horsehair::OCTAVE_PARAM + 1, -5.0f, 4.0f, -1.0f));

        // Shape
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(RACK_GRID_WIDTH, 142.0), module, Horsehair::SHAPE_PARAM + 0, 0.0f, 1.0f, 0.0f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(RACK_GRID_WIDTH * 4, 142.0), module, Horsehair::SHAPE_PARAM + 1, 0.0f, 1.0f, 1.0f));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 11.5, 172.0), Port::INPUT, module, Horsehair::SHAPE_CV_INPUT + 0));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH * 4 + 16.5, 172.0), Port::INPUT, module, Horsehair::SHAPE_CV_INPUT + 1));

        // Pulse width
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(RACK_GRID_WIDTH, 215.0), module, Horsehair::PW_PARAM + 0, 0.0f, 1.0f, 0.5f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(RACK_GRID_WIDTH * 4, 215.0), module, Horsehair::PW_PARAM + 1, 0.0f, 1.0f, 0.5f));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 11.5, 245.0), Port::INPUT, module, Horsehair::PW_CV_INPUT + 0));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH * 4 + 16.5, 245.0), Port::INPUT, module, Horsehair::PW_CV_INPUT + 1));

        // Osc Mix
        addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(RACK_GRID_WIDTH * 3.5 - (38.0/2), 264.0), module, Horsehair::MIX_PARAM, 0.0f, 1.0f, 0.5f));
        addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH - 8, 277.0), Port::INPUT, module, Horsehair::MIX_CV_INPUT));

        // Output
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH + 3, 320.0), Port::OUTPUT, module, Horsehair::MIX_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH * 4 + 3, 320.0), Port::OUTPUT, module, Horsehair::SIN_OUTPUT));

        // Lights
        //addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(68, 42.5f), module, Horsehair::OSC_LIGHT));
    }
};

} // namespace rack_plugin_QuantalAudio

using namespace rack_plugin_QuantalAudio;

RACK_PLUGIN_MODEL_INIT(QuantalAudio, Horsehair) {
   Model *modelHorsehair = Model::create<Horsehair, HorsehairWidget>("QuantalAudio", "Horsehair", "Horsehair VCO | 7HP", OSCILLATOR_TAG);
   return modelHorsehair;
}
