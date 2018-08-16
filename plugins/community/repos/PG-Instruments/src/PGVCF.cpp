#include "Template.hpp"

namespace rack_plugin_PG_Instruments {

struct PGVCF : Module
{
    enum ParamIds
    {
        FREQUENCY_PARAM,
        RESONANCE_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        INPUT,
        CUTOFF,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LOWPASS_OUTPUT,
        BANDPASS_OUTPUT,
        HIGHPASS_OUTPUT,
        NUM_OUTPUTS
    };
    
    float buf0;
    float buf1;
    float buf2;
    float buf3;
    float feedback;
    
    PGVCF() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 0)
    {
        buf0 = buf1 = buf2 = buf3 = 0.0f;
    }
    
    void step() override
    {
        calculateFeedback();
        
        float calcCutoff = getCalculatedCutoff();
        
        buf0 += calcCutoff * (inputs[INPUT].value - buf0 + feedback * (buf0 - buf1));
        buf1 += calcCutoff * (buf0 - buf1);
        buf2 += calcCutoff * (buf1 - buf2);
        buf3 += calcCutoff * (buf2 - buf3);
        
        outputs[LOWPASS_OUTPUT].value = buf3;
        outputs[BANDPASS_OUTPUT].value = buf0 - buf3;
        outputs[HIGHPASS_OUTPUT].value = inputs[INPUT].value - buf3;
    }
    
    void calculateFeedback()
    {
        feedback = params[RESONANCE_PARAM].value + params[RESONANCE_PARAM].value / (1.0f - getCalculatedCutoff());
    }
    
    float getCalculatedCutoff()
    {
        return fmax(fmin(params[FREQUENCY_PARAM].value + inputs[CUTOFF].value, 0.99f), 0.01f);
    }
};

struct PGVCFWidget : ModuleWidget
{
    PGVCFWidget(PGVCF *module) : ModuleWidget(module)
    {
        setPanel(SVG::load(assetPlugin(plugin, "res/PGVCF.svg")));
        
  		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(Port::create<PJ301MPort>(Vec(30, 100), Port::INPUT, module, PGVCF::INPUT));
        addInput(Port::create<PJ301MPort>(Vec(30, 140), Port::INPUT, module, PGVCF::CUTOFF));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(70, 100), module, PGVCF::FREQUENCY_PARAM, 0.0f, 1.0f, 0.5f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 100), module, PGVCF::RESONANCE_PARAM, 0.0f, 0.99f, 0.5f));
        addOutput(Port::create<PJ301MPort>(Vec(150, 100), Port::OUTPUT, module, PGVCF::LOWPASS_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(150, 140), Port::OUTPUT, module, PGVCF::BANDPASS_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(150, 180), Port::OUTPUT, module, PGVCF::HIGHPASS_OUTPUT));
    }
};

} // namespace rack_plugin_PG_Instruments

using namespace rack_plugin_PG_Instruments;

RACK_PLUGIN_MODEL_INIT(PG_Instruments, PGVCF) {
   Model *modelPGVCF = Model::create<PGVCF, PGVCFWidget>("PG-Instruments", "PGVCF", "PG VCF", ATTENUATOR_TAG);
   return modelPGVCF;
}
