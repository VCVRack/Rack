#include "../dsp/LadderFilter.hpp"
#include "../LindenbergResearch.hpp"
#include "../LRModel.hpp"

namespace rack_plugin_LindenbergResearch {
using namespace lrt;

struct AlmaFilter : LRModule {

    enum ParamIds {
        CUTOFF_PARAM,
        RESONANCE_PARAM,
        DRIVE_PARAM,
        SLOPE_PARAM,
        CUTOFF_CV_PARAM,
        RESONANCE_CV_PARAM,
        DRIVE_CV_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        FILTER_INPUT,
        CUTOFF_CV_INPUT,
        RESONANCE_CV_INPUT,
        DRIVE_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        LP_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        OVERLOAD_LIGHT,
        NUM_LIGHTS
    };

    dsp::LadderFilter *filter = new dsp::LadderFilter(engineGetSampleRate());

    LRBigKnob *frqKnob = NULL;
    LRMiddleKnob *peakKnob = NULL;
    LRMiddleKnob *driveKnob = NULL;


    AlmaFilter() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


    void step() override;
    void onSampleRateChange() override;
};


void AlmaFilter::step() {
    float frqcv = inputs[CUTOFF_CV_INPUT].value * 0.1f * quadraticBipolar(params[CUTOFF_CV_PARAM].value);
    float rescv = inputs[RESONANCE_CV_INPUT].value * 0.1f * quadraticBipolar(params[RESONANCE_CV_PARAM].value);
    float drvcv = inputs[DRIVE_CV_INPUT].value * 0.1f * quadraticBipolar(params[DRIVE_CV_PARAM].value);

    filter->setFrequency(params[CUTOFF_PARAM].value + frqcv);
    filter->setResonance(params[RESONANCE_PARAM].value + rescv);
    filter->setDrive(params[DRIVE_PARAM].value + drvcv);
    filter->setSlope(params[SLOPE_PARAM].value);


    /* pass modulated parameter to knob widget for cv indicator */
    if (frqKnob != NULL && peakKnob != NULL && driveKnob != NULL) {
        frqKnob->setIndicatorActive(inputs[CUTOFF_CV_INPUT].active);
        peakKnob->setIndicatorActive(inputs[RESONANCE_CV_INPUT].active);
        driveKnob->setIndicatorActive(inputs[DRIVE_CV_INPUT].active);

        frqKnob->setIndicatorValue(params[CUTOFF_PARAM].value + frqcv);
        peakKnob->setIndicatorValue(params[RESONANCE_PARAM].value + rescv);
        driveKnob->setIndicatorValue(params[DRIVE_PARAM].value + drvcv);
    }


    float y = inputs[FILTER_INPUT].value;

    filter->setIn(y);
    filter->process();

    outputs[LP_OUTPUT].value = filter->getLpOut();


    lights[OVERLOAD_LIGHT].value = filter->getLightValue();
}


void AlmaFilter::onSampleRateChange() {
    Module::onSampleRateChange();
    filter->setSamplerate(engineGetSampleRate());
}


/**
 * @brief ALMA filter
 */
struct AlmaFilterWidget : LRModuleWidget {
    AlmaFilterWidget(AlmaFilter *module);
};


AlmaFilterWidget::AlmaFilterWidget(AlmaFilter *module) : LRModuleWidget(module) {
    panel->addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/panels/VCF.svg")));
    panel->addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/panels/AlmaLight.svg")));
    panel->addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/panels/AlmaAged.svg")));

    panel->init();
    addChild(panel);
    box.size = panel->box.size;

    // ***** SCREWS **********
    addChild(Widget::create<ScrewLight>(Vec(15, 1)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewLight>(Vec(15, 366)));
    addChild(Widget::create<ScrewLight>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    module->frqKnob = LRKnob::create<LRBigKnob>(Vec(62, 150), module, AlmaFilter::CUTOFF_PARAM, 0.f, 1.f, 0.8f);
    module->peakKnob = LRKnob::create<LRMiddleKnob>(Vec(23, 228), module, AlmaFilter::RESONANCE_PARAM, -0.f, 1.5, 0.0f);
    module->driveKnob = LRKnob::create<LRMiddleKnob>(Vec(115, 227), module, AlmaFilter::DRIVE_PARAM, 0.0f, 1.f, 0.0f);

    addParam(module->frqKnob);
    addParam(module->peakKnob);
    addParam(module->driveKnob);

    addParam(ParamWidget::create<LRMiddleKnob>(Vec(69, 287), module, AlmaFilter::SLOPE_PARAM, 0.0f, 4.f, 2.0f));
    // ***** MAIN KNOBS ******

    // ***** CV INPUTS *******
    addParam(ParamWidget::create<LRSmallKnob>(Vec(27.5, 106), module, AlmaFilter::RESONANCE_CV_PARAM, -1.f, 1.0f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(78, 106), module, AlmaFilter::CUTOFF_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(127.1, 106), module, AlmaFilter::DRIVE_CV_PARAM, -1.f, 1.f, 0.f));

    addInput(Port::create<LRIOPortCV>(Vec(26, 50), Port::INPUT, module, AlmaFilter::RESONANCE_CV_INPUT));
    addInput(Port::create<LRIOPortCV>(Vec(76, 50), Port::INPUT, module, AlmaFilter::CUTOFF_CV_INPUT));
    addInput(Port::create<LRIOPortCV>(Vec(125, 50), Port::INPUT, module, AlmaFilter::DRIVE_CV_INPUT));
    // ***** CV INPUTS *******

    // ***** INPUTS **********
    addInput(Port::create<LRIOPortAudio>(Vec(25, 326.5), Port::INPUT, module, AlmaFilter::FILTER_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPortAudio>(Vec(124.5, 326.5), Port::OUTPUT, module, AlmaFilter::LP_OUTPUT));
    // ***** OUTPUTS *********

    // ***** LIGHTS **********
    addChild(ModuleLightWidget::create<LRLight>(Vec(85, 247), module, AlmaFilter::OVERLOAD_LIGHT));
    // ***** LIGHTS **********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, AlmaFilter) {
   Model *modelAlmaFilter = Model::create<AlmaFilter, AlmaFilterWidget>("Lindenberg Research", "VCF", "Alma Ladder Filter", FILTER_TAG);
   return modelAlmaFilter;
}
