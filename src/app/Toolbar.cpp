#include "app.hpp"
#include "gui.hpp"
#include "engine.hpp"


namespace rack {


struct NewItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->reset();
	}
};

struct OpenItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->openDialog();
	}
};

struct SaveItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->saveDialog();
	}
};

struct SaveAsItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->saveAsDialog();
	}
};

struct QuitItem : MenuItem {
	void onAction(EventAction &e) override {
		guiClose();
	}
};

struct FileChoice : ChoiceButton {
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		{
			menu->addChild(construct<NewItem>(&MenuItem::text, "New", &MenuItem::rightText, GUI_MOD_KEY_NAME "+N"));
			menu->addChild(construct<OpenItem>(&MenuItem::text, "Open", &MenuItem::rightText, GUI_MOD_KEY_NAME "+O"));
			menu->addChild(construct<SaveItem>(&MenuItem::text, "Save", &MenuItem::rightText, GUI_MOD_KEY_NAME "+S"));
			menu->addChild(construct<SaveAsItem>(&MenuItem::text, "Save as", &MenuItem::rightText, GUI_MOD_KEY_NAME "+Shift+S"));
			menu->addChild(construct<QuitItem>(&MenuItem::text, "Quit", &MenuItem::rightText, GUI_MOD_KEY_NAME "+Q"));
		}
	}
};


struct EnginePauseItem : MenuItem {
	void onAction(EventAction &e) override {
		gPaused = !gPaused;
	}
};

struct EngineSampleRateItem : MenuItem {
	float sampleRate;
	void onAction(EventAction &e) override {
		engineSetSampleRate(sampleRate);
		gPaused = false;
	}
};

struct EngineSampleRateChoice : ChoiceButton {
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		EnginePauseItem *pauseItem = new EnginePauseItem();
		pauseItem->text = gPaused ? "Resume engine" : "Pause engine";
		menu->addChild(pauseItem);

		std::vector<float> sampleRates = {44100, 48000, 88200, 96000, 176400, 192000};
		for (float sampleRate : sampleRates) {
			EngineSampleRateItem *item = new EngineSampleRateItem();
			item->text = stringf("%.0f Hz", sampleRate);
			item->sampleRate = sampleRate;
			menu->addChild(item);
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
		EngineSampleRateChoice *srChoice = new EngineSampleRateChoice();
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
			void onAction(EventAction &e) override {
				Slider::onAction(e);
				gRackScene->zoomWidget->setZoom(roundf(value) / 100.0);
			}
		};
		zoomSlider = new ZoomSlider();
		zoomSlider->box.pos = Vec(xPos, margin);
		zoomSlider->box.size.x = 150;
		zoomSlider->precision = 0;
		zoomSlider->label = "Zoom";
		zoomSlider->unit = "%";
		zoomSlider->setLimits(25.0, 200.0);
		zoomSlider->setDefaultValue(100.0);
		addChild(zoomSlider);
		xPos += zoomSlider->box.size.x;
	}
	xPos += margin;

	/*
	{
		cpuUsageButton = new RadioButton();
		cpuUsageButton->box.pos = Vec(xPos, margin);
		cpuUsageButton->box.size.x = 100;
		cpuUsageButton->label = "CPU usage";
		addChild(cpuUsageButton);
		xPos += cpuUsageButton->box.size.x;
	}
	xPos += margin;
	*/

#if defined(RELEASE)
	{
		Widget *pluginManager = new PluginManagerWidget();
		pluginManager->box.pos = Vec(xPos, margin);
		addChild(pluginManager);
		xPos += pluginManager->box.size.x;
	}
#endif
}

void Toolbar::draw(NVGcontext *vg) {
	bndBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	bndBevel(vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(vg);
}


} // namespace rack
