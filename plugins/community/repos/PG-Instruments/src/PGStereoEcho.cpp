#include "Template.hpp"

namespace rack_plugin_PG_Instruments {

#define HALF_BUFFER_SIZE 65536
#define BUFFER_SIZE HALF_BUFFER_SIZE * 2
#define BUFFER_MASK (BUFFER_SIZE - 1)

struct PGStereoEcho : Module
{
    enum ParamIds
    {
        TIME_PARAM,
        OFFSET_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        LEFT_INPUT,
        RIGHT_INPUT,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NUM_OUTPUTS
    };

    int reader;
    int leftWriter;
    int rightWriter;
    int offset;
    int rightOffset;
    float leftBuffer[BUFFER_SIZE];
    float rightBuffer[BUFFER_SIZE];
    
    PGStereoEcho() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 0)
    {
        reader = 0;
        offset = BUFFER_SIZE >> 1;
        rightOffset = 0;
        leftWriter = offset;
        rightWriter = offset;
        
        for(int i = 0; i < BUFFER_SIZE; i++)
        {
            leftBuffer[i] = 0.0f;
            rightBuffer[i] = 0.0f;
        }
    }
    
    void step() override
    {
        int timeParam = (int)(params[TIME_PARAM].value * BUFFER_SIZE);
        int offsetParam = (int)(params[OFFSET_PARAM].value * HALF_BUFFER_SIZE);
        
        if (timeParam != offset || offsetParam != rightOffset)
        {
            offset = timeParam;
            rightOffset = offsetParam;
            leftWriter = (reader - offset) & BUFFER_MASK;
            rightWriter = (reader - offset - rightOffset) & BUFFER_MASK;
        }
        
        float input;

        input = inputs[LEFT_INPUT].value;
        outputs[LEFT_OUTPUT].value = input + leftBuffer[reader];
        leftBuffer[leftWriter] = input + leftBuffer[leftWriter] * params[FEEDBACK_PARAM].value;
        
        input = inputs[RIGHT_INPUT].value;
        outputs[RIGHT_OUTPUT].value = input + rightBuffer[reader];
        rightBuffer[rightWriter] = input + rightBuffer[rightWriter] * params[FEEDBACK_PARAM].value;
        
        reader++;
        leftWriter++;
        rightWriter++;
        
        reader &= BUFFER_MASK;
        leftWriter &= BUFFER_MASK;
        rightWriter &= BUFFER_MASK;
    }
};

struct PGStereoEchoWidget : ModuleWidget
{
    PGStereoEchoWidget(PGStereoEcho *module) : ModuleWidget(module)
    {
        setPanel(SVG::load(assetPlugin(plugin, "res/PGStereoEcho.svg")));
        
  		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(Port::create<PJ301MPort>(Vec(30, 100), Port::INPUT, module, PGStereoEcho::LEFT_INPUT));
        addInput(Port::create<PJ301MPort>(Vec(30, 140), Port::INPUT, module, PGStereoEcho::RIGHT_INPUT));
        
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(70, 100), module, PGStereoEcho::TIME_PARAM, 0.0f, 1.0f, 0.5f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 100), module, PGStereoEcho::FEEDBACK_PARAM, 0.0f, 1.0f, 0.5f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(70, 140), module, PGStereoEcho::OFFSET_PARAM, 0.0f, 1.0f, 0.0f));
        
        addOutput(Port::create<PJ301MPort>(Vec(150, 100), Port::OUTPUT, module, PGStereoEcho::LEFT_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(150, 140), Port::OUTPUT, module, PGStereoEcho::RIGHT_OUTPUT));
    }
};

} // namespace rack_plugin_PG_Instruments

using namespace rack_plugin_PG_Instruments;

RACK_PLUGIN_MODEL_INIT(PG_Instruments, PGStereoEcho) {
   Model *modelPGStereoEcho = Model::create<PGStereoEcho, PGStereoEchoWidget>("PG-Instruments", "PGStereoEcho", "PG Stereo Echo", DELAY_TAG);
   return modelPGStereoEcho;
}
