#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
#include "asset.hpp"


namespace rack {


struct TooltipIconButton : IconButton {
	std::string tooltipText;
	void onMouseEnter(EventMouseEnter &e) override {
		TooltipOverlay *overlay = new TooltipOverlay();
		Tooltip *tooltip = new Tooltip();
		tooltip->box.pos = getAbsoluteOffset(Vec(0, BND_WIDGET_HEIGHT));
		tooltip->text = tooltipText;
		overlay->addChild(tooltip);
		gScene->setOverlay(overlay);
	}
	void onMouseLeave(EventMouseLeave &e) override {
		gScene->setOverlay(NULL);
	}
};

struct NewButton : TooltipIconButton {
	NewButton() {
		setSVG(SVG::load(assetGlobal("res/icons/037-file-empty.svg")));
		tooltipText = "New patch (" WINDOW_MOD_KEY_NAME "+N)";
	}
	void onAction(EventAction &e) override {
		gRackWidget->reset();
	}
};

struct OpenButton : TooltipIconButton {
	OpenButton() {
		setSVG(SVG::load(assetGlobal("res/icons/049-folder-open.svg")));
		tooltipText = "Open patch (" WINDOW_MOD_KEY_NAME "+O)";
	}
	void onAction(EventAction &e) override {
		gRackWidget->openDialog();
	}
};

struct SaveButton : TooltipIconButton {
	SaveButton() {
		setSVG(SVG::load(assetGlobal("res/icons/099-floppy-disk.svg")));
		tooltipText = "Save patch (" WINDOW_MOD_KEY_NAME "+S)";
	}
	void onAction(EventAction &e) override {
		gRackWidget->saveDialog();
	}
};

struct MeterButton : TooltipIconButton {
	MeterButton() {
		setSVG(SVG::load(assetGlobal("res/icons/167-meter.svg")));
		tooltipText = "Toggle CPU meter\nSee manual for mV definition";
	}
	void onAction(EventAction &e) override {
		gCpuMeters ^= true;
	}
};

struct EnginePauseItem : MenuItem {
	void onAction(EventAction &e) override {
		gPaused ^= true;
	}
};

struct SampleRateItem : MenuItem {
	float sampleRate;
	void onAction(EventAction &e) override {
		engineSetSampleRate(sampleRate);
		gPaused = false;
	}
};

struct SampleRateButton : TooltipIconButton {
	SampleRateButton() {
		setSVG(SVG::load(assetGlobal("res/icons/030-feed.svg")));
		tooltipText = "Internal sample rate";
	}
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		EnginePauseItem *pauseItem = new EnginePauseItem();
		pauseItem->text = gPaused ? "Resume engine" : "Pause engine";
		menu->addChild(pauseItem);

		std::vector<float> sampleRates = {44100, 48000, 88200, 96000, 176400, 192000};
		for (float sampleRate : sampleRates) {
			SampleRateItem *item = new SampleRateItem();
			item->text = stringf("%.0f Hz", sampleRate);
			item->rightText = CHECKMARK(engineGetSampleRate() == sampleRate);
			item->sampleRate = sampleRate;
			menu->addChild(item);
		}
	}
};



struct DisconnectItem : MenuItem {
	void onAction(EventAction &e) override {
		gRackWidget->disconnect();
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

struct FileChoice : ChoiceButton {
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(MenuItem::create<DisconnectItem>("Disconnect cables"));
		menu->addChild(MenuItem::create<SaveAsItem>("Save as", WINDOW_MOD_KEY_NAME "+Shift+S"));
		menu->addChild(MenuItem::create<RevertItem>("Revert"));
	}
};



Toolbar::Toolbar() {
	box.size.y = BND_WIDGET_HEIGHT + 2*5;

	SequentialLayout *layout = new SequentialLayout();
	layout->box.pos = Vec(5, 5);
	layout->spacing = 5;
	addChild(layout);

	layout->addChild(new NewButton());
	layout->addChild(new OpenButton());
	layout->addChild(new SaveButton());
	layout->addChild(new SampleRateButton());
	layout->addChild(new MeterButton());

	ChoiceButton *fileChoice = new FileChoice();
	fileChoice->box.size.x = 100;
	fileChoice->text = "File";
	layout->addChild(fileChoice);

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
