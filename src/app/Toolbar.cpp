#include "app.hpp"
#include "gui.hpp"
#include "engine.hpp"


namespace rack {


struct NewItem : MenuItem {
	void onAction() override {
		gRackWidget->reset();
	}
};

struct OpenItem : MenuItem {
	void onAction() override {
		gRackWidget->openDialog();
	}
};

struct SaveItem : MenuItem {
	void onAction() override {
		gRackWidget->saveDialog();
	}
};

struct SaveAsItem : MenuItem {
	void onAction() override {
		gRackWidget->saveAsDialog();
	}
};

struct QuitItem : MenuItem {
	void onAction() override {
		guiClose();
	}
};

struct FileChoice : ChoiceButton {
	void onAction() override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		{
			menu->pushChild(construct<NewItem>(&MenuItem::text, "New", &MenuItem::rightText, GUI_MOD_KEY_NAME "+N"));
			menu->pushChild(construct<OpenItem>(&MenuItem::text, "Open", &MenuItem::rightText, GUI_MOD_KEY_NAME "+O"));
			menu->pushChild(construct<SaveItem>(&MenuItem::text, "Save", &MenuItem::rightText, GUI_MOD_KEY_NAME "+S"));
			menu->pushChild(construct<SaveAsItem>(&MenuItem::text, "Save as", &MenuItem::rightText, GUI_MOD_KEY_NAME "+Shift+S"));
			menu->pushChild(construct<QuitItem>(&MenuItem::text, "Quit", &MenuItem::rightText, GUI_MOD_KEY_NAME "+Q"));
		}
	}
};


struct PauseItem : MenuItem {
	void onAction() override {
		gPaused = !gPaused;
	}
};

struct SampleRateItem : MenuItem {
	float sampleRate;
	void onAction() override {
		engineSetSampleRate(sampleRate);
		gPaused = false;
	}
};

struct SampleRateChoice : ChoiceButton {
	void onAction() override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		PauseItem *pauseItem = new PauseItem();
		pauseItem->text = gPaused ? "Resume engine" : "Pause engine";
		menu->pushChild(pauseItem);

		float sampleRates[] = {44100, 48000, 88200, 96000, 176400, 192000};
		int sampleRatesLen = sizeof(sampleRates) / sizeof(sampleRates[0]);
		for (int i = 0; i < sampleRatesLen; i++) {
			SampleRateItem *item = new SampleRateItem();
			item->text = stringf("%.0f Hz", sampleRates[i]);
			item->sampleRate = sampleRates[i];
			menu->pushChild(item);
		}
	}
	void step() override {
		if (gPaused)
			text = "Paused";
		else
			text = stringf("%.0f Hz", engineGetSampleRate());
	}
};


Toolbar::Toolbar() {
	float margin = 5;
	box.size.y = BND_WIDGET_HEIGHT + 2*margin;
	float xPos = 0;

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
		wireTensionSlider->unit = "";
		wireTensionSlider->setLimits(0.0, 1.0);
		wireTensionSlider->setDefaultValue(0.5);
		addChild(wireTensionSlider);
		xPos += wireTensionSlider->box.size.x;
	}

	xPos += margin;
	{
		struct ZoomSlider : Slider {
			void onAction() override {
				Slider::onAction();
				gRackScene->zoomWidget->setZoom(value / 100.0);
			}
		};
		zoomSlider = new ZoomSlider();
		zoomSlider->box.pos = Vec(xPos, margin);
		zoomSlider->box.size.x = 150;
		zoomSlider->label = "Zoom";
		zoomSlider->unit = "%";
		zoomSlider->setLimits(25.0, 200.0);
		zoomSlider->setDefaultValue(100.0);
		addChild(zoomSlider);
		xPos += zoomSlider->box.size.x;
	}

	xPos += margin;
	{
		plugLightButton = new RadioButton();
		plugLightButton->box.pos = Vec(xPos, margin);
		plugLightButton->box.size.x = 100;
		plugLightButton->label = "Plug lights";
		addChild(plugLightButton);
		xPos += plugLightButton->box.size.x;
	}

	/*
	xPos += margin;
	{
		cpuUsageButton = new RadioButton();
		cpuUsageButton->box.pos = Vec(xPos, margin);
		cpuUsageButton->box.size.x = 100;
		cpuUsageButton->label = "CPU usage";
		addChild(cpuUsageButton);
		xPos += cpuUsageButton->box.size.x;
	}
	*/

	xPos += margin;
	{
		Widget *pluginManager = new PluginManagerWidget();
		pluginManager->box.pos = Vec(xPos, margin);
		addChild(pluginManager);
		xPos += pluginManager->box.size.x;
	}
}

void Toolbar::draw(NVGcontext *vg) {
	bndBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	bndBevel(vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(vg);
}


} // namespace rack
