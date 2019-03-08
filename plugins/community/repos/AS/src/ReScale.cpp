//**************************************************************************************
//Volt rescale module module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//**************************************************************************************
#include "AS.hpp"

struct ReScale: Module {
    enum ParamIds {
		CONVERT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_0,
        INPUT_1,
		INPUT_2,
		INPUT_3,
        NUM_INPUTS
    };
    enum OutputIds {
	    OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    int selection = 0;
    float rescaled_value = 0.0f;
        float input_value = 0.0f;

    float getNoteInVolts(float noteValue) {
        int octaveInVolts = int(floorf(noteValue));
        float voltMinusOct = noteValue - octaveInVolts;
        return voltMinusOct;
    }

    ReScale() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

    }
    void step() override;
    
};

void ReScale::step() {

    selection = params[CONVERT_PARAM].value;

    if(inputs[INPUT_0].active){

        input_value = clamp(inputs[INPUT_0].value, -5.0f,5.0f);
        if(selection==0){
            rescaled_value = input_value;
        }else if(selection==1){
            rescaled_value = rescale(input_value, -5.0f, 5.0f, 0.0f, 5.0f);
        }else if(selection==2){
            rescaled_value = rescale(input_value, -5.0f, 5.0f, -10.0f, 10.0f);
        }else if(selection==3){
            rescaled_value = rescale(input_value, -5.0f, 5.0f, 0.0f, 10.0f);
        }

    }else if(inputs[INPUT_1].active){

        input_value = clamp(inputs[INPUT_1].value, 0.0f, 5.0f);
        if(selection==0){
            rescaled_value = rescale(input_value, 0.0f, 5.0f, -5.0f, 5.0f);
        }else if(selection==1){
            rescaled_value = input_value;
        }else if(selection==2){
            rescaled_value = rescale(input_value, 0.0f, 5.0f, -10.0f, 10.0f);
        }else if(selection==3){
            rescaled_value = rescale(input_value, -5.0f, 5.0f, 0.0f, 10.0f);
        }

    }else if(inputs[INPUT_2].active){
        
        input_value = clamp(inputs[INPUT_2].value, 0.0f, 10.0f);
        if(selection==0){
            rescaled_value = rescale(input_value, 0.0f, 10.0f, -5.0f, 5.0f);
        }else if(selection==1){
            rescaled_value = rescale(input_value, 0.0f, 10.0f, 0.0f, 5.0f);        
        }else if(selection==2){
            rescaled_value = rescale(input_value, 0.0f, 10.0f, -10.0f, 10.0f);
        }else if(selection==3){
            rescaled_value = input_value;
        }

    }else if(inputs[INPUT_3].active){
        
        input_value = inputs[INPUT_3].value;
        if(selection==0){
            rescaled_value = input_value;
        }else if(selection==1){
            rescaled_value = input_value;      
        }else if(selection==2){
            rescaled_value = input_value;
        }else if(selection==3){
            //take the input of a midi KB, get the voltage minus octave, convert it to 1V/KEY
            float ext_key = getNoteInVolts(input_value);
            rescaled_value = clamp( rescale( ext_key, 0.0f, 1.0f, 0.0f, 11.0f ), 0.0f, 10.0f );
        }

    }
    outputs[OUTPUT].value = rescaled_value;
    
}

////////////////////////////////////
struct ReScaleWidget : ModuleWidget 
{ 
    ReScaleWidget(ReScale *module);
};

ReScaleWidget::ReScaleWidget(ReScale *module) : ModuleWidget(module) {

  	setPanel(SVG::load(assetPlugin(plugin, "res/ReScale.svg")));
  
	//SCREWS - SPECIAL SPACING FOR RACK WIDTH*4
	addChild(Widget::create<as_HexScrew>(Vec(0, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    //PORTS
	addInput(Port::create<as_PJ301MPort>(Vec(18, 65), Port::INPUT, module, ReScale::INPUT_0));
	addInput(Port::create<as_PJ301MPort>(Vec(18, 105), Port::INPUT, module, ReScale::INPUT_1));
	addInput(Port::create<as_PJ301MPort>(Vec(18, 145), Port::INPUT, module, ReScale::INPUT_2));
	addInput(Port::create<as_PJ301MPort>(Vec(18, 185), Port::INPUT, module, ReScale::INPUT_3));

	addParam(ParamWidget::create<as_KnobBlackSnap4>(Vec(12, 230), module, ReScale::CONVERT_PARAM, 0.0f, 3.0f, 0.0f));

	addOutput(Port::create<as_PJ301MPort>(Vec(18, 280), Port::OUTPUT, module, ReScale::OUTPUT));


}

RACK_PLUGIN_MODEL_INIT(AS, ReScale) {
   Model *modelReScale = Model::create<ReScale, ReScaleWidget>("AS", "ReScale", "Voltage Converter", UTILITY_TAG);
   return modelReScale;
}
