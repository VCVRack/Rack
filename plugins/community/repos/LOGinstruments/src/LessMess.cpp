#include <string>
#include <math.h>
#include <float.h>
#include "LOGinstruments.hpp"

namespace rack_plugin_LOGinstruments {

struct LessMessWidget;

struct LessMess : Module {
	enum ParamIds {
		NUM_PARAMS,
	};
	enum InputIds {
		INPUT1,
		INPUT2,
		INPUT3,
		INPUT4,
		INPUT5,
		INPUT6,
		INPUT7,
		INPUT8,
		INPUT9,
		NUM_INPUTS,
	};
	enum OutputIds {
		OUTPUT1,
		OUTPUT2,
		OUTPUT3,
		OUTPUT4,
		OUTPUT5,
		OUTPUT6,
		OUTPUT7,
		OUTPUT8,
		OUTPUT9,
		NUM_OUTPUTS,
	};

	LessMess() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 0) {}
	void step() override;

	LessMessWidget * parent;

	void reset() override {
		;
	}
};

void LessMess::step() {

	for (int i = 0; i < NUM_INPUTS; i++) {
		if (inputs[i].active)
			outputs[i].value = inputs[i].value;
	}
}

#define V_SEP 35
struct LessMessWidget : ModuleWidget {
	TextField ** label;
	LessMessWidget(LessMess *module);
	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
};

LessMessWidget::LessMessWidget(LessMess *module) : ModuleWidget(module) {
	label = new TextField*[LessMess::NUM_INPUTS];

	box.size = Vec(15*16, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/LessMess_nofonts.svg")));
		addChild(panel);
	}

	for (int i = 0; i < LessMess::NUM_INPUTS; i++) {
		addInput(Port::create<PJ301MPort>(Vec(10, 30 + i*V_SEP), Port::INPUT, module, i));

		label[i] = new TextField();
		/*label[i]->text = "cable " + std::to_string(i);
		label[i]->placeholder = "plc:";*/
		label[i]->box.pos = Vec(40, 32 + i * V_SEP);
		label[i]->box.size.x = box.size.x-75;
		addChild(label[i]);

		addOutput(Port::create<PJ301MPort>(Vec(box.size.x-30, 30 + i * V_SEP), Port::OUTPUT, module, i));
	}

}


json_t *LessMessWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();

	for (int i = 0; i < LessMess::NUM_INPUTS; i++) {
		json_object_set_new(rootJ, ("label" + std::to_string(i)).c_str(), json_string( label[i]->text.c_str() ));
	}
	return rootJ;
}

void LessMessWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);

	for (int i = 0; i < LessMess::NUM_INPUTS; i++) {
		json_t *labJ = json_object_get(rootJ, ("label" + std::to_string(i)).c_str());
		if (labJ) {
			label[i]->text = json_string_value(labJ);
		}
	}
}

} // namespace rack_plugin_LOGinstruments

using namespace rack_plugin_LOGinstruments;

RACK_PLUGIN_MODEL_INIT(LOGinstruments, LessMess) {
   Model *modelLessMess = Model::create<LessMess, LessMessWidget>("LOGinstruments", "LessMess", "Tidy Up Cables", UTILITY_TAG, VISUAL_TAG);
   return modelLessMess;
}

