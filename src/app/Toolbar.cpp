#include "global_pre.hpp"
#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
#include "asset.hpp"
#include "global.hpp"
#include "global_ui.hpp"

#ifdef RACK_HOST
#define Dfltequal(a, b)  ( (((a)-(b)) < 0.0f) ? (((a)-(b)) > -0.0001f) : (((a)-(b)) < 0.0001f) )
extern void vst2_oversample_realtime_set (float _factor, int _quality);
extern void vst2_oversample_realtime_get (float *_factor, int *_quality);
extern void vst2_oversample_channels_set (int _numIn, int _numOut);
extern void vst2_oversample_channels_get (int *_numIn, int *_numOut);
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
   float factor;
   int quality;
};

static OversampleSetting oversample_settings[] = {
   /*  0 */ { "No resampling",            1.0f,  0 },
   // /*  1 */ { "Undersample /6 (low)",     0.166666666667f,  4 },
   // /*  2 */ { "Undersample /6 (medium)",  0.166666666667f,  7 },
   // /*  3 */ { "Undersample /6 (high)",    0.166666666667f, 10 },
   /*  4 */ { "Undersample /4 (low)",     0.25f,  4 },
   /*  5 */ { "Undersample /4 (medium)",  0.25f,  7 },
   /*  6 */ { "Undersample /4 (high)",    0.25f, 10 },
   /*  7 */ { "Undersample /2 (low)",     0.5f,  4 },
   /*  8 */ { "Undersample /2 (medium)",  0.5f,  7 },
   /*  9 */ { "Undersample /2 (high)",    0.5f, 10 },
   /* 10 */ { "Oversample x2 (low)",      2.0f,  4 },
   /* 11 */ { "Oversample x2 (medium)",   2.0f,  7 },
   /* 12 */ { "Oversample x2 (high)",     2.0f, 10 },
   /* 13 */ { "Oversample x4 (low)",      4.0f,  4 },
   /* 14 */ { "Oversample x4 (medium)",   4.0f,  7 },
   /* 15 */ { "Oversample x4 (high)",     4.0f, 10 },
   /* 16 */ { "Oversample x6 (low)",      6.0f,  4 },
   /* 17 */ { "Oversample x6 (medium)",   6.0f,  7 },
   /* 18 */ { "Oversample x6 (high)",     6.0f, 10 },
   /* 19 */ { "Oversample x8 (low)",      8.0f,  4 },
   /* 20 */ { "Oversample x8 (medium)",   8.0f,  7 },
   /* 21 */ { "Oversample x8 (high)",     8.0f, 10 },
   /* 22 */ { "Oversample x12 (low)",    12.0f,  4 },
   /* 23 */ { "Oversample x12 (medium)", 12.0f,  7 },
   /* 24 */ { "Oversample x12 (high)",   12.0f, 10 },
   /* 25 */ { "Oversample x16 (low)",    16.0f,  4 },
   /* 26 */ { "Oversample x16 (medium)", 16.0f,  7 },
   /* 27 */ { "Oversample x16 (high)",   16.0f, 10 },
};
#define NUM_OVERSAMPLE_SETTINGS  (sizeof(oversample_settings) / sizeof(OversampleSetting))

struct OversampleItem : MenuItem {
	const OversampleSetting *setting;

	void onAction(EventAction &e) override {
		vst2_oversample_realtime_set(setting->factor, setting->quality);
		global->gPaused = false;
	}
};

struct OversampleChannelSetting {
   const char *name;
   int num_in;
   int num_out;
};

static OversampleChannelSetting oversample_channel_settings[] = {
   /*  0 */ { "Oversample: 0 in, 1 out",  0,  1 },
   /*  1 */ { "Oversample: 0 in, 2 out",  0,  2 },
   /*  2 */ { "Oversample: 0 in, 4 out",  0,  4 },
   /*  3 */ { "Oversample: 0 in, 8 out",  0,  8 },
   /*  4 */ { "Oversample: 2 in, 2 out",  2,  2 },
   /*  5 */ { "Oversample: 2 in, 4 out",  2,  4 },
   /*  6 */ { "Oversample: 4 in, 8 out",  4,  8 },
   /*  7 */ { "Oversample: 8 in, 8 out",  8,  8 },
};
#define NUM_OVERSAMPLE_CHANNEL_SETTINGS  (sizeof(oversample_channel_settings) / sizeof(OversampleChannelSetting))

struct OversampleChannelItem : MenuItem {
	const OversampleChannelSetting *setting;

	void onAction(EventAction &e) override {
		vst2_oversample_channels_set(setting->num_in, setting->num_out);
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

#if defined(USE_VST2) && defined(RACK_HOST)
      {
         int numIn;
         int numOut;
         vst2_oversample_channels_get(&numIn, &numOut);

         for(unsigned int overIdx = 0u; overIdx < NUM_OVERSAMPLE_CHANNEL_SETTINGS; overIdx++)
         {
            const OversampleChannelSetting *setting = &oversample_channel_settings[overIdx];

            OversampleChannelItem *item = new OversampleChannelItem();
            item->text = setting->name;
            item->rightText = CHECKMARK( (setting->num_in == numIn) && (setting->num_out == numOut) );
            item->setting = setting;
            menu->addChild(item);
         }
      }
      {
         float factor;
         int quality;
         vst2_oversample_realtime_get(&factor, &quality);

         for(unsigned int overIdx = 0u; overIdx < NUM_OVERSAMPLE_SETTINGS; overIdx++)
         {
            const OversampleSetting *setting = &oversample_settings[overIdx];

            OversampleItem *item = new OversampleItem();
            item->text = setting->name;
            item->rightText = CHECKMARK( Dfltequal(setting->factor, factor) && (setting->quality == quality) );
            item->setting = setting;
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
#endif // USE_VST2 && RACK_HOST
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
