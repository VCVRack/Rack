#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"


namespace rack {


struct NewItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->reset();
	}
};

struct DisconnectItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->disconnect();
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

struct RevertItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->revert();
	}
};

struct QuitItem : MenuItem {
	void onAction(EventAction &e) override {
		windowClose();
	}
};

struct FileChoice : ChoiceButton {
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(MenuItem::create<NewItem>("New", WINDOW_MOD_KEY_NAME "+N"));
		menu->addChild(MenuItem::create<DisconnectItem>("Disconnect cables"));
		menu->addChild(MenuItem::create<OpenItem>("Open", WINDOW_MOD_KEY_NAME "+O"));
		menu->addChild(MenuItem::create<SaveItem>("Save", WINDOW_MOD_KEY_NAME "+S"));
		menu->addChild(MenuItem::create<SaveAsItem>("Save as", WINDOW_MOD_KEY_NAME "+Shift+S"));
		menu->addChild(MenuItem::create<RevertItem>("Revert"));
		menu->addChild(MenuItem::create<QuitItem>("Quit", WINDOW_MOD_KEY_NAME "+Q"));
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
	box.size.y = BND_WIDGET_HEIGHT + 2*5;

	SequentialLayout *layout = new SequentialLayout();
	layout->box.pos = Vec(5, 5);
	layout->spacing = 5;
	addChild(layout);

	ChoiceButton *fileChoice = new FileChoice();
	fileChoice->box.size.x = 100;
	fileChoice->text = "File";
	layout->addChild(fileChoice);

	EngineSampleRateChoice *srChoice = new EngineSampleRateChoice();
	srChoice->box.size.x = 100;
	layout->addChild(srChoice);

	wireOpacitySlider = new Slider();
	wireOpacitySlider->box.size.x = 150;
	wireOpacitySlider->label = "Cable opacity";
	wireOpacitySlider->precision = 0;
	wireOpacitySlider->unit = "%";
	wireOpacitySlider->setLimits(0.0, 100.0);
	wireOpacitySlider->setDefaultValue(50.0);
	layout->addChild(wireOpacitySlider);

	wireTensionSlider = new Slider();
	wireTensionSlider->box.size.x = 150;
	wireTensionSlider->label = "Cable tension";
	wireTensionSlider->unit = "";
	wireTensionSlider->setLimits(0.0, 1.0);
	wireTensionSlider->setDefaultValue(0.5);
	layout->addChild(wireTensionSlider);

	struct ZoomSlider : Slider {
		void onAction(EventAction &e) override {
			Slider::onAction(e);
			gRackScene->zoomWidget->setZoom(roundf(value) / 100.0);
		}
	};
	zoomSlider = new ZoomSlider();
	zoomSlider->box.size.x = 150;
	zoomSlider->precision = 0;
	zoomSlider->label = "Zoom";
	zoomSlider->unit = "%";
	zoomSlider->setLimits(25.0, 200.0);
	zoomSlider->setDefaultValue(100.0);
	layout->addChild(zoomSlider);

/*
	cpuUsageButton = new RadioButton();
	cpuUsageButton->box.size.x = 100;
	cpuUsageButton->label = "CPU usage";
	layout->addChild(cpuUsageButton);
*/

#if defined(RELEASE)
	Widget *pluginManager = new PluginManagerWidget();
	layout->addChild(pluginManager);
#endif
}

void Toolbar::draw(NVGcontext *vg) {
	bndBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	bndBevel(vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(vg);
}


} // namespace rack
