#include "Template.hpp"

namespace rack_plugin_PG_Instruments {

struct PGPanner : Module
{
    enum ParamIds
    {
        PAN_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        INPUT,
        PAN_INPUT,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NUM_OUTPUTS
    };
    
    enum LightIds
    {
        RUNNING_LIGHT,
        NUM_LIGHTS
    };
    
    float panning = 0.5f;
    
    PGPanner() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
        onReset();
    }
    
    void onReset() override
    {
    }
    
    void onRandomize() override
    {
        
    }
    
    void step() override
    {
        panning = params[PAN_PARAM].value;
        
        float mono = inputs[INPUT].value;
        float panInput = inputs[PAN_INPUT].value;
        float pan = panning + panInput;
        
        float leftGain = cosf(pan * M_PI / 2.0f) * mono;
        float rightGain = sinf(pan * M_PI / 2.0f) * mono;
        
        outputs[LEFT_OUTPUT].value = leftGain;
        outputs[RIGHT_OUTPUT].value = rightGain;
    }
};

struct PGPannerWidget : ModuleWidget
{
    PGPannerWidget(PGPanner *module) : ModuleWidget(module)
    {
        setPanel(SVG::load(assetPlugin(plugin, "res/PGPanner.svg")));
        
  		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

        addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(20, 40), module, PGPanner::PAN_PARAM, 0.0f, 1.0, 0.5f));
        
        addInput(Port::create<PJ301MPort>(Vec(26, 100), Port::INPUT, module, PGPanner::INPUT));
        addInput(Port::create<PJ301MPort>(Vec(26, 160), Port::INPUT, module, PGPanner::PAN_INPUT));
        addOutput(Port::create<PJ301MPort>(Vec(12, 220), Port::OUTPUT, module, PGPanner::LEFT_OUTPUT));
        addOutput(Port::create<PJ301MPort>(Vec(42, 220), Port::OUTPUT, module, PGPanner::RIGHT_OUTPUT));
    }
};

} // namespace rack_plugin_PG_Instruments

using namespace rack_plugin_PG_Instruments;

RACK_PLUGIN_MODEL_INIT(PG_Instruments, PGPanner) {
   Model *modelPGPanner = Model::create<PGPanner, PGPannerWidget>("PG-Instruments", "PGPanner", "PG Panner", ATTENUATOR_TAG);
   return modelPGPanner;
}
