#include "plugin.hpp"


namespace rack {
namespace core {


struct NotesModule : Module {
	std::string text;
	bool dirty = false;

	void onReset() override {
		text = "";
		dirty = true;
	}

	void fromJson(json_t* rootJ) override {
		Module::fromJson(rootJ);
		// In <1.0, module used "text" property at root level.
		json_t* textJ = json_object_get(rootJ, "text");
		if (textJ)
			text = json_string_value(textJ);
		dirty = true;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "text", json_stringn(text.c_str(), text.size()));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* textJ = json_object_get(rootJ, "text");
		if (textJ)
			text = json_string_value(textJ);
		dirty = true;
	}
};


struct NotesTextField : LedDisplayTextField {
	NotesModule* module;

	void step() override {
		LedDisplayTextField::step();
		if (module && module->dirty) {
			setText(module->text);
			module->dirty = false;
		}
	}

	void onChange(const ChangeEvent& e) override {
		if (module)
			module->text = getText();
	}
};


struct NotesDisplay : LedDisplay {
	void setModule(NotesModule* module) {
		NotesTextField* textField = createWidget<NotesTextField>(Vec(0, 0));
		textField->box.size = box.size;
		textField->multiline = true;
		textField->module = module;
		addChild(textField);
	}
};


struct NotesWidget : ModuleWidget {
	NotesWidget(NotesModule* module) {
		setModule(module);
		setPanel(Svg::load(asset::system("res/Core/Notes.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		NotesDisplay* notesDisplay = createWidget<NotesDisplay>(mm2px(Vec(0.0, 12.869)));
		notesDisplay->box.size = mm2px(Vec(81.28, 105.059));
		notesDisplay->setModule(module);
		addChild(notesDisplay);
	}
};


Model* modelNotes = createModel<NotesModule, NotesWidget>("Notes");


} // namespace core
} // namespace rack
