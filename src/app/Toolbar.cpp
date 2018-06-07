#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
#include "asset.hpp"


namespace rack {


struct TooltipIconButton : IconButton {
	Tooltip *tooltip = NULL;
	std::string tooltipText;
	void onMouseEnter(EventMouseEnter &e) override {
		if (!tooltip) {
			tooltip = new Tooltip();
			tooltip->box.pos = getAbsoluteOffset(Vec(0, BND_WIDGET_HEIGHT));
			tooltip->text = tooltipText;
			gScene->addChild(tooltip);
		}
		IconButton::onMouseEnter(e);
	}
	void onMouseLeave(EventMouseLeave &e) override {
		if (tooltip) {
			gScene->removeChild(tooltip);
			delete tooltip;
			tooltip = NULL;
		}
		IconButton::onMouseLeave(e);
	}
};

struct NewButton : TooltipIconButton {
	NewButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_146097_cc.svg")));
		tooltipText = "New (" WINDOW_MOD_KEY_NAME "+N)";
	}
	void onAction(EventAction &e) override {
		gRackWidget->reset();
	}
};

struct OpenButton : TooltipIconButton {
	OpenButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_31859_cc.svg")));
		tooltipText = "Open  (" WINDOW_MOD_KEY_NAME "+O)";
	}
	void onAction(EventAction &e) override {
		gRackWidget->openDialog();
	}
};

struct SaveButton : TooltipIconButton {
	SaveButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_1343816_cc.svg")));
		tooltipText = "Save (" WINDOW_MOD_KEY_NAME "+S)";
	}
	void onAction(EventAction &e) override {
		gRackWidget->saveDialog();
	}
};

struct SaveAsButton : TooltipIconButton {
	SaveAsButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_1343811_cc.svg")));
		tooltipText = "Save as (" WINDOW_MOD_KEY_NAME "+Shift+S)";
	}
	void onAction(EventAction &e) override {
		gRackWidget->saveAsDialog();
	}
};

struct RevertButton : TooltipIconButton {
	RevertButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_1084369_cc.svg")));
		tooltipText = "Revert";
	}
	void onAction(EventAction &e) override {
		gRackWidget->revert();
	}
};

struct DisconnectCablesButton : TooltipIconButton {
	DisconnectCablesButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_1745061_cc.svg")));
		tooltipText = "Disconnect cables";
	}
	void onAction(EventAction &e) override {
		gRackWidget->disconnect();
	}
};

struct PowerMeterButton : TooltipIconButton {
	PowerMeterButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_305536_cc.svg")));
		tooltipText = "Toggle power meter (see manual for explanation)";
	}
	void onAction(EventAction &e) override {
		gPowerMeter ^= true;
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
		setSVG(SVG::load(assetGlobal("res/icons/noun_1240789_cc.svg")));
		tooltipText = "Internal sample rate";
	}
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(MenuLabel::create("Internal sample rate"));

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



Toolbar::Toolbar() {
	box.size.y = BND_WIDGET_HEIGHT + 2*5;

	SequentialLayout *layout = new SequentialLayout();
	layout->box.pos = Vec(5, 5);
	layout->spacing = 5;
	addChild(layout);

	layout->addChild(new NewButton());
	layout->addChild(new OpenButton());
	layout->addChild(new SaveButton());
	layout->addChild(new SaveAsButton());
	layout->addChild(new RevertButton());
	layout->addChild(new DisconnectCablesButton());

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

	layout->addChild(new SampleRateButton());
	layout->addChild(new PowerMeterButton());

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
