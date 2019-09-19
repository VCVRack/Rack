#include "plugin.hpp"


namespace rack {
namespace core {


struct NotesWidget : ModuleWidget {
	TextField* textField;

	NotesWidget(Module* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::system("res/Core/Notes.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		textField = createWidget<LedDisplayTextField>(mm2px(Vec(3.39962, 14.8373)));
		textField->box.size = mm2px(Vec(74.480, 102.753));
		textField->multiline = true;
		addChild(textField);
	}

	json_t* toJson() override {
		json_t* rootJ = ModuleWidget::toJson();

		// text
		json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

		return rootJ;
	}

	void fromJson(json_t* rootJ) override {
		ModuleWidget::fromJson(rootJ);

		// text
		json_t* textJ = json_object_get(rootJ, "text");
		if (textJ)
			textField->text = json_string_value(textJ);
	}
};


Model* modelNotes = createModel<Module, NotesWidget>("Notes");


} // namespace core
} // namespace rack
