#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct DUKE : Module {
	enum ParamIds {
		SLIDER_PARAM,
		ADONF_PARAM = SLIDER_PARAM + 4,
		NADA_PARAM,
		MIN_PARAM = NADA_PARAM + 4,
		MAX_PARAM = MIN_PARAM + 4,
		TYPE_PARAM = MAX_PARAM + 4,
		NUM_PARAMS = TYPE_PARAM + 4,
	};
	enum InputIds {
		SLIDER_INPUT,
		NUM_INPUTS = SLIDER_INPUT + 4
	};
	enum OutputIds {
		CV_OUTPUT,
		NUM_OUTPUTS = CV_OUTPUT + 4
	};
	enum LightIds {
		NUM_LIGHTS
	};

	DUKE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	}

	void step() override;
};


void DUKE::step() {
	for (int i = 0; i < 4; i ++) {
			float min = params[MIN_PARAM + i].value - 5.0f * params[TYPE_PARAM + i].value;
			float max = params[MAX_PARAM + i].value - 5.0f * params[TYPE_PARAM + i].value;
			outputs[CV_OUTPUT + i].value = min + clamp(params[SLIDER_PARAM + i].value + inputs[SLIDER_INPUT + i].value, 0.0f , 10.0f) * (max - min) / 10.0f;
	}
}

struct DUKEWidget : ModuleWidget {
	ParamWidget *sliders[4];
	DUKEWidget(DUKE *module);
};

struct DUKECKD6 : BlueCKD6 {
	void onMouseDown(EventMouseDown &e) override {
		DUKEWidget *parent = dynamic_cast<DUKEWidget*>(this->parent);
		DUKE *module = dynamic_cast<DUKE*>(this->module);
		if (parent && module) {
			if (this->paramId == DUKE::ADONF_PARAM) {
				for (int i = 0; i < 4 ; i++) {
					parent->sliders[i]->setValue(10);
					module->params[DUKE::SLIDER_PARAM + i].value = 10.0f;
				}
			} else if (this->paramId == DUKE::NADA_PARAM) {
				for (int i = 0; i < 4 ; i++) {
					parent->sliders[i]->setValue(0.0f);
					module->params[DUKE::SLIDER_PARAM + i].value = 0.0f;
				}
			}
		}
		BlueCKD6::onMouseDown(e);
	}
};

DUKEWidget::DUKEWidget(DUKE *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/DUKE.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	static const float portX0[4] = {36.0f, 70.0f, 105.0f, 140.0f};

	addParam(ParamWidget::create<DUKECKD6>(Vec(portX0[0]-29.0f, 190.0f), module, DUKE::ADONF_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<DUKECKD6>(Vec(portX0[0]-29.0f, 230.0f), module, DUKE::NADA_PARAM, 0.0f, 1.0f, 0.0f));

	for (int i = 0; i < 4; i++) {
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[i]-2.0f, 51.0f), module, DUKE::MAX_PARAM + i, 0.0f, 10.0f, 10.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[i]-2.0f, 95.0f), module, DUKE::MIN_PARAM + i, 0.0f, 10.0f, 0.0f));
		addParam(ParamWidget::create<CKSS>(Vec(portX0[i]+6.0f, 139.0f), module, DUKE::TYPE_PARAM + i, 0.0f, 1.0f, 0.0f));
		sliders[i] = ParamWidget::create<BidooLongSlider>(Vec(portX0[i]+4.0f, 176.0f), module, DUKE::SLIDER_PARAM + i, 0.0f, 10.0f, 0.0f);
		addParam(sliders[i]);
		addInput(Port::create<PJ301MPort>(Vec(portX0[i]+1.0f, 284.0f), Port::INPUT, module, DUKE::SLIDER_INPUT + i));
		addOutput(Port::create<PJ301MPort>(Vec(portX0[i]+1.0f, 323.0f), Port::OUTPUT, module, DUKE::CV_OUTPUT + i));
	}
}

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, DUKE) {
   Model *modelDUKE = Model::create<DUKE, DUKEWidget>("Bidoo","dUKe", "dUKe controller", CONTROLLER_TAG);
   return modelDUKE;
}
