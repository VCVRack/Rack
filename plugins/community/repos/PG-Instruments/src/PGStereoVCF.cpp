#include "Template.hpp"

namespace rack_plugin_PG_Instruments {

struct PGStereoVCF : Module
{
    enum ParamIds
    {
        FREQUENCY_PARAM,
        RESONANCE_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        LEFT_INPUT,
        RIGHT_INPUT,
        CUTOFF,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LEFT_LOWPASS_OUTPUT,
        RIGHT_LOWPASS_OUTPUT,
        
        LEFT_BANDPASS_OUTPUT,
        RIGHT_BANDPASS_OUTPUT,
        
        LEFT_HIGHPASS_OUTPUT,
        RIGHT_HIGHPASS_OUTPUT,

        NUM_OUTPUTS
    };
    
    float buf0[2];
    float buf1[2];
    float buf2[2];
    float buf3[2];
    float feedback;
    
    PGStereoVCF() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 0)
    {
        for(int i = 0; i < 2; i++)
            buf0[i] = buf1[i] = buf2[i] = buf3[i] = 0.0f;
    }
    
    void step() override
    {
        calculateFeedback();
        
        float calcCutoff = getCalculatedCutoff();
        
        for(int i = 0; i < 2; i++)
        {
            float input;
            
            if (i)
                input = inputs[RIGHT_INPUT].value;
            else
                input = inputs[LEFT_INPUT].value;
            
            buf0[i] += calcCutoff * (input - buf0[i] + feedback * (buf0[i] - buf1[i]));
            buf1[i] += calcCutoff * (buf0[i] - buf1[i]);
            buf2[i] += calcCutoff * (buf1[i] - buf2[i]);
            buf3[i] += calcCutoff * (buf2[i] - buf3[i]);
            
            if (i)
            {
                outputs[RIGHT_LOWPASS_OUTPUT].value = buf3[i];
                outputs[RIGHT_BANDPASS_OUTPUT].value = buf0[i] - buf3[i];
                outputs[RIGHT_HIGHPASS_OUTPUT].value = inputs[RIGHT_INPUT].value - buf3[i];
            }
            else
            {
                outputs[LEFT_LOWPASS_OUTPUT].value = buf3[i];
                outputs[LEFT_BANDPASS_OUTPUT].value = buf0[i] - buf3[i];
                outputs[LEFT_HIGHPASS_OUTPUT].value = inputs[LEFT_INPUT].value - buf3[i];
            }
        }
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

struct PGStereoVCFWidget : ModuleWidget
{
    PGStereoVCFWidget(PGStereoVCF *module) : ModuleWidget(module)
    {
        setPanel(SVG::load(assetPlugin(plugin, "res/PGStereoVCF.svg")));
        
  		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(Port::create<PJ301MPort>(Vec(20, 100), Port::INPUT, module, PGStereoVCF::LEFT_INPUT));
        addInput(Port::create<PJ301MPort>(Vec(20, 140), Port::INPUT, module, PGStereoVCF::RIGHT_INPUT));
        
        addInput(Port::create<PJ301MPort>(Vec(20, 180), Port::INPUT, module, PGStereoVCF::CUTOFF));

        addParam(ParamWidget::create<RoundBlackKnob>(Vec(50, 100), module, PGStereoVCF::FREQUENCY_PARAM, 0.0f, 1.0f, 0.5f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(90, 100), module, PGStereoVCF::RESONANCE_PARAM, 0.0f, 0.99f, 0.5f));

        addOutput(Port::create<PJ301MPort>(Vec(130, 100), Port::OUTPUT, module, PGStereoVCF::LEFT_LOWPASS_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(130, 140), Port::OUTPUT, module, PGStereoVCF::LEFT_BANDPASS_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(130, 180), Port::OUTPUT, module, PGStereoVCF::LEFT_HIGHPASS_OUTPUT));

        addOutput(Port::create<PJ301MPort>(Vec(160, 100), Port::OUTPUT, module, PGStereoVCF::RIGHT_LOWPASS_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(160, 140), Port::OUTPUT, module, PGStereoVCF::RIGHT_BANDPASS_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(160, 180), Port::OUTPUT, module, PGStereoVCF::RIGHT_HIGHPASS_OUTPUT));
    }
};

} // namespace rack_plugin_PG_Instruments

using namespace rack_plugin_PG_Instruments;

RACK_PLUGIN_MODEL_INIT(PG_Instruments, PGStereoVCF) {
   Model *modelPGStereoVCF = Model::create<PGStereoVCF, PGStereoVCFWidget>("PG-Instruments", "PGStereoVCF", "PG Stereo VCF", ATTENUATOR_TAG);
   return modelPGStereoVCF;
}
