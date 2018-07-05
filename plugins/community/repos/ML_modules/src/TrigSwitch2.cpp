#include "ML_modules.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct TrigSwitch2 : Module {
	enum ParamIds {
		STEP_PARAM,
		NUM_PARAMS = STEP_PARAM + 9
	};
	enum InputIds {
		CV_INPUT,
		TRIG_INPUT,
		NUM_INPUTS = TRIG_INPUT + 8
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS = OUT_OUTPUT+8
	};
	enum LightIds {
		STEP_LIGHT,
		NUM_LIGHTS = STEP_LIGHT+8
	};


	TrigSwitch2() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };

	void step() override;

	enum OutMode {
		ZERO,
		LAST
	};

	OutMode outMode = ZERO;

	json_t *toJson() override {

		json_t *rootJ = json_object();

		// outMode:
	
		json_object_set_new(rootJ, "outMode", json_integer(outMode));
		json_object_set_new(rootJ, "position", json_integer(position));
	

		return rootJ;
	};
	
	void fromJson(json_t *rootJ) override {

		// outMode:

		json_t *outModeJ = json_object_get(rootJ, "outMode");
		if(outModeJ) outMode = (OutMode) json_integer_value(outModeJ);
	
		json_t *positionJ = json_object_get(rootJ, "position");
		if(positionJ) position = json_integer_value(positionJ);

	};


	int position=0;


	float outs[8] = {};


	SchmittTrigger stepTriggers[8];


	void reset() override {

		position = 0;
		for(int i=0; i<8; i++) lights[i].value = 0.0;
		for(int i=0; i<8; i++) outs[i]=0.0;
	};

};


void TrigSwitch2::step() {


	if(outMode==ZERO) { for(int i=0; i<8; i++) outs[i]=0.0; }

	for(int i=0; i<8; i++) {
		if( stepTriggers[i].process( inputs[TRIG_INPUT+i].normalize(0.0)) + params[STEP_PARAM+i].value ) position = i;

	};

	outs[position] = inputs[CV_INPUT].normalize(0.0);

	for(int i=0; i<8; i++) lights[i].value = (i==position)?1.0:0.0;

	for(int i=0; i<8; i++) outputs[OUT_OUTPUT+i].value = outs[i];
	
};



struct TrigSwitch2OutModeItem : MenuItem {

	TrigSwitch2 *trigSwitch2;
	TrigSwitch2::OutMode outMode;

	void onAction(EventAction &e) override {
		trigSwitch2->outMode = outMode;
	};


	void step() override {
		rightText = (trigSwitch2->outMode == outMode)? "✔" : "";
	};

};


struct TrigSwitch2Widget : ModuleWidget {
	TrigSwitch2Widget(TrigSwitch2 *module);
	json_t *toJsonData() ;
	void fromJsonData(json_t *root) ;
	Menu *createContextMenu() override;
};

TrigSwitch2Widget::TrigSwitch2Widget(TrigSwitch2 *module) : ModuleWidget(module) {

	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/TrigSwitch2.svg")));
		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));


	const float offset_y = 60, delta_y = 32, row1=14, row2 = 50, row3 = 79;

	for (int i=0; i<8; i++) {

		addInput(Port::create<MLPort>(             Vec(row1, offset_y + i*delta_y), Port::INPUT, module, TrigSwitch2::TRIG_INPUT + i));

		addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(row2 , offset_y + i*delta_y +3 ), module, TrigSwitch2::STEP_PARAM + i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>( Vec(row2 + 4, offset_y + i*delta_y + 7), module, TrigSwitch2::STEP_LIGHT+i));
		
		addOutput(Port::create<MLPort>(           Vec(row3, offset_y + i*delta_y), Port::OUTPUT, module, TrigSwitch2::OUT_OUTPUT + i));

	}
	addInput(Port::create<MLPort>(Vec(row3, 320), Port::INPUT, module, TrigSwitch2::CV_INPUT));

}

Menu *TrigSwitch2Widget::createContextMenu() {

	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	TrigSwitch2 *trigSwitch2 = dynamic_cast<TrigSwitch2*>(module);
	assert(trigSwitch2);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Output Mode";
	menu->addChild(modeLabel);

	TrigSwitch2OutModeItem *zeroItem = new TrigSwitch2OutModeItem();
	zeroItem->text = "Zero";	
	zeroItem->trigSwitch2 = trigSwitch2;
	zeroItem->outMode = TrigSwitch2::ZERO;
	menu->addChild(zeroItem);

	TrigSwitch2OutModeItem *lastItem = new TrigSwitch2OutModeItem();
	lastItem->text = "Last";	
	lastItem->trigSwitch2 = trigSwitch2;
	lastItem->outMode = TrigSwitch2::LAST;
	menu->addChild(lastItem);


	return menu;
};

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, TrigSwitch2) {
   Model *modelTrigSwitch2 = Model::create<TrigSwitch2, TrigSwitch2Widget>("ML modules", "TrigSwitch2", "TrigSwitch 1->8", SWITCH_TAG, UTILITY_TAG );
   return modelTrigSwitch2;
}
