#include "ML_modules.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct SeqSwitch : Module {
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
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		IN4_INPUT,
		IN5_INPUT,
		IN6_INPUT,
		IN7_INPUT,
		IN8_INPUT,
		POS_INPUT,
		TRIGUP_INPUT,
		TRIGDN_INPUT,
		RESET_INPUT,
		NUMSTEPS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
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

	SeqSwitch() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };

	void step() override;

	int position=0;



        const float in_min[4] = {0.0, 0.0, 0.0, -5.0};
        const float in_max[4] = {8.0, 6.0, 10.0, 5.0};


	SchmittTrigger upTrigger, downTrigger, resetTrigger, stepTriggers[8];

	enum InputRange {
                Zero_Eight,
		Zero_Six,
                Zero_Ten,
                MinusFive_Five
        };


        json_t *toJson() override {

                json_t *rootJ = json_object();

                // outMode:

                json_object_set_new(rootJ, "inputRange", json_integer(inputRange));

                return rootJ;
        };

        void fromJson(json_t *rootJ) override {

                // outMode:

                json_t *inputRangeJ = json_object_get(rootJ, "inputRange");
                if(inputRangeJ) inputRange = (InputRange) json_integer_value(inputRangeJ);
        };


        InputRange inputRange = Zero_Eight;

	void reset() override {

		position = 0;
		for(int i=0; i<8; i++) lights[i].value = 0.0;
	};

};


void SeqSwitch::step() {

	float out=0.0;

	int numSteps = round(clamp(params[NUM_STEPS].value,1.0f,8.0f));
	if( inputs[NUMSTEPS_INPUT].active ) numSteps = round(clamp(inputs[NUMSTEPS_INPUT].value,1.0f,8.0f));

	if( inputs[POS_INPUT].active ) {

//		position = round( clamp( inputs[POS_INPUT].value,0.0f,8.0f))/8.0f * (numSteps-1) ;

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

	out = inputs[IN1_INPUT+position].normalize(0.0);

	for(int i=0; i<8; i++) lights[i].value = (i==position)?1.0:0.0;


	outputs[OUT1_OUTPUT].value = out;
};

struct SeqSwitchRangeItem : MenuItem {

        SeqSwitch *seqSwitch;
        SeqSwitch::InputRange inputRange;

        void onAction(EventAction &e) override {
                seqSwitch->inputRange = inputRange;
        };

        void step() override {
                rightText = (seqSwitch->inputRange == inputRange)? "✔" : "";
        };

};


struct SeqSwitchWidget : ModuleWidget {
	SeqSwitchWidget(SeqSwitch *module);
	json_t *toJsonData() ;
	void fromJsonData(json_t *root) ;
	Menu *createContextMenu() override;
};

Menu *SeqSwitchWidget::createContextMenu() {

        Menu *menu = ModuleWidget::createContextMenu();

        MenuLabel *spacerLabel = new MenuLabel();
        menu->addChild(spacerLabel);

        SeqSwitch *seqSwitch = dynamic_cast<SeqSwitch*>(module);
        assert(seqSwitch);

        MenuLabel *modeLabel2 = new MenuLabel();
        modeLabel2->text = "Input Range";
        menu->addChild(modeLabel2);

        SeqSwitchRangeItem *zeroEightItem = new SeqSwitchRangeItem();
        zeroEightItem->text = "0 - 8V";
        zeroEightItem->seqSwitch = seqSwitch;
        zeroEightItem->inputRange = SeqSwitch::Zero_Eight;
        menu->addChild(zeroEightItem);

        SeqSwitchRangeItem *zeroSixItem = new SeqSwitchRangeItem();
        zeroSixItem->text = "0 - 6V";
        zeroSixItem->seqSwitch = seqSwitch;
        zeroSixItem->inputRange = SeqSwitch::Zero_Six;
        menu->addChild(zeroSixItem);

        SeqSwitchRangeItem *zeroTenItem = new SeqSwitchRangeItem();
        zeroTenItem->text = "0 - 10V";
        zeroTenItem->seqSwitch = seqSwitch;
        zeroTenItem->inputRange = SeqSwitch::Zero_Ten;
        menu->addChild(zeroTenItem);

        SeqSwitchRangeItem *fiveFiveItem = new SeqSwitchRangeItem();
        fiveFiveItem->text = "-5 - 5V";
        fiveFiveItem->seqSwitch = seqSwitch;
        fiveFiveItem->inputRange = SeqSwitch::MinusFive_Five;;
        menu->addChild(fiveFiveItem);

        return menu;
};


SeqSwitchWidget::SeqSwitchWidget(SeqSwitch *module) : ModuleWidget(module) {

	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/SeqSwitch.svg")));
		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<RedSnapMLKnob>(Vec(14,  63), module, SeqSwitch::NUM_STEPS, 1.0, 8.0, 8.0));

	addInput(Port::create<MLPort>(Vec(81, 64), Port::INPUT, module, SeqSwitch::NUMSTEPS_INPUT));

	addInput(Port::create<MLPort>(Vec(9, 272),  Port::INPUT, module, SeqSwitch::TRIGUP_INPUT));
	addInput(Port::create<MLPort>(Vec(47, 272), Port::INPUT, module, SeqSwitch::RESET_INPUT));
	addInput(Port::create<MLPort>(Vec(85, 272), Port::INPUT, module, SeqSwitch::TRIGDN_INPUT));

	const float offset_y = 118, delta_y=38;

	addInput(Port::create<MLPort>(Vec(32, offset_y + 0*delta_y), Port::INPUT, module, SeqSwitch::IN1_INPUT));
	addInput(Port::create<MLPort>(Vec(32, offset_y + 1*delta_y), Port::INPUT, module, SeqSwitch::IN2_INPUT));
	addInput(Port::create<MLPort>(Vec(32, offset_y + 2*delta_y), Port::INPUT, module, SeqSwitch::IN3_INPUT));
	addInput(Port::create<MLPort>(Vec(32, offset_y + 3*delta_y), Port::INPUT, module, SeqSwitch::IN4_INPUT));

	addInput(Port::create<MLPort>(Vec(62, offset_y + 0*delta_y), Port::INPUT, module, SeqSwitch::IN5_INPUT));
	addInput(Port::create<MLPort>(Vec(62, offset_y + 1*delta_y), Port::INPUT, module, SeqSwitch::IN6_INPUT));
	addInput(Port::create<MLPort>(Vec(62, offset_y + 2*delta_y), Port::INPUT, module, SeqSwitch::IN7_INPUT));
	addInput(Port::create<MLPort>(Vec(62, offset_y + 3*delta_y), Port::INPUT, module, SeqSwitch::IN8_INPUT));

	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(11, offset_y + 3 + 0*delta_y), module, SeqSwitch::STEP1_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(11, offset_y + 3 + 1*delta_y), module, SeqSwitch::STEP2_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(11, offset_y + 3 + 2*delta_y), module, SeqSwitch::STEP3_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(11, offset_y + 3 + 3*delta_y), module, SeqSwitch::STEP4_PARAM, 0.0, 1.0, 0.0));

	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(89, offset_y + 3 + 0*delta_y), module, SeqSwitch::STEP5_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(89, offset_y + 3 + 1*delta_y), module, SeqSwitch::STEP6_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(89, offset_y + 3 + 2*delta_y), module, SeqSwitch::STEP7_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(89, offset_y + 3 + 3*delta_y), module, SeqSwitch::STEP8_PARAM, 0.0, 1.0, 0.0));

	addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(15, offset_y + 7 + 0*delta_y), module, SeqSwitch::STEP1_LIGHT));
	addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(15, offset_y + 7 + 1*delta_y), module, SeqSwitch::STEP2_LIGHT));
	addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(15, offset_y + 7 + 2*delta_y), module, SeqSwitch::STEP3_LIGHT));
	addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(15, offset_y + 7 + 3*delta_y), module, SeqSwitch::STEP4_LIGHT));

	addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(93, offset_y + 7 + 0*delta_y), module, SeqSwitch::STEP5_LIGHT));
	addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(93, offset_y + 7 + 1*delta_y), module, SeqSwitch::STEP6_LIGHT));
	addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(93, offset_y + 7 + 2*delta_y), module, SeqSwitch::STEP7_LIGHT));
	addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>(Vec(93, offset_y + 7 + 3*delta_y), module, SeqSwitch::STEP8_LIGHT));

	addInput(Port::create<MLPort>( Vec(19, 318),  Port::INPUT, module, SeqSwitch::POS_INPUT));
	addOutput(Port::create<MLPort>(Vec(75, 318), Port::OUTPUT, module, SeqSwitch::OUT1_OUTPUT));

}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, SeqSwitch) {
   Model *modelSeqSwitch = Model::create<SeqSwitch, SeqSwitchWidget>("ML modules", "SeqSwitch", "Sequential Switch 8->1",SWITCH_TAG, SEQUENCER_TAG);
   return modelSeqSwitch;
}
