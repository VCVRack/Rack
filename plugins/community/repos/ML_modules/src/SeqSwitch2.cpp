#include "ML_modules.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct SeqSwitch2 : Module {
	enum ParamIds {
		NUM_STEPS,
		STEP1_PARAM,
		STEP2_PARAM,
		STEP3_PARAM,
		STEP4_PARAM,
		STEP5_PARAM,
		STEP6_PARAM,
		STEP7_PARAM,
		STEP8_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		TRIGUP_INPUT,
		TRIGDN_INPUT,
		RESET_INPUT,
		NUMSTEPS_INPUT,
		POS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
		OUT5_OUTPUT,
		OUT6_OUTPUT,
		OUT7_OUTPUT,
		OUT8_OUTPUT,
		NUM_OUTPUTS
	};
        enum LightIds {
                STEP1_LIGHT,
                STEP2_LIGHT,
                STEP3_LIGHT,
                STEP4_LIGHT,
                STEP5_LIGHT,
                STEP6_LIGHT,
                STEP7_LIGHT,
                STEP8_LIGHT,
                NUM_LIGHTS
        };

	SeqSwitch2() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };


	void step() override;

	int position=0;

	float outs[8] = {};

	const float in_min[4] = {0.0, 0.0, 0.0, -5.0};
	const float in_max[4] = {8.0, 6.0, 10.0, 5.0};

	SchmittTrigger upTrigger, downTrigger, resetTrigger, stepTriggers[8];

	void reset() override {
		position=0;
		for(int i=0; i<8; i++) lights[i].value = 0.0;
	};

	enum OutMode {
		ZERO,
		LAST
	};

	OutMode outMode = ZERO;

	enum InputRange {
		Zero_Eight,
		Zero_Six,
		Zero_Ten,
		MinusFive_Five
  	};

	InputRange inputRange = Zero_Eight;

	json_t *toJson() override {

		json_t *rootJ = json_object();

		// outMode:

		json_object_set_new(rootJ, "outMode", json_integer(outMode));
		json_object_set_new(rootJ, "inputRange", json_integer(inputRange));

		return rootJ;
	};

	void fromJson(json_t *rootJ) override {

		// outMode:

		json_t *outModeJ = json_object_get(rootJ, "outMode");
		if(outModeJ) outMode = (OutMode) json_integer_value(outModeJ);

		json_t *inputRangeJ = json_object_get(rootJ, "inputRange");
		if(inputRangeJ) inputRange = (InputRange) json_integer_value(inputRangeJ);
	};


};



void SeqSwitch2::step() {

	if(outMode==ZERO) { for(int i=0; i<8; i++) outs[i]=0.0f; }

	int numSteps = round(clamp(params[NUM_STEPS].value,1.0f,8.0f));
	if( inputs[NUMSTEPS_INPUT].active ) numSteps = round(clamp(inputs[NUMSTEPS_INPUT].value,1.0f,8.0f));

	if( inputs[POS_INPUT].active ) {

//		position = round( clamp( inputs[POS_INPUT].value,0.0,8.0))/8.0 * (numSteps-1) ;

		float in_value = clamp( inputs[POS_INPUT].value,in_min[inputRange],in_max[inputRange] );

		position = round( rescale( in_value, in_min[inputRange], in_max[inputRange], 0.0f, 1.0f*(numSteps-1) ) );

	} else {

		if( inputs[TRIGUP_INPUT].active ) {
			if (upTrigger.process(inputs[TRIGUP_INPUT].value) ) position++;
		}

		if( inputs[TRIGDN_INPUT].active ) {
			if (downTrigger.process(inputs[TRIGDN_INPUT].value) ) position--;
		}

		if( inputs[RESET_INPUT].active ) {
			if (resetTrigger.process(inputs[RESET_INPUT].value) ) position = 0;
		}

	};


	for(int i=0; i<numSteps; i++) {
		if( stepTriggers[i].process(params[STEP1_PARAM+i].value)) position = i;
	};

	while( position < 0 )         position += numSteps;
	while( position >= numSteps ) position -= numSteps;

	outs[position] = inputs[IN_INPUT].normalize(0.0);


	for(int i=0; i<8; i++) lights[i].value = (i==position)?1.0:0.0;

	for(int i=0; i<8; i++) outputs[OUT1_OUTPUT+i].value = outs[i];
};



struct SeqSwitch2Widget : ModuleWidget {
	SeqSwitch2Widget(SeqSwitch2 *module);
	json_t *toJsonData() ;
	void fromJsonData(json_t *root) ;
	Menu *createContextMenu() override;
};

SeqSwitch2Widget::SeqSwitch2Widget(SeqSwitch2 *module) : ModuleWidget(module) {

	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/SeqSwitch2.svg")));
		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));


	addParam(ParamWidget::create<RedSnapMLKnob>(Vec(14,  63), module, SeqSwitch2::NUM_STEPS, 1.0, 8.0, 8.0));

	addInput(Port::create<MLPort>(Vec(81, 64), Port::INPUT, module, SeqSwitch2::NUMSTEPS_INPUT));

	addInput(Port::create<MLPort>(Vec(9, 272), Port::INPUT, module, SeqSwitch2::TRIGUP_INPUT));
	addInput(Port::create<MLPort>(Vec(47, 272), Port::INPUT, module, SeqSwitch2::RESET_INPUT));
	addInput(Port::create<MLPort>(Vec(85, 272), Port::INPUT, module, SeqSwitch2::TRIGDN_INPUT));

	const float offset_y = 118, delta_y=38;

	addOutput(Port::create<MLPort>(Vec(32, offset_y + 0*delta_y), Port::OUTPUT, module, SeqSwitch2::OUT1_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(32, offset_y + 1*delta_y), Port::OUTPUT, module, SeqSwitch2::OUT2_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(32, offset_y + 2*delta_y), Port::OUTPUT, module, SeqSwitch2::OUT3_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(32, offset_y + 3*delta_y), Port::OUTPUT, module, SeqSwitch2::OUT4_OUTPUT));

	addOutput(Port::create<MLPort>(Vec(62, offset_y + 0*delta_y), Port::OUTPUT, module, SeqSwitch2::OUT5_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(62, offset_y + 1*delta_y), Port::OUTPUT, module, SeqSwitch2::OUT6_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(62, offset_y + 2*delta_y), Port::OUTPUT, module, SeqSwitch2::OUT7_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(62, offset_y + 3*delta_y), Port::OUTPUT, module, SeqSwitch2::OUT8_OUTPUT));

	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(11, offset_y + 3 + 0*delta_y), module, SeqSwitch2::STEP1_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(11, offset_y + 3 + 1*delta_y), module, SeqSwitch2::STEP2_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(11, offset_y + 3 + 2*delta_y), module, SeqSwitch2::STEP3_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(11, offset_y + 3 + 3*delta_y), module, SeqSwitch2::STEP4_PARAM, 0.0, 1.0, 0.0));

	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(89, offset_y + 3 + 0*delta_y), module, SeqSwitch2::STEP5_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(89, offset_y + 3 + 1*delta_y), module, SeqSwitch2::STEP6_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(89, offset_y + 3 + 2*delta_y), module, SeqSwitch2::STEP7_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(89, offset_y + 3 + 3*delta_y), module, SeqSwitch2::STEP8_PARAM, 0.0, 1.0, 0.0));


    addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(15, offset_y + 7 + 0*delta_y), module, SeqSwitch2::STEP1_LIGHT));
    addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(15, offset_y + 7 + 1*delta_y), module, SeqSwitch2::STEP2_LIGHT));
    addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(15, offset_y + 7 + 2*delta_y), module, SeqSwitch2::STEP3_LIGHT));
    addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(15, offset_y + 7 + 3*delta_y), module, SeqSwitch2::STEP4_LIGHT));

    addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(93, offset_y + 7 + 0*delta_y), module, SeqSwitch2::STEP5_LIGHT));
    addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(93, offset_y + 7 + 1*delta_y), module, SeqSwitch2::STEP6_LIGHT));
    addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(93, offset_y + 7 + 2*delta_y), module, SeqSwitch2::STEP7_LIGHT));
    addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(93, offset_y + 7 + 3*delta_y), module, SeqSwitch2::STEP8_LIGHT));

	addInput(Port::create<MLPort>(Vec(19, 318), Port::INPUT, module, SeqSwitch2::POS_INPUT));
	addInput(Port::create<MLPort>(Vec(75, 318), Port::INPUT, module, SeqSwitch2::IN_INPUT));

};

struct SeqSwitch2OutModeItem : MenuItem {

	SeqSwitch2 *seqSwitch2;
	SeqSwitch2::OutMode outMode;

	void onAction(EventAction &e) override {
		seqSwitch2->outMode = outMode;
	};


	void step() override {
		rightText = (seqSwitch2->outMode == outMode)? "✔" : "";
	};

};

struct SeqSwitch2RangeItem : MenuItem {

	SeqSwitch2 *seqSwitch2;
	SeqSwitch2::InputRange inputRange;


	void onAction(EventAction &e) override {
		seqSwitch2->inputRange = inputRange;
	};


	void step() override {
		rightText = (seqSwitch2->inputRange == inputRange)? "✔" : "";
	};

};

Menu *SeqSwitch2Widget::createContextMenu() {

	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	SeqSwitch2 *seqSwitch2 = dynamic_cast<SeqSwitch2*>(module);
	assert(seqSwitch2);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Output Mode";
	menu->addChild(modeLabel);

	SeqSwitch2OutModeItem *zeroItem = new SeqSwitch2OutModeItem();
	zeroItem->text = "Zero";
	zeroItem->seqSwitch2 = seqSwitch2;
	zeroItem->outMode = SeqSwitch2::ZERO;
	menu->addChild(zeroItem);

	SeqSwitch2OutModeItem *lastItem = new SeqSwitch2OutModeItem();
	lastItem->text = "Last";
	lastItem->seqSwitch2 = seqSwitch2;
	lastItem->outMode = SeqSwitch2::LAST;
	menu->addChild(lastItem);

	MenuLabel *modeLabel2 = new MenuLabel();
	modeLabel2->text = "Input Range";
	menu->addChild(modeLabel2);

	SeqSwitch2RangeItem *zeroEightItem = new SeqSwitch2RangeItem();
	zeroEightItem->text = "0 - 8V";
	zeroEightItem->seqSwitch2 = seqSwitch2;
	zeroEightItem->inputRange = SeqSwitch2::Zero_Eight;
	menu->addChild(zeroEightItem);

	SeqSwitch2RangeItem *zeroSixItem = new SeqSwitch2RangeItem();
	zeroSixItem->text = "0 - 6V";
	zeroSixItem->seqSwitch2 = seqSwitch2;
	zeroSixItem->inputRange = SeqSwitch2::Zero_Six;
	menu->addChild(zeroSixItem);

	SeqSwitch2RangeItem *zeroTenItem = new SeqSwitch2RangeItem();
	zeroTenItem->text = "0 - 10V";
	zeroTenItem->seqSwitch2 = seqSwitch2;
	zeroTenItem->inputRange = SeqSwitch2::Zero_Ten;
	menu->addChild(zeroTenItem);

	SeqSwitch2RangeItem *fiveFiveItem = new SeqSwitch2RangeItem();
	fiveFiveItem->text = "-5 - 5V";
	fiveFiveItem->seqSwitch2 = seqSwitch2;
	fiveFiveItem->inputRange = SeqSwitch2::MinusFive_Five;;
	menu->addChild(fiveFiveItem);

	return menu;
};

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, SeqSwitch2) {
   Model *modelSeqSwitch2 = Model::create<SeqSwitch2, SeqSwitch2Widget>("ML modules", "SeqSwitch2", "Sequential Switch 1->8", SWITCH_TAG);
   return modelSeqSwitch2;
}
