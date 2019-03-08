#include "rcm.h"
#include "GVerbWidget.hpp"
#include "../include/BaseWidget.hpp"

namespace rack_plugin_rcm {

struct CV0to10Module : Module {
	enum ParamIds {
		AMOUNT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CV0to10Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void CV0to10Module::step() {
	outputs[CV_OUTPUT].value = params[AMOUNT_PARAM].value;
}

struct CV0to10ModuleWidget : BaseWidget {
    TextField *textField;

	CV0to10ModuleWidget(CV0to10Module *module) : BaseWidget(module) {
		colourHotZone = Rect(Vec(10, 10), Vec(50, 13));
		backgroundHue = 0.754;
		backgroundSaturation = 1.f;
		backgroundLuminosity = 0.58f;

		setPanel(SVG::load(assetPlugin(plugin, "res/CV0to10.svg")));

		addParam(ParamWidget::create<Davies1900hLargeWhiteKnob>(Vec(10, 156.23), module, CV0to10Module::AMOUNT_PARAM, 0.0, 10.0, 0.0));

		addOutput(Port::create<PJ301MPort>(Vec(26, 331), Port::OUTPUT, module, CV0to10Module::CV_OUTPUT));

        textField = Widget::create<LedDisplayTextField>(Vec(7.5, 38.0));
		textField->box.size = Vec(60.0, 80.0);
		textField->multiline = true;
        ((LedDisplayTextField*)textField)->color = COLOR_WHITE;
		addChild(textField);

	}

	json_t *toJson() override {
		json_t *rootJ = BaseWidget::toJson();

		// text
		json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		BaseWidget::fromJson(rootJ);

		// text
		json_t *textJ = json_object_get(rootJ, "text");
		if (textJ)
			textField->text = json_string_value(textJ);
	}
};

} // namespace rack_plugin_rcm

using namespace rack_plugin_rcm;

RACK_PLUGIN_MODEL_INIT(rcm, CV0to10Module) {
   Model *modelCV0to10Module = Model::create<CV0to10Module, CV0to10ModuleWidget>("rcm", "rcm-CV0to10", "CV0to10", ENVELOPE_FOLLOWER_TAG);
   return modelCV0to10Module;
}
