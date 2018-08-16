#include "Template.hpp"

namespace rack_plugin_PG_Instruments {

#define HALF_BUFFER_SIZE 65536
#define BUFFER_SIZE HALF_BUFFER_SIZE * 2
#define BUFFER_MASK (BUFFER_SIZE - 1)

struct PGStereoPingPongEcho : Module
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
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NUM_OUTPUTS
    };

    int reader;
    int writer;
    int writer2;
    int offset;
    float buffer[BUFFER_SIZE];
    
    PGStereoPingPongEcho() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 0)
    {
        reader = 0;
        offset = BUFFER_SIZE >> 1;
        writer = offset;
        writer2 = BUFFER_SIZE - 1;
        
        for(int i = 0; i < BUFFER_SIZE; i++)
        {
            buffer[i] = 0.0f;
        }
    }
    
    void step() override
    {
        int timeParam = (int)(params[TIME_PARAM].value * BUFFER_SIZE);
        
        if (timeParam != offset)
        {
            offset = timeParam;
            writer = (reader - offset) & BUFFER_MASK;
            writer2 = (reader - offset * 2 - 1) & BUFFER_MASK;
        }
        
        float input;

        input = inputs[INPUT].value;
        outputs[LEFT_OUTPUT].value = input * 0.7f + buffer[reader] + buffer[writer];
        outputs[RIGHT_OUTPUT].value = input * 0.7f + buffer[reader] + buffer[writer2];
        
        buffer[writer] = input + buffer[writer] * params[FEEDBACK_PARAM].value;
        buffer[writer2] = input + buffer[writer2] * params[FEEDBACK_PARAM].value / 2.0f;
        
        reader++;
        writer++;
        writer2++;
        
        reader &= BUFFER_MASK;
        writer &= BUFFER_MASK;
        writer2 &= BUFFER_MASK;
    }
};

struct PGStereoPingPongEchoWidget : ModuleWidget
{
    PGStereoPingPongEchoWidget(PGStereoPingPongEcho *module) : ModuleWidget(module)
    {
        setPanel(SVG::load(assetPlugin(plugin, "res/PGStereoPingPongEcho.svg")));
        
  		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(Port::create<PJ301MPort>(Vec(30, 100), Port::INPUT, module, PGStereoPingPongEcho::INPUT));
        
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(70, 100), module, PGStereoPingPongEcho::TIME_PARAM, 0.0f, 1.0f, 0.5f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 100), module, PGStereoPingPongEcho::FEEDBACK_PARAM, 0.0f, 1.0f, 0.5f));
        
        addOutput(Port::create<PJ301MPort>(Vec(150, 100), Port::OUTPUT, module, PGStereoPingPongEcho::LEFT_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(150, 140), Port::OUTPUT, module, PGStereoPingPongEcho::RIGHT_OUTPUT));
    }
};

} // namespace rack_plugin_PG_Instruments

using namespace rack_plugin_PG_Instruments;

RACK_PLUGIN_MODEL_INIT(PG_Instruments, PGStereoPingPongEcho) {
   Model *modelPGStereoPingPongEcho = Model::create<PGStereoPingPongEcho, PGStereoPingPongEchoWidget>("PG-Instruments", "PGStereoPingPongEcho", "PG Stereo Ping Pong Echo", DELAY_TAG);
   return modelPGStereoPingPongEcho;
}
