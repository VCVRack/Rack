#include "global_pre.hpp"
#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
#include "asset.hpp"
#include "global.hpp"
#include "global_ui.hpp"

#ifdef RACK_HOST
extern void vst2_oversample_set (int _factor, int _quality);
extern void vst2_oversample_get (int *_factor, int *_quality);
#endif // RACK_HOST

namespace rack {


struct TooltipIconButton : IconButton {
	Tooltip *tooltip = NULL;
	std::string tooltipText;
	void onMouseEnter(EventMouseEnter &e) override {
		if (!tooltip) {
			tooltip = new Tooltip();
			tooltip->box.pos = getAbsoluteOffset(Vec(0, BND_WIDGET_HEIGHT));
			tooltip->text = tooltipText;
			global_ui->ui.gScene->addChild(tooltip);
		}
		IconButton::onMouseEnter(e);
	}
	void onMouseLeave(EventMouseLeave &e) override {
		if (tooltip) {
			global_ui->ui.gScene->removeChild(tooltip);
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
		global_ui->app.gRackWidget->reset();
	}
};

struct OpenButton : TooltipIconButton {
	OpenButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_31859_cc.svg")));
		tooltipText = "Open  (" WINDOW_MOD_KEY_NAME "+O)";
	}
	void onAction(EventAction &e) override {
		global_ui->app.gRackWidget->openDialog();
	}
};

struct SaveButton : TooltipIconButton {
	SaveButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_1343816_cc.svg")));
		tooltipText = "Save (" WINDOW_MOD_KEY_NAME "+S)";
	}
	void onAction(EventAction &e) override {
		global_ui->app.gRackWidget->saveDialog();
	}
};

struct SaveAsButton : TooltipIconButton {
	SaveAsButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_1343811_cc.svg")));
		tooltipText = "Save as (" WINDOW_MOD_KEY_NAME "+Shift+S)";
	}
	void onAction(EventAction &e) override {
		global_ui->app.gRackWidget->saveAsDialog();
	}
};

struct RevertButton : TooltipIconButton {
	RevertButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_1084369_cc.svg")));
		tooltipText = "Revert";
	}
	void onAction(EventAction &e) override {
		global_ui->app.gRackWidget->revert();
	}
};

struct DisconnectCablesButton : TooltipIconButton {
	DisconnectCablesButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_1745061_cc.svg")));
		tooltipText = "Disconnect cables";
	}
	void onAction(EventAction &e) override {
		global_ui->app.gRackWidget->disconnect();
	}
};

struct PowerMeterButton : TooltipIconButton {
	PowerMeterButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_305536_cc.svg")));
		tooltipText = "Toggle power meter (see manual for explanation)";
	}
	void onAction(EventAction &e) override {
		global->gPowerMeter ^= true;
	}
};

struct EnginePauseItem : MenuItem {
	void onAction(EventAction &e) override {
		global->gPaused ^= true;
	}
};

struct SampleRateItem : MenuItem {
	float sampleRate;
	void onAction(EventAction &e) override {
		engineSetSampleRate(sampleRate);
		global->gPaused = false;
	}
};

#ifdef RACK_HOST
struct OversampleSetting {
   const char *name;
   int factor;
   int quality;
};

static OversampleSetting oversample_settings[] = {
   /*  0 */ { "No oversampling",        1,  0 },
   /*  1 */ { "Oversample x2 (low)",    2,  4 },
   /*  2 */ { "Oversample x2 (medium)", 2,  7 },
   /*  3 */ { "Oversample x2 (high)",   2, 10 },
   /*  4 */ { "Oversample x4 (low)",    4,  4 },
   /*  5 */ { "Oversample x4 (medium)", 4,  7 },
   /*  6 */ { "Oversample x4 (high)",   4, 10 },
   /*  7 */ { "Oversample x6 (low)",    6,  4 },
   /*  8 */ { "Oversample x6 (medium)", 6,  7 },
   /*  9 */ { "Oversample x6 (high)",   6, 10 },
   /* 10 */ { "Oversample x8 (low)",    8,  4 },
   /* 11 */ { "Oversample x8 (medium)", 8,  7 },
   /* 12 */ { "Oversample x8 (high)",   8, 10 },
};
#define NUM_OVERSAMPLE_SETTINGS  (sizeof(oversample_settings) / sizeof(OversampleSetting))

struct OversampleItem : MenuItem {
	const OversampleSetting *setting;

	void onAction(EventAction &e) override {
		vst2_oversample_set(setting->factor, setting->quality);
		global->gPaused = false;
	}
};
#endif // RACK_HOST


struct SampleRateButton : TooltipIconButton {
	SampleRateButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_1240789_cc.svg")));
		tooltipText = "Internal sample rate";
	}
	void onAction(EventAction &e) override {
		Menu *menu = global_ui->ui.gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(MenuLabel::create("Internal sample rate"));

		EnginePauseItem *pauseItem = new EnginePauseItem();
		pauseItem->text = global->gPaused ? "Resume engine" : "Pause engine";
		menu->addChild(pauseItem);

#ifdef USE_VST2
      {
         int factor;
         int quality;
         vst2_oversample_get(&factor, &quality);

         for(unsigned int overIdx = 0u; overIdx < NUM_OVERSAMPLE_SETTINGS; overIdx++)
         {
            const OversampleSetting *overSetting = &oversample_settings[overIdx];

            OversampleItem *item = new OversampleItem();
            item->text = overSetting->name;
            item->rightText = CHECKMARK( (overSetting->factor == factor) && (overSetting->quality == quality) );
            item->setting = overSetting;
            menu->addChild(item);
         }
      }
#else
		std::vector<float> sampleRates = {44100, 48000, 88200, 96000, 176400, 192000};
		for (float sampleRate : sampleRates) {
			SampleRateItem *item = new SampleRateItem();
			item->text = stringf("%.0f Hz", sampleRate);
			item->rightText = CHECKMARK(engineGetSampleRate() == sampleRate);
			item->sampleRate = sampleRate;
			menu->addChild(item);
		}
#endif // USE_VST2
	}
};

struct RackLockButton : TooltipIconButton {
	RackLockButton() {
		setSVG(SVG::load(assetGlobal("res/icons/noun_468341_cc.svg")));
		tooltipText = "Lock modules";
	}
	void onAction(EventAction &e) override {
		global_ui->app.gRackWidget->lockModules ^= true;
	}
};

struct ZoomSlider : Slider {
	void onAction(EventAction &e) override {
		Slider::onAction(e);
		global_ui->app.gRackScene->zoomWidget->setZoom(roundf(value) / 100.0);
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

	layout->addChild(new SampleRateButton());
	layout->addChild(new PowerMeterButton());
	layout->addChild(new RackLockButton());

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

	zoomSlider = new ZoomSlider();
	zoomSlider->box.size.x = 150;
	zoomSlider->precision = 0;
	zoomSlider->label = "Zoom";
	zoomSlider->unit = "%";
	zoomSlider->setLimits(25.0, 200.0);
	zoomSlider->setDefaultValue(100.0);
	layout->addChild(zoomSlider);

	// Kind of hacky, but display the PluginManagerWidget only if the local directory is not the development directory
	if (assetLocal("") != "./") {
		Widget *pluginManager = new PluginManagerWidget();
		layout->addChild(pluginManager);
	}
}

void Toolbar::draw(NVGcontext *vg) {
	bndBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	bndBevel(vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(vg);
}


} // namespace rack
