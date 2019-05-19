#include "Fundamental.hpp"

namespace rack_plugin_Fundamental {

struct _8vert : Module {
	enum ParamIds {
		NUM_PARAMS = 8
	};
	enum InputIds {
		NUM_INPUTS = 8
	};
	enum OutputIds {
		NUM_OUTPUTS = 8
	};
	enum LightIds {
		NUM_LIGHTS = 16
	};

	_8vert() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void _8vert::step() {
	float lastIn = 10.0f;
	for (int i = 0; i < 8; i++) {
		lastIn = inputs[i].normalize(lastIn);
		float out = lastIn * params[i].value;
		outputs[i].value = out;
		lights[2*i + 0].setBrightnessSmooth(fmaxf(0.0f, out / 5.0f));
		lights[2*i + 1].setBrightnessSmooth(fmaxf(0.0f, -out / 5.0f));
	}
}


struct _8vertWidget : ModuleWidget {
	_8vertWidget(_8vert *module);
};

_8vertWidget::_8vertWidget(_8vert *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/8vert.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(45.308, 47.753), module, 0, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(45.308, 86.198), module, 1, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(45.308, 124.639), module, 2, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(45.308, 163.084), module, 3, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(45.308, 201.529), module, 4, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(45.308, 239.974), module, 5, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(45.308, 278.415), module, 6, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(45.308, 316.86), module, 7, -1.0f, 1.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(Vec(9.507, 50.397), Port::INPUT, module, 0));
	addInput(Port::create<PJ301MPort>(Vec(9.507, 88.842), Port::INPUT, module, 1));
	addInput(Port::create<PJ301MPort>(Vec(9.507, 127.283), Port::INPUT, module, 2));
	addInput(Port::create<PJ301MPort>(Vec(9.507, 165.728), Port::INPUT, module, 3));
	addInput(Port::create<PJ301MPort>(Vec(9.507, 204.173), Port::INPUT, module, 4));
	addInput(Port::create<PJ301MPort>(Vec(9.507, 242.614), Port::INPUT, module, 5));
	addInput(Port::create<PJ301MPort>(Vec(9.507, 281.059), Port::INPUT, module, 6));
	addInput(Port::create<PJ301MPort>(Vec(9.507, 319.504), Port::INPUT, module, 7));

	addOutput(Port::create<PJ301MPort>(Vec(86.393, 50.397), Port::OUTPUT, module, 0));
	addOutput(Port::create<PJ301MPort>(Vec(86.393, 88.842), Port::OUTPUT, module, 1));
	addOutput(Port::create<PJ301MPort>(Vec(86.393, 127.283), Port::OUTPUT, module, 2));
	addOutput(Port::create<PJ301MPort>(Vec(86.393, 165.728), Port::OUTPUT, module, 3));
	addOutput(Port::create<PJ301MPort>(Vec(86.393, 204.173), Port::OUTPUT, module, 4));
	addOutput(Port::create<PJ301MPort>(Vec(86.393, 242.614), Port::OUTPUT, module, 5));
	addOutput(Port::create<PJ301MPort>(Vec(86.393, 281.059), Port::OUTPUT, module, 6));
	addOutput(Port::create<PJ301MPort>(Vec(86.393, 319.504), Port::OUTPUT, module, 7));

	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(107.702, 50.414), module, 0));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(107.702, 88.859), module, 2));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(107.702, 127.304), module, 4));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(107.702, 165.745), module, 6));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(107.702, 204.19), module, 8));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(107.702, 242.635), module, 10));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(107.702, 281.076), module, 12));
	addChild(ModuleLightWidget::create<TinyLight<GreenRedLight>>(Vec(107.702, 319.521), module, 14));
}

} // namespace rack_plugin_Fundamental

using namespace rack_plugin_Fundamental;

RACK_PLUGIN_MODEL_INIT(Fundamental, 8vert) {
   Model *model = Model::create<_8vert, _8vertWidget>("Fundamental", "8vert", "8vert", ATTENUATOR_TAG);
   return model;
}

