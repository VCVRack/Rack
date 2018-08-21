/*
PlainVoidCanvas

*/////////////////////////////////////////////////////////////////////////////
#include "pvc.hpp"

namespace rack_plugin_PvC {

struct PlainVoidCanvas : Module {
	enum ParamIds {

		NUM_PARAMS
	};
	enum InputIds {

		NUM_INPUTS
	};
	enum OutputIds {

		NUM_OUTPUTS
	};
	enum LightIds {
		
		NUM_LIGHTS
	};

	PlainVoidCanvas() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	}

	void step() override;

	// void reset() override;

	// void randomize() override;
	
	// json_t *toJson() override {
		// json_t *rootJ = json_object();
		// return rootJ;
	// }

	// void fromJson(json_t *rootJ) override {
	// }
};


void PlainVoidCanvas::step() {
	//  do stuff
}

struct PlainVoidCanvasWidget : ModuleWidget {
	PlainVoidCanvasWidget(PlainVoidCanvas *module);
};

PlainVoidCanvasWidget::PlainVoidCanvasWidget(PlainVoidCanvas *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/panels/PlainVoidCanvas.svg")));
	
	// screws
	addChild(Widget::create<ScrewHead1>(Vec(0, 0)));
	addChild(Widget::create<ScrewHead2>(Vec(box.size.x - 15, 0)));
	addChild(Widget::create<ScrewHead3>(Vec(0, 365)));
	addChild(Widget::create<ScrewHead4>(Vec(box.size.x - 15, 365)));

	// addChild(ModuleLightWidget::create<FourPixLight<RedLED>>(Vec(,), module, PlainVoidCanvas::LIGHT_ID));
	// addParam(ParamWidget::create<EmptyButton>(Vec(,), module, PlainVoidCanvas::PARAM_ID, 0, 1, 0));
	// addInput(Port::create<InPortAud>(Vec(,), Port::INPUT, module, PlainVoidCanvas::INPUT_ID));
	// addOutput(Port::create<OutPortVal>(Vec(,), Port::OUTPUT, module, PlainVoidCanvas::OUTPUT_ID));
}

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;

RACK_PLUGIN_MODEL_INIT(PvC, PlainVoidCanvas) {
   Model *modelPlainVoidCanvas = Model::create<PlainVoidCanvas, PlainVoidCanvasWidget>(
      "PvC", "PlainVoidCanvas", "PlainVoidCanvas", BLANK_TAG);
   return modelPlainVoidCanvas;
}
