//**************************************************************************************
//Flow module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//**************************************************************************************
#include "AS.hpp"
#include "dsp/digital.hpp"

struct Flow: Module {
    enum ParamIds {
        SWITCH_1,
        SWITCH_2,
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_1,
        INPUT_2,
        RESET_1,
        RESET_2,
        CV_TRIG_INPUT_1,
        CV_TRIG_INPUT_2,
        NUM_INPUTS
    };
    enum OutputIds {
        OUTPUT_1,
        OUTPUT_2,
        NUM_OUTPUTS
    };
    enum LightIds {
        TRIGGER_LED_1,
        TRIGGER_LED_2,
        NUM_LIGHTS
    };

    SchmittTrigger btnTrigger1;
    SchmittTrigger extTrigger1;
    SchmittTrigger extReset1;
    SchmittTrigger btnTrigger2;
    SchmittTrigger extTrigger2;
    SchmittTrigger extReset2;

    bool on_1 = false;
    bool on_2 = false;

    float mute_fade1 =0.0f;
    float mute_fade2 =0.0f;
    const float fade_speed = 0.001f;

    Flow() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
 
  	json_t *toJson()override {
		json_t *rootJm = json_object();

		json_t *on_statesJ = json_array();
		
			json_t *on_stateJ1 = json_integer((int) on_1);
			json_t *on_stateJ2 = json_integer((int) on_2);

			json_array_append_new(on_statesJ, on_stateJ1);
			json_array_append_new(on_statesJ, on_stateJ2);
		
		json_object_set_new(rootJm, "as_FlowStates", on_statesJ);

		return rootJm;
	}

	void fromJson(json_t *rootJm)override {
		json_t *on_statesJ = json_object_get(rootJm, "as_FlowStates");
		
			json_t *on_stateJ1 = json_array_get(on_statesJ, 0);
			json_t *on_stateJ2 = json_array_get(on_statesJ, 1);

			on_1 = !!json_integer_value(on_stateJ1);
			on_2 = !!json_integer_value(on_stateJ2);
		
	}
    
};

void Flow::step() {

    //TRIGGER 1
    if (btnTrigger1.process(params[SWITCH_1].value)||extTrigger1.process(inputs[CV_TRIG_INPUT_1].value)) {
        on_1 = !on_1; 
    }
    if (extReset1.process(inputs[RESET_1].value)) {
        on_1 = false; 
    }
  //SOFT MUTE/UNMUTE
    mute_fade1 -= on_1 ? fade_speed : -fade_speed;
    if ( mute_fade1 < 0.0f ) {
      mute_fade1 = 0.0f;
    } else if ( mute_fade1 > 1.0f ) {
      mute_fade1 = 1.0f;
    }
    outputs[OUTPUT_1].value = inputs[INPUT_1].value * mute_fade1;
    lights[TRIGGER_LED_1].value = on_1 ? 1.0f : 0.0f;
    //TRIGGER 2
    if (btnTrigger2.process(params[SWITCH_2].value)||extTrigger2.process(inputs[CV_TRIG_INPUT_2].value)) {
        on_2 = !on_2; 
    }
    if (extReset2.process(inputs[RESET_2].value)) {
        on_2 = false; 
    }
    //SOFT MUTE/UNMUTE
    mute_fade2 -= on_2 ? fade_speed : -fade_speed;
    if ( mute_fade2 < 0.0f ) {
      mute_fade2 = 0.0f;
    } else if ( mute_fade2 > 1.0f ) {
      mute_fade2 = 1.0f;
    }
    outputs[OUTPUT_2].value = inputs[INPUT_2].value * mute_fade2;
    lights[TRIGGER_LED_2].value = on_2 ? 1.0f : 0.0f;
}

////////////////////////////////////
struct FlowWidget : ModuleWidget 
{ 
    FlowWidget(Flow *module);
};

FlowWidget::FlowWidget(Flow *module) : ModuleWidget(module) {

  setPanel(SVG::load(assetPlugin(plugin, "res/Flow.svg")));
  
	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    static const float led_offset = 3.3;
    static const float led_center = 15;
    static const float y_offset = 150;
    //TRIGGER 1
    //SWITCH
    addParam(ParamWidget::create<BigLEDBezel>(Vec(led_center, 50), module, Flow::SWITCH_1, 0.0, 1.0, 0.0));
  	addChild(ModuleLightWidget::create<GiantLight<RedLight>>(Vec(led_center+led_offset, 50+led_offset), module, Flow::TRIGGER_LED_1));
    //PORTS
	addInput(Port::create<as_PJ301MPort>(Vec(10, 140), Port::INPUT, module, Flow::CV_TRIG_INPUT_1));
	addInput(Port::create<as_PJ301MPort>(Vec(55, 140), Port::INPUT, module, Flow::RESET_1));
	addInput(Port::create<as_PJ301MPort>(Vec(10, 174), Port::INPUT, module, Flow::INPUT_1));
	addOutput(Port::create<as_PJ301MPort>(Vec(55, 174), Port::OUTPUT, module, Flow::OUTPUT_1));
    //TRIGGER 2
    //SWITCH
    addParam(ParamWidget::create<BigLEDBezel>(Vec(led_center, 50+y_offset), module, Flow::SWITCH_2, 0.0, 1.0, 0.0));
  	addChild(ModuleLightWidget::create<GiantLight<RedLight>>(Vec(led_center+led_offset, 50+led_offset+y_offset), module, Flow::TRIGGER_LED_2));
    //PORTS
	addInput(Port::create<as_PJ301MPort>(Vec(10, 140+y_offset), Port::INPUT, module, Flow::CV_TRIG_INPUT_2));
	addInput(Port::create<as_PJ301MPort>(Vec(55, 140+y_offset), Port::INPUT, module, Flow::RESET_2));
	addInput(Port::create<as_PJ301MPort>(Vec(10, 174+y_offset), Port::INPUT, module, Flow::INPUT_2));
	addOutput(Port::create<as_PJ301MPort>(Vec(55, 174+y_offset), Port::OUTPUT, module, Flow::OUTPUT_2));

}

RACK_PLUGIN_MODEL_INIT(AS, Flow) {
   Model *modelFlow = Model::create<Flow, FlowWidget>("AS", "Flow", "Flow",  SWITCH_TAG, UTILITY_TAG);
   return modelFlow;
}
