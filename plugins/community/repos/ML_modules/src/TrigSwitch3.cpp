#include "ML_modules.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct TrigSwitch3 : Module {
	enum ParamIds {
		STEP_PARAM,
		NUM_PARAMS = STEP_PARAM+8
	};

	enum InputIds {
		TRIG_INPUT,
		CV_INPUT1 = TRIG_INPUT + 8,
		CV_INPUT2 = CV_INPUT1 + 8,
		CV_INPUT3 = CV_INPUT2 + 8,
		NUM_INPUTS = CV_INPUT3 + 8
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		STEP_LIGHT,
		NUM_LIGHTS = STEP_LIGHT+8
	};

	TrigSwitch3() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };

	void step() override;

	int position=0;



        const float in_min[4] = {0.0, 0.0, 0.0, -5.0};
        const float in_max[4] = {8.0, 6.0, 10.0, 5.0};


	SchmittTrigger stepTriggers[8];


	void reset() override {

		position = 0;
		for(int i=0; i<8; i++) lights[i].value = 0.0;
	};

	json_t *toJson() override {

		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "position", json_integer(position));
	

		return rootJ;
	};
	
	void fromJson(json_t *rootJ) override {

	
		json_t *positionJ = json_object_get(rootJ, "position");
		if(positionJ) position = json_integer_value(positionJ);

	};


};


void TrigSwitch3::step() {

	for(int i=0; i<8; i++) {
		if( stepTriggers[i].process( inputs[TRIG_INPUT+i].normalize(0.0)) + params[STEP_PARAM+i].value ) position = i;
		lights[i].value = (i==position)?1.0:0.0;
	};

	outputs[OUT1_OUTPUT].value = inputs[CV_INPUT1+position].normalize(0.0);
	outputs[OUT2_OUTPUT].value = inputs[CV_INPUT2+position].normalize(0.0);
	outputs[OUT3_OUTPUT].value = inputs[CV_INPUT3+position].normalize(0.0);

};



struct TrigSwitch3Widget : ModuleWidget {
	TrigSwitch3Widget(TrigSwitch3 *module);
};

TrigSwitch3Widget::TrigSwitch3Widget(TrigSwitch3 *module) : ModuleWidget(module) {

	box.size = Vec(15*12, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/TrigSwitch3.svg")));
		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));


	const float offset_y = 60, delta_y = 32, row1=15, row2 = row1+33, row3 = row2 + 25;

	for (int i=0; i<8; i++) {

		addInput(Port::create<MLPort>(             Vec(row1, offset_y + i*delta_y), Port::INPUT, module, TrigSwitch3::TRIG_INPUT + i));

		addParam(ParamWidget::create<ML_MediumLEDButton>(Vec(row2 , offset_y + i*delta_y +3 ), module, TrigSwitch3::STEP_PARAM + i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MLMediumLight<GreenLight>>( Vec(row2 + 4, offset_y + i*delta_y + 7), module, TrigSwitch3::STEP_LIGHT+i));


		addInput(Port::create<MLPort>(             Vec(row3, offset_y + i*delta_y), Port::INPUT, module, TrigSwitch3::CV_INPUT1 + i));
		addInput(Port::create<MLPort>(             Vec(row3+32, offset_y + i*delta_y), Port::INPUT, module, TrigSwitch3::CV_INPUT2 + i));
		addInput(Port::create<MLPort>(             Vec(row3+64, offset_y + i*delta_y), Port::INPUT, module, TrigSwitch3::CV_INPUT3 + i));

	}
	addOutput(Port::create<MLPort>(Vec(row3,    320), Port::OUTPUT, module, TrigSwitch3::OUT1_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(row3+32, 320), Port::OUTPUT, module, TrigSwitch3::OUT2_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(row3+64, 320), Port::OUTPUT, module, TrigSwitch3::OUT3_OUTPUT));

}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, TrigSwitch3) {
   Model *modelTrigSwitch3 = Model::create<TrigSwitch3, TrigSwitch3Widget>("ML modules", "TrigSwitch3", "TrigSwitch3 8->1", SWITCH_TAG, UTILITY_TAG );
   return modelTrigSwitch3;
}
