#include "ML_modules.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct TrigSwitch3_2 : Module {
	enum ParamIds {
		STEP_PARAM,
		NUM_PARAMS = STEP_PARAM+8
	};

	enum InputIds {
		TRIG_INPUT,
		CV1_INPUT = TRIG_INPUT + 8,
		CV2_INPUT,
		CV3_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT = OUT1_OUTPUT + 8,
		OUT3_OUTPUT = OUT2_OUTPUT + 8,
		NUM_OUTPUTS = OUT3_OUTPUT + 8
	};
	enum LightIds {
		STEP_LIGHT,
		NUM_LIGHTS = STEP_LIGHT+8
	};

	TrigSwitch3_2() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };

	void step() override;

	int position=0;

	enum OutMode {
		ZERO,
		LAST
	};

	OutMode outMode = ZERO;



	SchmittTrigger stepTriggers[8];

	float out1[8];
	float out2[8];
	float out3[8];

	void reset() override {

		position = 0;
		for(int i=0; i<8; i++) {
			lights[i].value = 0.0;
			out1[i] = 0.0f;
			out2[i] = 0.0f;
			out3[i] = 0.0f;
		};
	};

	json_t *toJson() override {

		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "outMode", json_integer(outMode));
		json_object_set_new(rootJ, "position", json_integer(position));
	

		return rootJ;
	};
	
	void fromJson(json_t *rootJ) override {

		json_t *outModeJ = json_object_get(rootJ, "outMode");
		if(outModeJ) outMode = (OutMode) json_integer_value(outModeJ);
	
		json_t *positionJ = json_object_get(rootJ, "position");
		if(positionJ) position = json_integer_value(positionJ);

	};


};


void TrigSwitch3_2::step() {

	if(outMode==ZERO) { 
		for(int i=0; i<8; i++) {
			out1[i] = 0.0f;
			out2[i] = 0.0f;
			out3[i] = 0.0f;
		}
	}



	for(int i=0; i<8; i++) {
		if( stepTriggers[i].process( inputs[TRIG_INPUT+i].normalize(0.0)) + params[STEP_PARAM+i].value ) position = i;
		lights[i].value = (i==position)?1.0:0.0;
	};

	out1[position] = inputs[CV1_INPUT].normalize(0.0);
	out2[position] = inputs[CV2_INPUT].normalize(0.0);
	out3[position] = inputs[CV3_INPUT].normalize(0.0);

	for(int i=0; i<8; i++) {
		outputs[OUT1_OUTPUT+i].value = out1[i];
		outputs[OUT2_OUTPUT+i].value = out2[i];
		outputs[OUT3_OUTPUT+i].value = out3[i];
	}

};


struct TrigSwitch3_2OutModeItem : MenuItem {

	TrigSwitch3_2 *trigSwitch;
	TrigSwitch3_2::OutMode outMode;

	void onAction(EventAction &e) override {
		trigSwitch->outMode = outMode;
	};


	void step() override {
		rightText = (trigSwitch->outMode == outMode)? "✔" : "";
	};

};



struct TrigSwitch3_2Widget : ModuleWidget {
	TrigSwitch3_2Widget(TrigSwitch3_2 *module);
	json_t *toJsonData();
	void fromJsonData(json_t *root) ;
	Menu *createContextMenu() override;

};

TrigSwitch3_2Widget::TrigSwitch3_2Widget(TrigSwitch3_2 *module) : ModuleWidget(module) {

	box.size = Vec(15*12, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/TrigSwitch3_2.svg")));
		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));


	const float offset_y = 60, delta_y = 32, row1=15, row2 = row1+33, row3 = row2 + 25;

	for (int i=0; i<8; i++) {

		addInput(Port::create<MLPort>(             Vec(row1, offset_y + i*delta_y), Port::INPUT, module, TrigSwitch3_2::TRIG_INPUT + i));

		addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(row2 , offset_y + i*delta_y +3 ), module, TrigSwitch3_2::STEP_PARAM + i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>( Vec(row2 + 4, offset_y + i*delta_y + 7), module, TrigSwitch3_2::STEP_LIGHT+i));


		addOutput(Port::create<MLPort>(             Vec(row3, offset_y + i*delta_y),    Port::OUTPUT, module, TrigSwitch3_2::OUT1_OUTPUT + i));
		addOutput(Port::create<MLPort>(             Vec(row3+32, offset_y + i*delta_y), Port::OUTPUT, module, TrigSwitch3_2::OUT2_OUTPUT + i));
		addOutput(Port::create<MLPort>(             Vec(row3+64, offset_y + i*delta_y), Port::OUTPUT, module, TrigSwitch3_2::OUT3_OUTPUT + i));

	}
	addInput(Port::create<MLPort>(Vec(row3,    320), Port::INPUT, module, TrigSwitch3_2::CV1_INPUT));
	addInput(Port::create<MLPort>(Vec(row3+32, 320), Port::INPUT, module, TrigSwitch3_2::CV2_INPUT));
	addInput(Port::create<MLPort>(Vec(row3+64, 320), Port::INPUT, module, TrigSwitch3_2::CV3_INPUT));

}



Menu *TrigSwitch3_2Widget::createContextMenu() {

	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	TrigSwitch3_2 *trigSwitch = dynamic_cast<TrigSwitch3_2*>(module);
	assert(trigSwitch);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Output Mode";
	menu->addChild(modeLabel);

	TrigSwitch3_2OutModeItem *zeroItem = new TrigSwitch3_2OutModeItem();
	zeroItem->text = "Zero";	
	zeroItem->trigSwitch = trigSwitch;
	zeroItem->outMode = TrigSwitch3_2::ZERO;
	menu->addChild(zeroItem);

	TrigSwitch3_2OutModeItem *lastItem = new TrigSwitch3_2OutModeItem();
	lastItem->text = "Last";	
	lastItem->trigSwitch = trigSwitch;
	lastItem->outMode = TrigSwitch3_2::LAST;
	menu->addChild(lastItem);


	return menu;
};

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, TrigSwitch3_2) {
   Model *modelTrigSwitch3_2 = Model::create<TrigSwitch3_2, TrigSwitch3_2Widget>("ML modules", "TrigSwitch3_2", "TrigSwitch3 1->8", SWITCH_TAG, UTILITY_TAG );
   return modelTrigSwitch3_2;
}
