//***********************************************************************************************
//
//MonoVUmeter module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//  
//***********************************************************************************************

#include "AS.hpp"
#include "dsp/vumeter.hpp"

struct MonoVUmeter : Module {
	enum ParamIds {
	 	NUM_PARAMS
	};

	enum InputIds {
		INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT,
	 	NUM_OUTPUTS
	};
	
	enum LightIds {
		METER_LIGHT,
		NUM_LIGHTS = METER_LIGHT+15
	};

	MonoVUmeter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

	VUMeter vuBar;

};

void MonoVUmeter::step(){
	//GET VALUES AND ROUTE SIGNAL TO OUTPUT
	float signal_in = inputs[INPUT].value;
	outputs[OUT].value = signal_in;
	//VU METER BARS LIGHTS
	vuBar.dBInterval = 3;
	vuBar.setValue(signal_in / 10.0f);
	for (int i = 0; i < 15; i++){
		lights[METER_LIGHT + i].setBrightnessSmooth(vuBar.getBrightness(i));
	}
};

struct MonoVUmeterWidget : ModuleWidget 
{ 
    MonoVUmeterWidget(MonoVUmeter *module);
};


MonoVUmeterWidget::MonoVUmeterWidget(MonoVUmeter *module) : ModuleWidget(module) {

  setPanel(SVG::load(assetPlugin(plugin, "res/MonoVUmeter.svg")));
  
	//SCREWS - SPECIAL SPACING FOR RACK WIDTH*4
	addChild(Widget::create<as_HexScrew>(Vec(0, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	// LEFT COLUMN LEDs
	static const float ledCol = 11;
	static const float offsetY = 12;
	static const float startY = 66;
		addChild(ModuleLightWidget::create<MeterLight<RedLight>>(Vec(ledCol, startY + offsetY * 0), module, MonoVUmeter::METER_LIGHT + 0));
		addChild(ModuleLightWidget::create<MeterLight<RedLight>>(Vec(ledCol, startY + offsetY * 1), module, MonoVUmeter::METER_LIGHT + 1));
		addChild(ModuleLightWidget::create<MeterLight<RedLight>>(Vec(ledCol, startY + offsetY * 2), module, MonoVUmeter::METER_LIGHT + 2));
	addChild(ModuleLightWidget::create<MeterLight<OrangeLight>>(Vec(ledCol, startY + offsetY * 3), module, MonoVUmeter::METER_LIGHT + 3));
	addChild(ModuleLightWidget::create<MeterLight<OrangeLight>>(Vec(ledCol, startY + offsetY * 4), module, MonoVUmeter::METER_LIGHT + 4));
	addChild(ModuleLightWidget::create<MeterLight<OrangeLight>>(Vec(ledCol, startY + offsetY * 5), module, MonoVUmeter::METER_LIGHT + 5));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(ledCol, startY + offsetY * 6), module, MonoVUmeter::METER_LIGHT + 6));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(ledCol, startY + offsetY * 7), module, MonoVUmeter::METER_LIGHT + 7));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(ledCol, startY + offsetY * 8), module, MonoVUmeter::METER_LIGHT + 8));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(ledCol, startY + offsetY * 9), module, MonoVUmeter::METER_LIGHT + 9));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(ledCol, startY + offsetY * 10), module, MonoVUmeter::METER_LIGHT + 10));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(ledCol, startY + offsetY * 11), module, MonoVUmeter::METER_LIGHT + 11));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(ledCol, startY + offsetY * 12), module, MonoVUmeter::METER_LIGHT + 12));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(ledCol, startY + offsetY * 13), module, MonoVUmeter::METER_LIGHT + 13));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(ledCol, startY + offsetY * 14), module, MonoVUmeter::METER_LIGHT + 14));
	
	//INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(3, 270), Port::INPUT, module, MonoVUmeter::INPUT));
	//OUTPUTS
	addOutput(Port::create<as_PJ301MPort>(Vec(3,310), Port::OUTPUT, module, MonoVUmeter::OUT));
}

RACK_PLUGIN_MODEL_INIT(AS, MonoVUmeter) {
   Model *modelMonoVUmeter = Model::create<MonoVUmeter, MonoVUmeterWidget>("AS", "MonoVUmeter", "Mono VU meter", VISUAL_TAG, UTILITY_TAG);
   return modelMonoVUmeter;
}
