#include "plugin.hpp"


namespace rack {
namespace core {


struct NotesModule : Module {
	std::string text;

	/** Legacy for <=v1 patches */
	void fromJson(json_t* rootJ) override {
		Module::fromJson(rootJ);
		json_t* textJ = json_object_get(rootJ, "text");
		if (textJ)
			text = json_string_value(textJ);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "text", json_string(text.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* textJ = json_object_get(rootJ, "text");
		if (textJ)
			text = json_string_value(textJ);
	}
};


struct NotesWidget : ModuleWidget {
	// TODO Subclass this or something and keep `module->text` in sync with the text field's string.
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
};


Model* modelNotes = createModel<Module, NotesWidget>("Notes");


} // namespace core
} // namespace rack
