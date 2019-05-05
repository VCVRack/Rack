#include "global_pre.hpp"
#include "Core.hpp"
#include "global.hpp"

using namespace rack;



struct HalfNotesWidget : ModuleWidget {
	TextField *textField;

	HalfNotesWidget(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetGlobal("res/Core/HalfNotes.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		textField = Widget::create<LedDisplayTextField>(mm2px(Vec(3.39962f, 14.8373f)));
		textField->box.size = mm2px(Vec(74.480f*0.46f, 102.753f));
		textField->multiline = true;
		addChild(textField);
	}

	json_t *toJson() override {
		json_t *rootJ = ModuleWidget::toJson();

		// text
		json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		ModuleWidget::fromJson(rootJ);

		// text
		json_t *textJ = json_object_get(rootJ, "text");
		if (textJ)
			textField->text = json_string_value(textJ);
	}
};


RACK_PLUGIN_MODEL_INIT(Core, HalfNotes) {
   Model *modelHalfNotes = Model::create<Module, HalfNotesWidget>("Core", "HalfNotes", "Half Notes", BLANK_TAG);
   return modelHalfNotes;
}
