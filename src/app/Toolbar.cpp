#include "app/Toolbar.hpp"
#include "window.hpp"
#include "engine/Engine.hpp"
#include "asset.hpp"
#include "ui/Tooltip.hpp"
#include "ui/IconButton.hpp"
#include "ui/SequentialLayout.hpp"
#include "ui/Slider.hpp"
#include "app/PluginManagerWidget.hpp"
#include "app/Scene.hpp"
#include "helpers.hpp"


namespace rack {


struct TooltipIconButton : IconButton {
	Tooltip *tooltip = NULL;
	void onEnter(event::Enter &e) override {
		if (!tooltip) {
			tooltip = new Tooltip;
			tooltip->box.pos = getAbsoluteOffset(Vec(0, BND_WIDGET_HEIGHT));
			tooltip->text = getTooltipText();
			gScene->addChild(tooltip);
		}
		IconButton::onEnter(e);
	}
	void onLeave(event::Leave &e) override {
		if (tooltip) {
			gScene->removeChild(tooltip);
			delete tooltip;
			tooltip = NULL;
		}
		IconButton::onLeave(e);
	}
	virtual std::string getTooltipText() {return "";}
};

struct NewButton : TooltipIconButton {
	NewButton() {
		setSVG(SVG::load(asset::system("res/icons/noun_146097_cc.svg")));
	}
	std::string getTooltipText() override {return "New patch (" WINDOW_MOD_KEY_NAME "+N)";}
	void onAction(event::Action &e) override {
		gScene->rackWidget->reset();
	}
};

struct OpenButton : TooltipIconButton {
	OpenButton() {
		setSVG(SVG::load(asset::system("res/icons/noun_31859_cc.svg")));
	}
	std::string getTooltipText() override {return "Open patch (" WINDOW_MOD_KEY_NAME "+O)";}
	void onAction(event::Action &e) override {
		gScene->rackWidget->loadDialog();
	}
};

struct SaveButton : TooltipIconButton {
	SaveButton() {
		setSVG(SVG::load(asset::system("res/icons/noun_1343816_cc.svg")));
	}
	std::string getTooltipText() override {return "Save patch (" WINDOW_MOD_KEY_NAME "+S)";}
	void onAction(event::Action &e) override {
		gScene->rackWidget->saveDialog();
	}
};

struct SaveAsButton : TooltipIconButton {
	SaveAsButton() {
		setSVG(SVG::load(asset::system("res/icons/noun_1343811_cc.svg")));
	}
	std::string getTooltipText() override {return "Save patch as (" WINDOW_MOD_KEY_NAME "+Shift+S)";}
	void onAction(event::Action &e) override {
		gScene->rackWidget->saveAsDialog();
	}
};

struct RevertButton : TooltipIconButton {
	RevertButton() {
		setSVG(SVG::load(asset::system("res/icons/noun_1084369_cc.svg")));
	}
	std::string getTooltipText() override {return "Revert patch";}
	void onAction(event::Action &e) override {
		gScene->rackWidget->revert();
	}
};

struct DisconnectCablesButton : TooltipIconButton {
	DisconnectCablesButton() {
		setSVG(SVG::load(asset::system("res/icons/noun_1745061_cc.svg")));
	}
	std::string getTooltipText() override {return "Disconnect cables";}
	void onAction(event::Action &e) override {
		gScene->rackWidget->disconnect();
	}
};

struct PowerMeterButton : TooltipIconButton {
	PowerMeterButton() {
		setSVG(SVG::load(asset::system("res/icons/noun_305536_cc.svg")));
	}
	std::string getTooltipText() override {return "Toggle power meter (see manual for explanation)";}
	void onAction(event::Action &e) override {
		gEngine->powerMeter ^= true;
	}
};

struct EnginePauseItem : MenuItem {
	void onAction(event::Action &e) override {
		gEngine->paused ^= true;
	}
};

struct SampleRateItem : MenuItem {
	float sampleRate;
	void onAction(event::Action &e) override {
		gEngine->setSampleRate(sampleRate);
		gEngine->paused = false;
	}
};

struct SampleRateButton : TooltipIconButton {
	SampleRateButton() {
		setSVG(SVG::load(asset::system("res/icons/noun_1240789_cc.svg")));
	}
	std::string getTooltipText() override {return "Engine sample rate";}
	void onAction(event::Action &e) override {
		Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(createMenuLabel("Engine sample rate"));

		EnginePauseItem *pauseItem = new EnginePauseItem;
		pauseItem->text = gEngine->paused ? "Resume engine" : "Pause engine";
		menu->addChild(pauseItem);

		std::vector<float> sampleRates = {44100, 48000, 88200, 96000, 176400, 192000};
		for (float sampleRate : sampleRates) {
			SampleRateItem *item = new SampleRateItem;
			item->text = string::f("%.0f Hz", sampleRate);
			item->rightText = CHECKMARK(gEngine->getSampleRate() == sampleRate);
			item->sampleRate = sampleRate;
			menu->addChild(item);
		}
	}
};

struct RackLockButton : TooltipIconButton {
	RackLockButton() {
		setSVG(SVG::load(asset::system("res/icons/noun_468341_cc.svg")));
	}
	std::string getTooltipText() override {return "Lock modules";}
	void onAction(event::Action &e) override {
		gScene->rackWidget->lockModules ^= true;
	}
};

struct WireOpacityQuantity : Quantity {
	void setValue(float value) override {
		// TODO
	}
	float getValue() override {
		return 0;
	}
	float getDefaultValue() override {return 0.5;}
	std::string getLabel() override {return "Cable opacity";}
	int getDisplayPrecision() override {return 0;}
};


struct WireTensionQuantity : Quantity {
	void setValue(float value) override {
		// TODO
	}
	float getValue() override {
		return 0;
	}
	float getDefaultValue() override {return 0.5;}
	std::string getLabel() override {return "Cable tension";}
	int getDisplayPrecision() override {return 0;}
};


struct ZoomQuantity : Quantity {
	void setValue(float value) override {
		gScene->zoomWidget->setZoom(std::round(value) / 100);
	}
	float getValue() override {
		return gScene->zoomWidget->zoom * 100;
	}
	float getMinValue() override {return 25;}
	float getMaxValue() override {return 200;}
	float getDefaultValue() override {return 100;}
	std::string getLabel() override {return "Zoom";}
	std::string getUnit() override {return "%";}
	int getDisplayPrecision() override {return 0;}
};


Toolbar::Toolbar() {
	box.size.y = BND_WIDGET_HEIGHT + 2*5;

	SequentialLayout *layout = new SequentialLayout;
	layout->box.pos = Vec(5, 5);
	layout->spacing = 5;
	addChild(layout);

	layout->addChild(new NewButton);
	layout->addChild(new OpenButton);
	layout->addChild(new SaveButton);
	layout->addChild(new SaveAsButton);
	layout->addChild(new RevertButton);
	layout->addChild(new DisconnectCablesButton);

	layout->addChild(new SampleRateButton);
	layout->addChild(new PowerMeterButton);
	layout->addChild(new RackLockButton);

	Slider *wireOpacitySlider = new Slider;
	WireOpacityQuantity *wireOpacityQuantity = new WireOpacityQuantity;
	wireOpacitySlider->quantity = wireOpacityQuantity;
	wireOpacitySlider->box.size.x = 150;
	layout->addChild(wireOpacitySlider);

	Slider *wireTensionSlider = new Slider;
	WireTensionQuantity *wireTensionQuantity = new WireTensionQuantity;
	wireTensionSlider->quantity = wireTensionQuantity;
	wireTensionSlider->box.size.x = 150;
	layout->addChild(wireTensionSlider);

	Slider *zoomSlider = new Slider;
	ZoomQuantity *zoomQuantity = new ZoomQuantity;
	zoomSlider->quantity = zoomQuantity;
	zoomSlider->box.size.x = 150;
	layout->addChild(zoomSlider);

	// Kind of hacky, but display the PluginManagerWidget only if the user directory is not the development directory
	if (asset::user("") != "./") {
		Widget *pluginManager = new PluginManagerWidget;
		layout->addChild(pluginManager);
	}
}

void Toolbar::draw(NVGcontext *vg) {
	bndBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	bndBevel(vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(vg);
}


} // namespace rack
