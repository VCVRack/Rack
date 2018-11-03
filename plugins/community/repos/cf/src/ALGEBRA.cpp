#include "cf.hpp"
#include "dsp/digital.hpp"


using namespace std;

namespace rack_plugin_cf {

struct ALGEBRA : Module {
	enum ParamIds {
		OP_PARAM,
		NUM_PARAMS = OP_PARAM+6
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_LIGHT,
		NUM_LIGHTS = LED_LIGHT+6
	};
	
	int OP_STATE = 0 ;
	SchmittTrigger trTrigger[6];

ALGEBRA() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }

	void step() override;
	
json_t *toJson() override {
		json_t *rootJ = json_object();
		

		json_object_set_new(rootJ, "opstate", json_integer(OP_STATE));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {
		

		json_t *opstateJ = json_object_get(rootJ, "opstate");
		if (opstateJ)
			OP_STATE = json_integer_value(opstateJ);
	
	}
};




void ALGEBRA::step() {
	for (int i=0; i<6; i++) {
		if (trTrigger[i].process(params[OP_PARAM+i].value)) OP_STATE= i;
		if (OP_STATE == i) lights[LED_LIGHT+i].value=1; else lights[LED_LIGHT+i].value=0;
	}
	if (OP_STATE==0) outputs[OUT_OUTPUT].value = inputs[IN1_INPUT].value + inputs[IN2_INPUT].value;
	if (OP_STATE==1) outputs[OUT_OUTPUT].value = inputs[IN1_INPUT].value - inputs[IN2_INPUT].value;
	if (OP_STATE==2) outputs[OUT_OUTPUT].value = inputs[IN1_INPUT].value * inputs[IN2_INPUT].value;
	if ((OP_STATE==3) & (inputs[IN2_INPUT].value!=0)) outputs[OUT_OUTPUT].value = inputs[IN1_INPUT].value / inputs[IN2_INPUT].value;
	if (OP_STATE==4) {
			if (inputs[IN1_INPUT].value>=inputs[IN2_INPUT].value)	outputs[OUT_OUTPUT].value = inputs[IN1_INPUT].value;
				else outputs[OUT_OUTPUT].value = inputs[IN2_INPUT].value;
			}
	if (OP_STATE==5) {
			if (inputs[IN1_INPUT].value<=inputs[IN2_INPUT].value)	outputs[OUT_OUTPUT].value = inputs[IN1_INPUT].value;
				else outputs[OUT_OUTPUT].value = inputs[IN2_INPUT].value;
			}

}

struct plusButton : SVGSwitch, MomentarySwitch {
	plusButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/plusButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/plusButton.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct minusButton : SVGSwitch, MomentarySwitch {
	minusButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/minusButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/minusButton.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct multButton : SVGSwitch, MomentarySwitch {
	multButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/multButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/multButton.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct divButton : SVGSwitch, MomentarySwitch {
	divButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/divButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/divButton.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct maxButton : SVGSwitch, MomentarySwitch {
	maxButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/maxButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/maxButton.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct minButton : SVGSwitch, MomentarySwitch {
	minButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/minButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/minButton.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct ALGEBRAWidget : ModuleWidget {
	ALGEBRAWidget(ALGEBRA *module);
 //void step() override;

};

ALGEBRAWidget::ALGEBRAWidget(ALGEBRA *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/ALGEBRA.svg")));


	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));


	addInput(Port::create<PJ301MPort>(Vec(3, 31), Port::INPUT, module, ALGEBRA::IN1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(3, 95), Port::INPUT, module, ALGEBRA::IN2_INPUT));

	int i = 0;
		addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(3+4.4, i*24+133+4.4), module, ALGEBRA::LED_LIGHT + i));
     		addParam(ParamWidget::create<plusButton>(Vec(6, i*24+136), module, ALGEBRA::OP_PARAM + i, 0.0, 1.0, 0.0)); i=i+1;
		addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(3+4.4, i*24+133+4.4), module, ALGEBRA::LED_LIGHT + i));
     		addParam(ParamWidget::create<minusButton>(Vec(6, i*24+136), module, ALGEBRA::OP_PARAM + i, 0.0, 1.0, 0.0)); i=i+1;
		addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(3+4.4, i*24+133+4.4), module, ALGEBRA::LED_LIGHT + i));
     		addParam(ParamWidget::create<multButton>(Vec(6, i*24+136), module, ALGEBRA::OP_PARAM + i, 0.0, 1.0, 0.0)); i=i+1;
		addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(3+4.4, i*24+133+4.4), module, ALGEBRA::LED_LIGHT + i));
     		addParam(ParamWidget::create<divButton>(Vec(6, i*24+136), module, ALGEBRA::OP_PARAM + i, 0.0, 1.0, 0.0)); i=i+1;
		addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(3+4.4, i*24+133+4.4), module, ALGEBRA::LED_LIGHT + i));
     		addParam(ParamWidget::create<maxButton>(Vec(6, i*24+136), module, ALGEBRA::OP_PARAM + i, 0.0, 1.0, 0.0)); i=i+1;
		addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(3+4.4, i*24+133+4.4), module, ALGEBRA::LED_LIGHT + i));
     		addParam(ParamWidget::create<minButton>(Vec(6, i*24+136), module, ALGEBRA::OP_PARAM + i, 0.0, 1.0, 0.0));
		
	


	addOutput(Port::create<PJ301MPort>(Vec(3, 321), Port::OUTPUT, module, ALGEBRA::OUT_OUTPUT));
	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, ALGEBRA) {
   Model *modelALGEBRA = Model::create<ALGEBRA, ALGEBRAWidget>("cf", "ALGEBRA", "Algebra", UTILITY_TAG);
   return modelALGEBRA;
}
