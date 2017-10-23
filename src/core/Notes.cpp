#include "core.hpp"

using namespace rack;



NotesWidget::NotesWidget() {
	box.size = Vec(RACK_GRID_WIDTH * 18, RACK_GRID_HEIGHT);

	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));

	textField = new TextField();
	textField->box.pos = Vec(15, 15);
	textField->box.size = box.size.minus(Vec(30, 30));
	textField->multiline = true;
	addChild(textField);
}

json_t *NotesWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();

	// text
	json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

	return rootJ;
}

void NotesWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);

	// text
	json_t *textJ = json_object_get(rootJ, "text");
	if (textJ)
		textField->text = json_string_value(textJ);
}
