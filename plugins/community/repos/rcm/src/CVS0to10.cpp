#include "rcm.h"
#include "GVerbWidget.hpp"
#include "../include/BaseWidget.hpp"

namespace rack_plugin_rcm {

struct CVS0to10Module : Module {
	enum ParamIds {
		AMOUNT_PARAM_A,
		AMOUNT_PARAM_B,
		AMOUNT_PARAM_C,
		AMOUNT_PARAM_D,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT_A,
		CV_OUTPUT_B,
		CV_OUTPUT_C,
		CV_OUTPUT_D,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CVS0to10Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void CVS0to10Module::step() {
	outputs[CV_OUTPUT_A].value = params[AMOUNT_PARAM_A].value;
	outputs[CV_OUTPUT_B].value = params[AMOUNT_PARAM_B].value;
	outputs[CV_OUTPUT_C].value = params[AMOUNT_PARAM_C].value;
	outputs[CV_OUTPUT_D].value = params[AMOUNT_PARAM_D].value;
}

struct CVS0to10ModuleWidget : BaseWidget {
    TextField *textField;

	CVS0to10ModuleWidget(CVS0to10Module *module) : BaseWidget(module) {
		colourHotZone = Rect(Vec(10, 10), Vec(50, 13));
		backgroundHue = 0.754;
		backgroundSaturation = 1.f;
		backgroundLuminosity = 0.58f;

		setPanel(SVG::load(assetPlugin(plugin, "res/CVS0to10.svg")));

        auto x = 6.f;
		addParam(ParamWidget::create<LEDSliderWhite>(Vec(11.5-x, 135), module, CVS0to10Module::AMOUNT_PARAM_A, 0.0, 10.0, 0.0));
		addParam(ParamWidget::create<LEDSliderWhite>(Vec(26.0-x, 135), module, CVS0to10Module::AMOUNT_PARAM_B, 0.0, 10.0, 0.0));
		addParam(ParamWidget::create<LEDSliderWhite>(Vec(40.5-x, 135), module, CVS0to10Module::AMOUNT_PARAM_C, 0.0, 10.0, 0.0));
		addParam(ParamWidget::create<LEDSliderWhite>(Vec(55.0-x, 135), module, CVS0to10Module::AMOUNT_PARAM_D, 0.0, 10.0, 0.0));

		addOutput(Port::create<PJ301MPort>(Vec(12.5, 278), Port::OUTPUT, module, CVS0to10Module::CV_OUTPUT_A));
		addOutput(Port::create<PJ301MPort>(Vec(42, 278), Port::OUTPUT, module, CVS0to10Module::CV_OUTPUT_B));
		addOutput(Port::create<PJ301MPort>(Vec(12.5, 317), Port::OUTPUT, module, CVS0to10Module::CV_OUTPUT_C));
		addOutput(Port::create<PJ301MPort>(Vec(42, 317), Port::OUTPUT, module, CVS0to10Module::CV_OUTPUT_D));

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

RACK_PLUGIN_MODEL_INIT(rcm, CVS0to10Module) {
   Model *modelCVS0to10Module = Model::create<CVS0to10Module, CVS0to10ModuleWidget>("rcm", "rcm-CVS0to10", "CVS0to10", ENVELOPE_FOLLOWER_TAG);
   return modelCVS0to10Module;
}
