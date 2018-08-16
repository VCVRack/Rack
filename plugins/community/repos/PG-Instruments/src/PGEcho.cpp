#include "Template.hpp"

namespace rack_plugin_PG_Instruments {

#define BUFFER_SIZE 65536 * 2
#define BUFFER_MASK (BUFFER_SIZE - 1)

struct PGEcho : Module
{
    enum ParamIds
    {
        TIME_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        INPUT,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        OUTPUT,
        NUM_OUTPUTS
    };

    int reader;
    int writer;
    int offset;
    float buffer[BUFFER_SIZE];
    
    PGEcho() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 0)
    {
        reader = 0;
        offset = BUFFER_SIZE >> 1;
        writer = offset;
        
        for(int i = 0; i < BUFFER_SIZE; i++)
            buffer[i] = 0.0f;
    }
    
    void step() override
    {
        int echoOffset = (int)(params[TIME_PARAM].value * BUFFER_SIZE);
        
        if (echoOffset != offset)
        {
            offset = echoOffset;
            writer = (reader - offset) & BUFFER_MASK;
        }
        
        float input = inputs[INPUT].value;

        outputs[OUTPUT].value = input + buffer[reader];
        buffer[writer] = input + buffer[writer] * params[FEEDBACK_PARAM].value;
        
        reader++;
        writer++;
        
        reader &= BUFFER_MASK;
        writer &= BUFFER_MASK;
    }
};

struct PGEchoWidget : ModuleWidget
{
    PGEchoWidget(PGEcho *module) : ModuleWidget(module)
    {
        setPanel(SVG::load(assetPlugin(plugin, "res/PGEcho.svg")));
        
  		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(Port::create<PJ301MPort>(Vec(30, 100), Port::INPUT, module, PGEcho::INPUT));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(70, 100), module, PGEcho::TIME_PARAM, 0.0f, 1.0f, 0.5f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 100), module, PGEcho::FEEDBACK_PARAM, 0.0f, 1.0f, 0.5f));
        addOutput(Port::create<PJ301MPort>(Vec(150, 100), Port::OUTPUT, module, PGEcho::OUTPUT));
    }
};

} // namespace rack_plugin_PG_Instruments

using namespace rack_plugin_PG_Instruments;

RACK_PLUGIN_MODEL_INIT(PG_Instruments, PGEcho) {
   Model *modelPGEcho = Model::create<PGEcho, PGEchoWidget>("PG-Instruments", "PGEcho", "PG Echo", DELAY_TAG);
   return modelPGEcho;
}
