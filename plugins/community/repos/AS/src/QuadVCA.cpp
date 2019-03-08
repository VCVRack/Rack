//**************************************************************************************
//Quad QuadVCA module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Code adapted from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//**************************************************************************************

#include "AS.hpp"

struct QuadVCA : Module {
	enum ParamIds {
		GAIN1_PARAM,
		GAIN2_PARAM,
		GAIN3_PARAM,
		GAIN4_PARAM,
        MODE1_PARAM,
        MODE2_PARAM,
		MODE3_PARAM,
        MODE4_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GAIN1_CV_INPUT,
		IN1_INPUT,
		GAIN2_CV_INPUT,
		IN2_INPUT,
		GAIN3_CV_INPUT,
		IN3_INPUT,
		GAIN4_CV_INPUT,
		IN4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		GAIN1_LIGHT,
		GAIN2_LIGHT,
		GAIN3_LIGHT,
		GAIN4_LIGHT,
		NUM_LIGHTS
	};

	float v1= 0.0f;
	float v2= 0.0f;
	float v3= 0.0f;
	float v4= 0.0f;
	const float expBase = 50.0f;

	QuadVCA() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void QuadVCA::step() {
	//QuadVCA 1
	float out = 0.0;
	v1 = inputs[IN1_INPUT].value * params[GAIN1_PARAM].value;
	if(inputs[GAIN1_CV_INPUT].active){
		if(params[MODE1_PARAM].value==1){
			v1 *= clamp(inputs[GAIN1_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
		}else{
			v1 *= rescale(powf(expBase, clamp(inputs[GAIN1_CV_INPUT].value / 10.0f, 0.0f, 1.0f)), 1.0f, expBase, 0.0f, 1.0f);
		}
	}
	out+=v1;
	lights[GAIN1_LIGHT].setBrightnessSmooth(fmaxf(0.0f, out / 5.0f));
	if (outputs[OUT1_OUTPUT].active) {
			outputs[OUT1_OUTPUT].value = out;
			out = 0.0f;
	}
	//QuadVCA 2
	v2 = inputs[IN2_INPUT].value * params[GAIN2_PARAM].value;
	if(inputs[GAIN2_CV_INPUT].active){
		if(params[MODE2_PARAM].value){
			v2 *= clamp(inputs[GAIN2_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
		}else{
			v2 *= rescale(powf(expBase, clamp(inputs[GAIN2_CV_INPUT].value / 10.0f, 0.0f, 1.0f)), 1.0f, expBase, 0.0f, 1.0f);
		}
	}
	out+=v2;
	lights[GAIN2_LIGHT].setBrightnessSmooth(fmaxf(0.0f, out / 5.0f));
	if (outputs[OUT2_OUTPUT].active) {
			outputs[OUT2_OUTPUT].value = out;
			out = 0.0f;
	}
	//QuadVCA 3
	v3 = inputs[IN3_INPUT].value * params[GAIN3_PARAM].value;
	if(inputs[GAIN3_CV_INPUT].active){
		if(params[MODE3_PARAM].value){
			v3 *= clamp(inputs[GAIN3_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
		}else{
			v3 *= rescale(powf(expBase, clamp(inputs[GAIN3_CV_INPUT].value / 10.0f, 0.0f, 1.0f)), 1.0f, expBase, 0.0f, 1.0f);
		}
	}
	out+=v3;
	lights[GAIN3_LIGHT].setBrightnessSmooth(fmaxf(0.0f, out / 5.0f));
	if (outputs[OUT3_OUTPUT].active) {
			outputs[OUT3_OUTPUT].value = out;
			out = 0.0f;
	}
	//QuadVCA 4
	v4 = inputs[IN4_INPUT].value * params[GAIN4_PARAM].value;
	if(inputs[GAIN4_CV_INPUT].active){
		if(params[MODE4_PARAM].value){
			v4 *= clamp(inputs[GAIN4_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
		}else{
			v4 *= rescale(powf(expBase, clamp(inputs[GAIN4_CV_INPUT].value / 10.0f, 0.0f, 1.0f)), 1.0f, expBase, 0.0f, 1.0f);
		}
	}
	out+=v4;
	lights[GAIN4_LIGHT].setBrightnessSmooth(fmaxf(0.0f, out / 5.0f));
	if (outputs[OUT4_OUTPUT].active) {
			outputs[OUT4_OUTPUT].value = out;
			out = 0.0f;
	}
}

struct QuadVCAWidget : ModuleWidget 
{ 
    QuadVCAWidget(QuadVCA *module);
};


QuadVCAWidget::QuadVCAWidget(QuadVCA *module) : ModuleWidget(module) {

  setPanel(SVG::load(assetPlugin(plugin, "res/QuadVCA.svg")));
	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	static const float posX[4] = {13,39,65,91};
    //SLIDERS
	addParam(ParamWidget::create<as_SlidePot>(Vec(posX[0]-3, 70), module, QuadVCA::GAIN1_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_SlidePot>(Vec(posX[1]-3, 70), module, QuadVCA::GAIN2_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_SlidePot>(Vec(posX[2]-3, 70), module, QuadVCA::GAIN3_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_SlidePot>(Vec(posX[3]-3, 70), module, QuadVCA::GAIN4_PARAM, 0.0f, 1.0f, 0.5f));
    //MODE SWITCHES
    addParam(ParamWidget::create<as_CKSS>(Vec(posX[0], 190), module, QuadVCA::MODE1_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<as_CKSS>(Vec(posX[1], 190), module, QuadVCA::MODE2_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<as_CKSS>(Vec(posX[2], 190), module, QuadVCA::MODE3_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<as_CKSS>(Vec(posX[3], 190), module, QuadVCA::MODE4_PARAM, 0.0f, 1.0f, 1.0f));
	//CV INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(posX[0]-4, 217), Port::INPUT, module, QuadVCA::GAIN1_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(posX[1]-4, 217), Port::INPUT, module, QuadVCA::GAIN2_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(posX[2]-4, 217), Port::INPUT, module, QuadVCA::GAIN3_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(posX[3]-4, 217), Port::INPUT, module, QuadVCA::GAIN4_CV_INPUT));
	//INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(posX[0]-4, 260), Port::INPUT, module, QuadVCA::IN1_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(posX[1]-4, 260), Port::INPUT, module, QuadVCA::IN2_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(posX[2]-4, 260), Port::INPUT, module, QuadVCA::IN3_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(posX[3]-4, 260), Port::INPUT, module, QuadVCA::IN4_INPUT));
	//LEDS
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(posX[0]+5, 288), module, QuadVCA::GAIN1_LIGHT));//294
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(posX[1]+5, 288), module, QuadVCA::GAIN2_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(posX[2]+5, 288), module, QuadVCA::GAIN3_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(posX[3]+5, 288), module, QuadVCA::GAIN4_LIGHT));
	//OUTPUTS
	addOutput(Port::create<as_PJ301MPort>(Vec(posX[0]-4, 310), Port::OUTPUT, module, QuadVCA::OUT1_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(posX[1]-4, 310), Port::OUTPUT, module, QuadVCA::OUT2_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(posX[2]-4, 310), Port::OUTPUT, module, QuadVCA::OUT3_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(posX[3]-4, 310), Port::OUTPUT, module, QuadVCA::OUT4_OUTPUT));

}

RACK_PLUGIN_MODEL_INIT(AS, QuadVCA) {
   Model *modelQuadVCA = Model::create<QuadVCA, QuadVCAWidget>("AS", "QuadVCA", "Quad VCA/Mixer", AMPLIFIER_TAG, MIXER_TAG);
   return modelQuadVCA;
}
