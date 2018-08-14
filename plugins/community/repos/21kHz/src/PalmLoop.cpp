#include "21kHz.hpp"
#include "dsp/digital.hpp"
#include "dsp/math.hpp"
#include <array>


using std::array;

namespace rack_plugin_21kHz {

struct PalmLoop : Module {
	enum ParamIds {
        OCT_PARAM,
        COARSE_PARAM,
        FINE_PARAM,
        EXP_FM_PARAM,
        LIN_FM_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
        RESET_INPUT,
        V_OCT_INPUT,
        EXP_FM_INPUT,
        LIN_FM_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
        SAW_OUTPUT,
        SQR_OUTPUT,
        TRI_OUTPUT,
        SIN_OUTPUT,
        SUB_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
    
    float phase = 0.0f;
    float oldPhase = 0.0f;
    float square = 1.0f;
    
    int discont = 0;
    int oldDiscont = 0;
    
    array<float, 4> sawBuffer;
    array<float, 4> sqrBuffer;
    array<float, 4> triBuffer;
    
    float log2sampleFreq = 15.4284f;
    
    SchmittTrigger resetTrigger;

	PalmLoop() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
    void onSampleRateChange() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void PalmLoop::onSampleRateChange() {
    log2sampleFreq = log2f(1 / engineGetSampleTime()) - 0.00009f;
}


// quick explanation: the whole thing is driven by a naive sawtooth, which writes to a four-sample buffer for each
// (non-sine) waveform. the waves are calculated such that their discontinuities (or in the case of triangle, derivative
// discontinuities) only occur each time the phasor exceeds a [0, 1) range. when we calculate the outputs, we look to see
// if a discontinuity occured in the previous sample. if one did, we calculate the polyblep or polyblamp and add it to
// each sample in the buffer. the output is the oldest buffer sample, which gets overwritten in the following step.
void PalmLoop::step() {
    if (resetTrigger.process(inputs[RESET_INPUT].value)) {
        phase = 0.0f;
    }
    
    for (int i = 0; i <= 2; ++i) {
        sawBuffer[i] = sawBuffer[i + 1];
        sqrBuffer[i] = sqrBuffer[i + 1];
        triBuffer[i] = triBuffer[i + 1];
    }
    
    float freq = params[OCT_PARAM].value + 0.031360 + 0.083333 * params[COARSE_PARAM].value + params[FINE_PARAM].value + inputs[V_OCT_INPUT].value;
    if (inputs[EXP_FM_INPUT].active) {
        freq += params[EXP_FM_PARAM].value * inputs[EXP_FM_INPUT].value;
        if (freq >= log2sampleFreq) {
            freq = log2sampleFreq;
        }
        freq = powf(2.0f, freq);
    }
    else {
        if (freq >= log2sampleFreq) {
            freq = log2sampleFreq;
        }
        freq = powf(2.0f, freq);
    }
    float incr = 0.0f;
    if (inputs[LIN_FM_INPUT].active) {
        freq += params[LIN_FM_PARAM].value * params[LIN_FM_PARAM].value * inputs[LIN_FM_INPUT].value;
        incr = engineGetSampleTime() * freq;
        if (incr > 1.0f) {
            incr = 1.0f;
        }
        else if (incr < -1.0f) {
            incr = -1.0f;
        }
    }
    else {
        incr = engineGetSampleTime() * freq;
    }
    
    phase += incr;
    if (phase >= 0.0f && phase < 1.0f) {
        discont = 0;
    }
    else if (phase >= 1.0f) {
        discont = 1;
        --phase;
        square *= -1.0f;
    }
    else {
        discont = -1;
        ++phase;
        square *= -1.0f;
    }
    
    sawBuffer[3] = phase;
    sqrBuffer[3] = square;
    if (square >= 0.0f) {
        triBuffer[3] = phase;
    }
    else {
        triBuffer[3] = 1.0f - phase;
    }
    
    if (outputs[SAW_OUTPUT].active) {
        if (oldDiscont == 1) {
            polyblep4(sawBuffer, 1.0f - oldPhase / incr, 1.0f);
        }
        else if (oldDiscont == -1) {
            polyblep4(sawBuffer, 1.0f - (oldPhase - 1.0f) / incr, -1.0f);
        }
        outputs[SAW_OUTPUT].value = clampf(10.0f * (sawBuffer[0] - 0.5f), -5.0f, 5.0f);
    }
    if (outputs[SQR_OUTPUT].active) {
        // for some reason i don't understand, if discontinuities happen in two
        // adjacent samples, the first one must be inverted. otherwise the polyblep
        // is bad and causes aliasing. don't ask me how i managed to figure this out.
        if (discont == 0) {
            if (oldDiscont == 1) {
                polyblep4(sqrBuffer, 1.0f - oldPhase / incr, -2.0f * square);
            }
            else if (oldDiscont == -1) {
                polyblep4(sqrBuffer, 1.0f - (oldPhase - 1.0f) / incr, -2.0f * square);
            }
        }
        else {
            if (oldDiscont == 1) {
                polyblep4(sqrBuffer, 1.0f - oldPhase / incr, 2.0f * square);
            }
            else if (oldDiscont == -1) {
                polyblep4(sqrBuffer, 1.0f - (oldPhase - 1.0f) / incr, 2.0f * square);
            }
        }
        outputs[SQR_OUTPUT].value = clampf(4.9999f * sqrBuffer[0], -5.0f, 5.0f);
    }
    if (outputs[TRI_OUTPUT].active) {
        if (discont == 0) {
            if (oldDiscont == 1) {
                polyblamp4(triBuffer, 1.0f - oldPhase / incr, 2.0f * square * incr);
            }
            else if (oldDiscont == -1) {
                polyblamp4(triBuffer, 1.0f - (oldPhase - 1.0f) / incr, 2.0f * square * incr);
            }
        }
        else {
            if (oldDiscont == 1) {
                polyblamp4(triBuffer, 1.0f - oldPhase / incr, -2.0f * square * incr);
            }
            else if (oldDiscont == -1) {
                polyblamp4(triBuffer, 1.0f - (oldPhase - 1.0f) / incr, -2.0f * square * incr);
            }
        }
        outputs[TRI_OUTPUT].value = clampf(10.0f * (triBuffer[0] - 0.5f), -5.0f, 5.0f);
    }
    if (outputs[SIN_OUTPUT].active) {
        outputs[SIN_OUTPUT].value = 5.0f * sin_01(phase);
    }
    if (outputs[SUB_OUTPUT].active) {
        if (square >= 0.0f) {
            outputs[SUB_OUTPUT].value = 5.0f * sin_01(0.5f * phase);
        }
        else {
            outputs[SUB_OUTPUT].value = 5.0f * sin_01(0.5f * (1.0f - phase));
        }
    }
    
    oldPhase = phase;
    oldDiscont = discont;
}


struct PalmLoopWidget : ModuleWidget {
	PalmLoopWidget(PalmLoop *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Panels/PalmLoop.svg")));

		addChild(Widget::create<kHzScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<kHzScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<kHzScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<kHzScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(ParamWidget::create<kHzKnobSnap>(Vec(36, 40), module, PalmLoop::OCT_PARAM, 4, 12, 8));
        
        addParam(ParamWidget::create<kHzKnobSmallSnap>(Vec(16, 112), module, PalmLoop::COARSE_PARAM, -7, 7, 0));
        addParam(ParamWidget::create<kHzKnobSmall>(Vec(72, 112), module, PalmLoop::FINE_PARAM, -0.083333, 0.083333, 0.0));
        
        addParam(ParamWidget::create<kHzKnobSmall>(Vec(16, 168), module, PalmLoop::EXP_FM_PARAM, -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<kHzKnobSmall>(Vec(72, 168), module, PalmLoop::LIN_FM_PARAM, -40.0, 40.0, 0.0));
        
        addInput(Port::create<kHzPort>(Vec(10, 234), Port::INPUT, module, PalmLoop::EXP_FM_INPUT));
        addInput(Port::create<kHzPort>(Vec(47, 234), Port::INPUT, module, PalmLoop::V_OCT_INPUT));
        addInput(Port::create<kHzPort>(Vec(84, 234), Port::INPUT, module, PalmLoop::LIN_FM_INPUT));
        
        addInput(Port::create<kHzPort>(Vec(10, 276), Port::INPUT, module, PalmLoop::RESET_INPUT));
        addOutput(Port::create<kHzPort>(Vec(47, 276), Port::OUTPUT, module, PalmLoop::SAW_OUTPUT));
        addOutput(Port::create<kHzPort>(Vec(84, 276), Port::OUTPUT, module, PalmLoop::SIN_OUTPUT));
        
        addOutput(Port::create<kHzPort>(Vec(10, 318), Port::OUTPUT, module, PalmLoop::SQR_OUTPUT));
        addOutput(Port::create<kHzPort>(Vec(47, 318), Port::OUTPUT, module, PalmLoop::TRI_OUTPUT));
        addOutput(Port::create<kHzPort>(Vec(84, 318), Port::OUTPUT, module, PalmLoop::SUB_OUTPUT));
        
	}
};

} // namespace rack_plugin_21kHz

using namespace rack_plugin_21kHz;

RACK_PLUGIN_MODEL_INIT(21kHz, PalmLoop) {
   Model *modelPalmLoop = Model::create<PalmLoop, PalmLoopWidget>("21kHz", "kHzPalmLoop", "Palm Loop — basic VCO — 8hp", OSCILLATOR_TAG);
   return modelPalmLoop;
}

// history
// 0.6.0
//	create
// 0.6.1
//	minor optimizations
//	coarse goes -7 to +7
//	waveform labels & rearrangement on panel
