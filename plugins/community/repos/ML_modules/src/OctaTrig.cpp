#include "ML_modules.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct OctaTrig : Module {
	enum ParamIds {
		RESET_PARAM,
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
		NUM_INPUTS
	};
	enum OutputIds {
		UP1_OUTPUT,
		UP2_OUTPUT,
		UP3_OUTPUT,
		UP4_OUTPUT,
		UP5_OUTPUT,
		UP6_OUTPUT,
		UP7_OUTPUT,
		UP8_OUTPUT,
		DN1_OUTPUT,
		DN2_OUTPUT,
		DN3_OUTPUT,
		DN4_OUTPUT,
		DN5_OUTPUT,
		DN6_OUTPUT,
		DN7_OUTPUT,
		DN8_OUTPUT,
		SUM1_OUTPUT,
		SUM2_OUTPUT,
		SUM3_OUTPUT,
		SUM4_OUTPUT,
		SUM5_OUTPUT,
		SUM6_OUTPUT,
		SUM7_OUTPUT,
		SUM8_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};

	PulseGenerator upPulse[8], dnPulse[8];

	bool state[8] = {};

	float out_up[8] = {};
	float out_dn[8] = {};

	float delta;


	OctaTrig() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { 
		for(int i=0; i<8; i++) state[i]=false; 
		onSampleRateChange();
	};

	void onSampleRateChange() override { delta = 1.0/engineGetSampleRate(); }

	void step() override;

};



void OctaTrig::step() {

	bool gate[8];



	for(int i=0; i<8; i++) {
		
		gate[i] = inputs[IN1_INPUT+i].normalize(0.0) > 1.0;

		if (gate[i]^state[i]) {

			if(gate[i]) upPulse[i].trigger(0.01);
			else	    dnPulse[i].trigger(0.01);
		}

		out_up[i] = upPulse[i].process(delta)?10.0:0.0;
		out_dn[i] = dnPulse[i].process(delta)?10.0:0.0;

		state[i] = gate[i];
	}



	for(int i=0; i<8; i++) { 
		outputs[UP1_OUTPUT+i].value  = out_up[i];
		outputs[DN1_OUTPUT+i].value  = out_dn[i];
		outputs[SUM1_OUTPUT+i].value = out_up[i] + out_dn[i];
	}

};



struct OctaTrigWidget : ModuleWidget {
	OctaTrigWidget(OctaTrig *module);
};

OctaTrigWidget::OctaTrigWidget(OctaTrig *module) : ModuleWidget(module) {

	box.size = Vec(15*10, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/OctaTrig.svg")));

		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));



	const float offset_y = 60, delta_y = 32, row1=15, row2 = 50, row3 = 80, row4 = 110;

	for( int i=0; i<8; i++) {
		addInput(Port::create<MLPort>(Vec(row1, offset_y + i*delta_y  ), Port::INPUT, module, OctaTrig::IN1_INPUT+i));

		addOutput(Port::create<MLPort>(Vec(row2, offset_y + i*delta_y ), Port::OUTPUT, module, OctaTrig::UP1_OUTPUT+i));
		addOutput(Port::create<MLPort>(Vec(row3, offset_y + i*delta_y ), Port::OUTPUT, module, OctaTrig::DN1_OUTPUT+i));
		addOutput(Port::create<MLPort>(Vec(row4, offset_y + i*delta_y ), Port::OUTPUT, module, OctaTrig::SUM1_OUTPUT+i));
	};

}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, OctaTrig) {
   Model *modelOctaTrig = Model::create<OctaTrig, OctaTrigWidget>("ML modules", "OctaTrig", "OctaTrig", UTILITY_TAG );
   return modelOctaTrig;
}
