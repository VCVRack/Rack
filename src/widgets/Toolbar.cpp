#include "Rack.hpp"

extern "C" {
	#include "../lib/noc/noc_file_dialog.h"
}


namespace rack {

static const char *filters = "JSON Patch\0*.json\0";


struct NewItem : MenuItem {
	void onAction() {
		gRackWidget->clear();
	}
};

struct SaveItem : MenuItem {
	void onAction() {
		const char *path = noc_file_dialog_open(NOC_FILE_DIALOG_SAVE, filters, NULL, "Untitled.json");
		if (path) {
			gRackWidget->savePatch(path);
		}
	}
};

struct OpenItem : MenuItem {
	void onAction() {
		const char *path = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, filters, NULL, NULL);
		if (path) {
			gRackWidget->loadPatch(path);
		}
	}
};

struct FileChoice : ChoiceButton {
	void onAction() {
		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;
		{
			MenuItem *newItem = new NewItem();
			newItem->text = "New";
			menu->pushChild(newItem);

			MenuItem *openItem = new OpenItem();
			openItem->text = "Open";
			menu->pushChild(openItem);

			MenuItem *saveItem = new SaveItem();
			saveItem->text = "Save";
			menu->pushChild(saveItem);

			MenuItem *saveAsItem = new SaveItem();
			saveAsItem->text = "Save As";
			menu->pushChild(saveAsItem);
		}
		overlay->addChild(menu);
		gScene->addChild(overlay);
	}
};


struct SampleRateItem : MenuItem {
	float sampleRate;
	void onAction() {
		printf("\"\"\"\"\"\"\"\"switching\"\"\"\"\"\"\"\" sample rate to %f\n", sampleRate);
	}
};

struct SampleRateChoice : ChoiceButton {
	void onAction() {
		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		float sampleRates[6] = {44100, 48000, 88200, 96000, 176400, 192000};
		for (int i = 0; i < 6; i++) {
			SampleRateItem *item = new SampleRateItem();
			char text[100];
			snprintf(text, 100, "%.0f Hz", sampleRates[i]);
			item->text = std::string(text);
			item->sampleRate = sampleRates[i];
			menu->pushChild(item);
		}

		overlay->addChild(menu);
		gScene->addChild(overlay);
	}
};


Toolbar::Toolbar() {
	float margin = 5;
	box.size = Vec(1020, BND_WIDGET_HEIGHT + 2*margin);

	float xPos = margin;
	{
		Label *label = new Label();
		label->box.pos = Vec(xPos, margin);
		label->text = "Rack v0.0.0 alpha";
		addChild(label);
		xPos += 150;
	}

	xPos += margin;
	{
		ChoiceButton *fileChoice = new FileChoice();
		fileChoice->box.pos = Vec(xPos, margin);
		fileChoice->box.size.x = 100;
		fileChoice->text = "File";
		addChild(fileChoice);
		xPos += fileChoice->box.size.x;
	}

	xPos += margin;
	{
		SampleRateChoice *srChoice = new SampleRateChoice();
		srChoice->box.pos = Vec(xPos, margin);
		srChoice->box.size.x = 100;
		// TODO Change to actual sample rate, e.g. 44100 Hz
		srChoice->text = "Sample Rate";
		addChild(srChoice);
		xPos += srChoice->box.size.x;
	}

	xPos += margin;
	{
		wireOpacitySlider = new Slider();
		wireOpacitySlider->box.pos = Vec(xPos, margin);
		wireOpacitySlider->box.size.x = 150;
		wireOpacitySlider->label = "Wire opacity";
		wireOpacitySlider->unit = "%";
		wireOpacitySlider->setLimits(0.0, 100.0);
		wireOpacitySlider->setDefaultValue(100.0);
		addChild(wireOpacitySlider);
		xPos += wireOpacitySlider->box.size.x;
	}
}

void Toolbar::draw(NVGcontext *vg) {
	bndBackground(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	bndBevel(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);

	Widget::draw(vg);
}


} // namespace rack
