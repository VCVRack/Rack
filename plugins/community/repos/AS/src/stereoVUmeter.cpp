/////////////////////////////////////////////////////
// Stereo VU Meter module by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
// StereoVUmeter
//
/////////////////////////////////////////////////////

#include "AS.hpp"
#include "dsp/vumeter.hpp"

struct StereoVUmeter : Module {
	enum ParamIds {
	 	NUM_PARAMS
	};

	enum InputIds {
		INPUT_LEFT,
		INPUT_RIGHT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT_LEFT,
		OUT_RIGHT,
	 	NUM_OUTPUTS
	};
	
	enum LightIds {
		METER_LIGHT_LEFT,
		METER_LIGHT_RIGHT = METER_LIGHT_LEFT+15,
		NUM_LIGHTS = METER_LIGHT_RIGHT + 15
	};

	StereoVUmeter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

	VUMeter vuBar;

};

void StereoVUmeter::step(){
	//GET VALUES AND ROUTE SIGNALS TO OUTPUTS
	float signal_in_Left = inputs[INPUT_LEFT].value;
	float signal_in_Right = inputs[INPUT_RIGHT].value;
	outputs[OUT_LEFT].value = signal_in_Left;
	outputs[OUT_RIGHT].value = signal_in_Right;
	//VU METER BARS LIGHTS
	vuBar.dBInterval = 3;
	for (int i = 0; i < 15; i++){
		vuBar.setValue(signal_in_Left / 10.0f);
		lights[METER_LIGHT_LEFT + i].setBrightnessSmooth(vuBar.getBrightness(i));
		vuBar.setValue(signal_in_Right / 10.0f);
		lights[METER_LIGHT_RIGHT + i].setBrightnessSmooth(vuBar.getBrightness(i));
	}
};

struct StereoVUmeterWidget : ModuleWidget 
{ 
    StereoVUmeterWidget(StereoVUmeter *module);
};


StereoVUmeterWidget::StereoVUmeterWidget(StereoVUmeter *module) : ModuleWidget(module) {

   setPanel(SVG::load(assetPlugin(plugin, "res/StereoVUmeter.svg")));
   
	//SCREWS - SPECIAL SPACING FOR RACK WIDTH*4
	addChild(Widget::create<as_HexScrew>(Vec(0, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	// LEFT COLUMN LEDs
	static const float leftCol = 15;
	static const float offsetY = 12;
	static const float startY = 66;
		addChild(ModuleLightWidget::create<MeterLight<RedLight>>(Vec(leftCol, startY + offsetY * 0), module, StereoVUmeter::METER_LIGHT_LEFT + 0));
		addChild(ModuleLightWidget::create<MeterLight<RedLight>>(Vec(leftCol, startY + offsetY * 1), module, StereoVUmeter::METER_LIGHT_LEFT + 1));
		addChild(ModuleLightWidget::create<MeterLight<RedLight>>(Vec(leftCol, startY + offsetY * 2), module, StereoVUmeter::METER_LIGHT_LEFT + 2));
	addChild(ModuleLightWidget::create<MeterLight<OrangeLight>>(Vec(leftCol, startY + offsetY * 3), module, StereoVUmeter::METER_LIGHT_LEFT + 3));
	addChild(ModuleLightWidget::create<MeterLight<OrangeLight>>(Vec(leftCol, startY + offsetY * 4), module, StereoVUmeter::METER_LIGHT_LEFT + 4));
	addChild(ModuleLightWidget::create<MeterLight<OrangeLight>>(Vec(leftCol, startY + offsetY * 5), module, StereoVUmeter::METER_LIGHT_LEFT + 5));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(leftCol, startY + offsetY * 6), module, StereoVUmeter::METER_LIGHT_LEFT + 6));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(leftCol, startY + offsetY * 7), module, StereoVUmeter::METER_LIGHT_LEFT + 7));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(leftCol, startY + offsetY * 8), module, StereoVUmeter::METER_LIGHT_LEFT + 8));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(leftCol, startY + offsetY * 9), module, StereoVUmeter::METER_LIGHT_LEFT + 9));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(leftCol, startY + offsetY * 10), module, StereoVUmeter::METER_LIGHT_LEFT + 10));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(leftCol, startY + offsetY * 11), module, StereoVUmeter::METER_LIGHT_LEFT + 11));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(leftCol, startY + offsetY * 12), module, StereoVUmeter::METER_LIGHT_LEFT + 12));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(leftCol, startY + offsetY * 13), module, StereoVUmeter::METER_LIGHT_LEFT + 13));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(leftCol, startY + offsetY * 14), module, StereoVUmeter::METER_LIGHT_LEFT + 14));
	//RIGHT COLUMN LEDs
	static const float rightCol = 37;
	
		addChild(ModuleLightWidget::create<MeterLight<RedLight>>(Vec(rightCol, startY + offsetY * 0), module, StereoVUmeter::METER_LIGHT_RIGHT + 0));
		addChild(ModuleLightWidget::create<MeterLight<RedLight>>(Vec(rightCol, startY + offsetY * 1), module, StereoVUmeter::METER_LIGHT_RIGHT + 1));
		addChild(ModuleLightWidget::create<MeterLight<RedLight>>(Vec(rightCol, startY + offsetY * 2), module, StereoVUmeter::METER_LIGHT_RIGHT + 2));
	addChild(ModuleLightWidget::create<MeterLight<OrangeLight>>(Vec(rightCol, startY + offsetY * 3), module, StereoVUmeter::METER_LIGHT_RIGHT + 3));
	addChild(ModuleLightWidget::create<MeterLight<OrangeLight>>(Vec(rightCol, startY + offsetY * 4), module, StereoVUmeter::METER_LIGHT_RIGHT + 4));
	addChild(ModuleLightWidget::create<MeterLight<OrangeLight>>(Vec(rightCol, startY + offsetY * 5), module, StereoVUmeter::METER_LIGHT_RIGHT + 5));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(rightCol, startY + offsetY * 6), module, StereoVUmeter::METER_LIGHT_RIGHT + 6));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(rightCol, startY + offsetY * 7), module, StereoVUmeter::METER_LIGHT_RIGHT + 7));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(rightCol, startY + offsetY * 8), module, StereoVUmeter::METER_LIGHT_RIGHT + 8));
	addChild(ModuleLightWidget::create<MeterLight<YellowLight>>(Vec(rightCol, startY + offsetY * 9), module, StereoVUmeter::METER_LIGHT_RIGHT + 9));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(rightCol, startY + offsetY * 10), module, StereoVUmeter::METER_LIGHT_RIGHT + 10));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(rightCol, startY + offsetY * 11), module, StereoVUmeter::METER_LIGHT_RIGHT + 11));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(rightCol, startY + offsetY * 12), module, StereoVUmeter::METER_LIGHT_RIGHT + 12));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(rightCol, startY + offsetY * 13), module, StereoVUmeter::METER_LIGHT_RIGHT + 13));
	addChild(ModuleLightWidget::create<MeterLight<GreenLight>>(Vec(rightCol, startY + offsetY * 14), module, StereoVUmeter::METER_LIGHT_RIGHT + 14));
	
	//INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(3, 270), Port::INPUT, module, StereoVUmeter::INPUT_LEFT));
	addInput(Port::create<as_PJ301MPort>(Vec(3, 310), Port::INPUT, module, StereoVUmeter::INPUT_RIGHT));
	//OUTPUTS
	addOutput(Port::create<as_PJ301MPort>(Vec(33,270), Port::OUTPUT, module, StereoVUmeter::OUT_LEFT));
	addOutput(Port::create<as_PJ301MPort>(Vec(33,310), Port::OUTPUT, module, StereoVUmeter::OUT_RIGHT));
}

RACK_PLUGIN_MODEL_INIT(AS, StereoVUmeter) {
   Model *modelStereoVUmeter = Model::create<StereoVUmeter, StereoVUmeterWidget>("AS", "StereoVUmeter", "Stereo VU meter", VISUAL_TAG, UTILITY_TAG);
   return modelStereoVUmeter;
}
