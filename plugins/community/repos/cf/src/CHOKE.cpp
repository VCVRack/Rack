#include "cf.hpp"
#include "dsp/digital.hpp"


using namespace std;

namespace rack_plugin_cf {

struct CHOKE : Module {
	enum ParamIds {
		TR1_PARAM,
		TR2_PARAM,
		PAN_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		TRIG1_INPUT,
		TRIG2_INPUT,
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		L2_LIGHT,
		NUM_LIGHTS
	};
	
	bool play = false;
	SchmittTrigger tr1Trigger;
	SchmittTrigger tr2Trigger;

CHOKE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }

	void step() override;
	

};




void CHOKE::step() {

	if (tr1Trigger.process(inputs[TRIG1_INPUT].value))
		{
		play = false ;

		};
	if (tr2Trigger.process(inputs[TRIG2_INPUT].value))
		{
		play = true ;

		};
	if (play) 
		outputs[OUT_OUTPUT].value = inputs[IN2_INPUT].value*(1-clamp(-params[PAN_PARAM].value,0.0f,1.0f));
			else outputs[OUT_OUTPUT].value = inputs[IN1_INPUT].value*(1-clamp(params[PAN_PARAM].value,0.0f,1.0f)); 
		

lights[L2_LIGHT].value=play;
}




struct CHOKEWidget : ModuleWidget {
	CHOKEWidget(CHOKE *module);
 //void step() override;

};

CHOKEWidget::CHOKEWidget(CHOKE *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/CHOKE.svg")));


	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

 	addParam(ParamWidget::create<Trimpot>(Vec(6, 298), module, CHOKE::PAN_PARAM, -1.0f, 1.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(Vec(3, 61), Port::INPUT, module, CHOKE::IN1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(3, 91), Port::INPUT, module, CHOKE::TRIG1_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(3, 181), Port::INPUT, module, CHOKE::IN2_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(3, 211), Port::INPUT, module, CHOKE::TRIG2_INPUT));

 addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(8, 276), module, CHOKE::L2_LIGHT));

	addOutput(Port::create<PJ301MPort>(Vec(3, 321), Port::OUTPUT, module, CHOKE::OUT_OUTPUT));

}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;


RACK_PLUGIN_MODEL_INIT(cf, CHOKE) {
   Model *modelCHOKE = Model::create<CHOKE, CHOKEWidget>("cf", "CHOKE", "Choke", UTILITY_TAG);
   return modelCHOKE;
}
