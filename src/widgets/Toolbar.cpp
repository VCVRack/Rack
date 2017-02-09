#include "scene.hpp"
#include "gui.hpp"
#include "engine.hpp"


namespace rack {

static const char *filters = "JSON Patch\0*.json\0";


struct NewItem : MenuItem {
	void onAction() {
		gRackWidget->clear();
	}
};

struct SaveItem : MenuItem {
	void onAction() {
		const char *path = guiSaveDialog(filters, "Untitled.json");
		if (path) {
			gRackWidget->savePatch(path);
		}
	}
};

struct OpenItem : MenuItem {
	void onAction() {
		const char *path = guiOpenDialog(filters, NULL);
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

			// MenuItem *saveItem = new SaveItem();
			// saveItem->text = "Save";
			// menu->pushChild(saveItem);

			MenuItem *saveAsItem = new SaveItem();
			saveAsItem->text = "Save As";
			menu->pushChild(saveAsItem);
		}
		overlay->addChild(menu);
		gScene->setOverlay(overlay);
	}
};


struct SampleRateItem : MenuItem {
	float sampleRate;
	void onAction() {
		gSampleRate = sampleRate;
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
			item->text = stringf("%.0f Hz", sampleRates[i]);
			item->sampleRate = sampleRates[i];
			menu->pushChild(item);
		}

		overlay->addChild(menu);
		gScene->setOverlay(overlay);
	}
	void step() {
		text = stringf("%.0f Hz", gSampleRate);
	}
};


Toolbar::Toolbar() {
	float margin = 5;
	box.size.y = BND_WIDGET_HEIGHT + 2*margin;

	float xPos = margin;
	{
		Label *label = new Label();
		label->box.pos = Vec(xPos, margin);
		label->text = gApplicationVersion;
		addChild(label);
		xPos += 100;
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
		wireOpacitySlider->label = "Cable opacity";
		wireOpacitySlider->precision = 0;
		wireOpacitySlider->unit = "%";
		wireOpacitySlider->setLimits(0.0, 100.0);
		wireOpacitySlider->setDefaultValue(50.0);
		addChild(wireOpacitySlider);
		xPos += wireOpacitySlider->box.size.x;
	}

	xPos += margin;
	{
		wireTensionSlider = new Slider();
		wireTensionSlider->box.pos = Vec(xPos, margin);
		wireTensionSlider->box.size.x = 150;
		wireTensionSlider->label = "Cable tension";
		// wireTensionSlider->unit = "";
		wireTensionSlider->setLimits(0.0, 1.0);
		wireTensionSlider->setDefaultValue(0.5);
		addChild(wireTensionSlider);
		xPos += wireTensionSlider->box.size.x;
	}

	xPos += margin;
	{
		cpuUsageButton = new RadioButton();
		cpuUsageButton->box.pos = Vec(xPos, margin);
		cpuUsageButton->box.size.x = 100;
		cpuUsageButton->label = "CPU usage";
		addChild(cpuUsageButton);
		xPos += cpuUsageButton->box.size.x;
	}
}

void Toolbar::draw(NVGcontext *vg) {
	bndBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	bndBevel(vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(vg);
}


} // namespace rack
