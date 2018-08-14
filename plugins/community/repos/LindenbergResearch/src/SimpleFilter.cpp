#include "LindenbergResearch.hpp"

namespace rack_plugin_LindenbergResearch {

struct SimpleFilter : Module {
    enum ParamIds {
        CUTOFF_PARAM,
        RESONANCE_PARAM,
        CUTOFF_CV_PARAM,
        RESONANCE_CV_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        FILTER_INPUT,
        CUTOFF_CV_INPUT,
        RESONANCE_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        FILTER_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float f, p, q;
    float b0, b1, b2, b3, b4;
    float t1, t2;
    float frequency, resonance, in;


    SimpleFilter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
        f = 0;
        p = 0;
        q = 0;
        b0 = 0;
        b1 = 0;
        b2 = 0;
        b3 = 0;
        b4 = 0;
        t1 = 0;
        t2 = 0;
        frequency = 0;
        resonance = 0;
        in = 0;
    }


    void step() override;


    // For more advanced Module features, read Rack's engine.hpp header file
    // - toJson, fromJson: serialization of internal data
    // - onSampleRateChange: event triggered by a change of sample rate
    // - reset, randomize: implements special behavior when user clicks these from the context menu
};


float clip(float in, float level) {
    // clipping high
    if (in > level) {
        in = level;
    }

    // clipping low
    if (in < -level) {
        in = -level;
    }

    return in;
}


void SimpleFilter::step() {
    // Moog 24 dB/oct resonant lowpass VCF
    // References: CSound source code, Stilson/Smith CCRMA paper.
    // Modified by paul.kellett@maxim.abel.co.uk July 2000
    //
    // Adapted for VCV Rack by Lindenberg Research
    // http://musicdsp.org/showArchiveComment.php?ArchiveID=25

    // calculate CV inputs
    float cutoffCVValue = (inputs[CUTOFF_CV_INPUT].value * 0.05f * params[CUTOFF_CV_PARAM].value);
    float resonanceCVValue = (inputs[RESONANCE_CV_INPUT].value * 0.1f * params[RESONANCE_CV_PARAM].value);

    // translate frequency to logarithmic scale
    float freqHz = 20.f * powf(1000.f, params[CUTOFF_PARAM].value + cutoffCVValue);
    frequency = clip(freqHz * (1.f / (engineGetSampleRate() / 2.0f)), 1.f);
    resonance = clip(params[RESONANCE_PARAM].value + resonanceCVValue, 1.f);

    // normalize signal input to [-1.0...+1.0]
    // filter starts to be very unstable for input gain above 1.f and below 0.f
    in = clip(inputs[FILTER_INPUT].value * 0.1f, 1.0f);

    // Set coefficients given frequency & resonance [0.0...1.0]
    q = 1.0f - frequency;
    p = frequency + 0.8f * frequency * q;
    f = p + p - 1.0f;
    q = resonance * (1.0f + 0.5f * q * (1.0f - q + 5.6f * q * q));


    in -= q * b4;

    t1 = b1;
    b1 = (in + b0) * p - b1 * f;

    t2 = b2;
    b2 = (b1 + t1) * p - b2 * f;

    t1 = b3;
    b3 = (b2 + t2) * p - b3 * f;

    b4 = (b3 + t1) * p - b4 * f;

    b4 = b4 - b4 * b4 * b4 * 0.166666667f;
    b0 = in;

    // Lowpass  output:  b4
    // Highpass output:  in - b4;
    // Bandpass output:  3.0f * (b3 - b4);


    // scale normalized output back to +/-5V
    outputs[FILTER_OUTPUT].value = clip(b4, 1.0f) * 5.0f;
}


/**
 * @brief Recover of old filer
 */
struct SimpleFilterWidget : LRModuleWidget {
    SimpleFilterWidget(SimpleFilter *module);
};


SimpleFilterWidget::SimpleFilterWidget(SimpleFilter *module) : LRModuleWidget(module) {
    //setPanel(SVG::load(assetPlugin(plugin, "res/SimpleFilter.svg")));

    panel = new LRPanel();
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/SimpleFilter.svg")));
    addChild(panel);

    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewDarkA>(Vec(15, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(15, 366)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    addParam(ParamWidget::create<LRBigKnob>(Vec(75 - 28, 167), module, SimpleFilter::CUTOFF_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRMiddleKnob>(Vec(75 - 21, 252), module, SimpleFilter::RESONANCE_PARAM, -0.f, 1.f, 0.0f));
    // ***** MAIN KNOBS ******

    // ***** CV INPUTS *******
    addParam(ParamWidget::create<LRSmallKnob>(Vec(39 - 12, 120), module, SimpleFilter::CUTOFF_CV_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(111 - 12, 120), module, SimpleFilter::RESONANCE_CV_PARAM, 0.f, 1.f, 0.f));

    addInput(Port::create<LRIOPort>(Vec(39 - 14, 60), Port::INPUT, module, SimpleFilter::CUTOFF_CV_INPUT));
    addInput(Port::create<LRIOPort>(Vec(111 - 14, 60), Port::INPUT, module, SimpleFilter::RESONANCE_CV_INPUT));
    // ***** CV INPUTS *******

    // ***** INPUTS **********
    addInput(Port::create<LRIOPort>(Vec(39 - 14, 320), Port::INPUT, module, SimpleFilter::FILTER_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPort>(Vec(111 - 14, 320), Port::OUTPUT, module, SimpleFilter::FILTER_OUTPUT));
    // ***** OUTPUTS *********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, SimpleFilter) {
   Model *modelSimpleFilter = Model::create<SimpleFilter, SimpleFilterWidget>("Lindenberg Research", "LPFilter24dB", "24dB Lowpass Filter",
                                                                              FILTER_TAG);
   return modelSimpleFilter;
}

