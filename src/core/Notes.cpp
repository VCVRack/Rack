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

	/** Legacy for <=v1 patches */
	void fromJson(json_t* rootJ) override {
		Module::fromJson(rootJ);
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
		TextField::step();
		if (module && module->dirty) {
			setText(module->text);
			module->dirty = false;
		}
	}
	void onChange(const ChangeEvent& e) override {
		if (module)
			module->text = text;
	}
};


struct NotesWidget : ModuleWidget {
	NotesTextField* textField;

	NotesWidget(NotesModule* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::system("res/Core/Notes.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		textField = createWidget<NotesTextField>(mm2px(Vec(3.39962, 14.8373)));
		textField->box.size = mm2px(Vec(74.480, 102.753));
		textField->multiline = true;
		textField->module = dynamic_cast<NotesModule*>(module);
		addChild(textField);
	}
};


Model* modelNotes = createModel<NotesModule, NotesWidget>("Notes");


} // namespace core
} // namespace rack
