/*
 Vamps

 a 2 RackUnit stereo mod of Andrew Belt's Fundamental VCA

 MAYBE TODO:
  - third channel

*/////////////////////////////////////////////////////////////////////////////



#include "pvc.hpp"

namespace rack_plugin_PvC {

struct Vamps : Module {
	enum ParamIds {
		LEVEL,
		
		NUM_PARAMS
	};
	enum InputIds {
		EXP_CV,
		LIN_CV,
		IN_L,
		IN_R,
		
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_L,
		OUT_R,
		
		NUM_OUTPUTS
	};

	Vamps() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};


void Vamps::step() {

	float left = inputs[IN_L].value * params[LEVEL].value;
	float right = inputs[IN_R].normalize(inputs[IN_L].value) * params[LEVEL].value;
	
	const float expBase = 50.0f;

	if (inputs[LIN_CV].active) {
		float linCV = clamp(inputs[LIN_CV].value * 0.1f, 0.0f, 1.0f);
		left *= linCV;
		right *= linCV;
	}
	if (inputs[EXP_CV].active) {
		float expCV = rescale(powf(expBase, clamp(inputs[EXP_CV].value / 10.0f, 0.0f, 1.0f)), 1.0, expBase, 0.0f, 1.0f);
		left *= expCV;
		right *= expCV;
	}
	outputs[OUT_L].value = left;
	outputs[OUT_R].value = right;
}

struct VampsWidget : ModuleWidget {	VampsWidget(Vamps *module); };

VampsWidget::VampsWidget(Vamps *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/panels/Vamps.svg")));
	// screws
	addChild(Widget::create<ScrewHead1>(Vec(0, 0)));
	//addChild(Widget::create<ScrewHead2>(Vec(box.size.x - 15, 0)));
	//addChild(Widget::create<ScrewHead3>(Vec(0, 365)));
	addChild(Widget::create<ScrewHead4>(Vec(box.size.x - 15, 365)));
	
	addInput(Port::create<InPortAud>(Vec(4, 22), Port::INPUT, module, Vamps::IN_L));
	addInput(Port::create<InPortAud>(Vec(4, 64), Port::INPUT, module, Vamps::IN_R));

	addParam(ParamWidget::create<PvCKnob>(Vec(4, 120), module, Vamps::LEVEL, 0.0f, 1.0f, 0.5f));
	addInput(Port::create<InPortCtrl>(Vec(4, 164), Port::INPUT, module, Vamps::EXP_CV));
	addInput(Port::create<InPortCtrl>(Vec(4, 208), Port::INPUT, module, Vamps::LIN_CV));

	addOutput(Port::create<OutPortVal>(Vec(4, 296), Port::OUTPUT, module, Vamps::OUT_L));
	addOutput(Port::create<OutPortVal>(Vec(4, 336), Port::OUTPUT, module, Vamps::OUT_R));
}

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;

RACK_PLUGIN_MODEL_INIT(PvC, Vamps) {
   Model *modelVamps = Model::create<Vamps, VampsWidget>(
      "PvC", "Vamps", "Vamps", AMPLIFIER_TAG, ATTENUATOR_TAG, DUAL_TAG);
   return modelVamps;
}
