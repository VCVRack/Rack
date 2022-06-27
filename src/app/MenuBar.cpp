#include <thread>
#include <utility>

#include <osdialog.h>

#include <app/MenuBar.hpp>
#include <app/TipWindow.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Button.hpp>
#include <ui/MenuItem.hpp>
#include <ui/MenuSeparator.hpp>
#include <ui/SequentialLayout.hpp>
#include <ui/Slider.hpp>
#include <ui/TextField.hpp>
#include <ui/ProgressBar.hpp>
#include <ui/Label.hpp>
#include <engine/Engine.hpp>
#include <window/Window.hpp>
#include <asset.hpp>
#include <context.hpp>
#include <settings.hpp>
#include <helpers.hpp>
#include <system.hpp>
#include <plugin.hpp>
#include <patch.hpp>
#include <library.hpp>


namespace rack {
namespace app {
namespace menuBar {


struct MenuButton : ui::Button {
	void step() override {
		box.size.x = bndLabelWidth(APP->window->vg, -1, text.c_str()) + 1.0;
		Widget::step();
	}
	void draw(const DrawArgs& args) override {
		BNDwidgetState state = BND_DEFAULT;
		if (APP->event->hoveredWidget == this)
			state = BND_HOVER;
		if (APP->event->draggedWidget == this)
			state = BND_ACTIVE;
		bndMenuItem(args.vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());
		Widget::draw(args);
	}
};


struct NotificationIcon : widget::Widget {
	void draw(const DrawArgs& args) override {
		nvgBeginPath(args.vg);
		float radius = 4;
		nvgCircle(args.vg, radius, radius, radius);
		nvgFillColor(args.vg, nvgRGBf(1.0, 0.0, 0.0));
		nvgFill(args.vg);
		nvgStrokeColor(args.vg, nvgRGBf(0.5, 0.0, 0.0));
		nvgStroke(args.vg);
	}
};


////////////////////
// File
////////////////////


struct FileButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		menu->addChild(createMenuItem("New", RACK_MOD_CTRL_NAME "+N", []() {
			APP->patch->loadTemplateDialog();
		}));

		menu->addChild(createMenuItem("Open", RACK_MOD_CTRL_NAME "+O", []() {
			APP->patch->loadDialog();
		}));

		menu->addChild(createSubmenuItem("Open recent", "", [](ui::Menu* menu) {
			for (const std::string& path : settings::recentPatchPaths) {
				std::string name = system::getStem(path);
				menu->addChild(createMenuItem(name, "", [=]() {
					APP->patch->loadPathDialog(path);
				}));
			}
		}, settings::recentPatchPaths.empty()));

		menu->addChild(createMenuItem("Save", RACK_MOD_CTRL_NAME "+S", []() {
			APP->patch->saveDialog();
		}));

		menu->addChild(createMenuItem("Save as", RACK_MOD_CTRL_NAME "+Shift+S", []() {
			APP->patch->saveAsDialog();
		}));

		menu->addChild(createMenuItem("Save a copy", "", []() {
			APP->patch->saveAsDialog(false);
		}));

		menu->addChild(createMenuItem("Revert", RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME "+O", []() {
			APP->patch->revertDialog();
		}, APP->patch->path == ""));

		menu->addChild(createMenuItem("Overwrite template", "", []() {
			APP->patch->saveTemplateDialog();
		}));

		menu->addChild(new ui::MenuSeparator);

		// Load selection
		menu->addChild(createMenuItem("Import selection", "", [=]() {
			APP->scene->rack->loadSelectionDialog();
		}, false, true));

		menu->addChild(new ui::MenuSeparator);

		menu->addChild(createMenuItem("Quit", RACK_MOD_CTRL_NAME "+Q", []() {
			APP->window->close();
		}));
	}
};


////////////////////
// Edit
////////////////////


struct EditButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		struct UndoItem : ui::MenuItem {
			void step() override {
				text = "Undo " + APP->history->getUndoName();
				disabled = !APP->history->canUndo();
				MenuItem::step();
			}
			void onAction(const ActionEvent& e) override {
				APP->history->undo();
			}
		};
		menu->addChild(createMenuItem<UndoItem>("", RACK_MOD_CTRL_NAME "+Z"));

		struct RedoItem : ui::MenuItem {
			void step() override {
				text = "Redo " + APP->history->getRedoName();
				disabled = !APP->history->canRedo();
				MenuItem::step();
			}
			void onAction(const ActionEvent& e) override {
				APP->history->redo();
			}
		};
		menu->addChild(createMenuItem<RedoItem>("", RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME "+Z"));

		menu->addChild(createMenuItem("Clear cables", "", [=]() {
			APP->patch->disconnectDialog();
		}));

		menu->addChild(new ui::MenuSeparator);

		APP->scene->rack->appendSelectionContextMenu(menu);
	}
};


////////////////////
// View
////////////////////


struct ZoomQuantity : Quantity {
	void setValue(float value) override {
		APP->scene->rackScroll->setZoom(std::pow(2.f, value));
	}
	float getValue() override {
		return std::log2(APP->scene->rackScroll->getZoom());
	}
	float getMinValue() override {
		return -2.f;
	}
	float getMaxValue() override {
		return 2.f;
	}
	float getDefaultValue() override {
		return 0.0;
	}
	float getDisplayValue() override {
		return std::round(std::pow(2.f, getValue()) * 100);
	}
	void setDisplayValue(float displayValue) override {
		setValue(std::log2(displayValue / 100));
	}
	std::string getLabel() override {
		return "Zoom";
	}
	std::string getUnit() override {
		return "%";
	}
};
struct ZoomSlider : ui::Slider {
	ZoomSlider() {
		quantity = new ZoomQuantity;
	}
	~ZoomSlider() {
		delete quantity;
	}
};


struct CableOpacityQuantity : Quantity {
	void setValue(float value) override {
		settings::cableOpacity = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::cableOpacity;
	}
	float getDefaultValue() override {
		return 0.5;
	}
	float getDisplayValue() override {
		return getValue() * 100;
	}
	void setDisplayValue(float displayValue) override {
		setValue(displayValue / 100);
	}
	std::string getLabel() override {
		return "Cable opacity";
	}
	std::string getUnit() override {
		return "%";
	}
};
struct CableOpacitySlider : ui::Slider {
	CableOpacitySlider() {
		quantity = new CableOpacityQuantity;
	}
	~CableOpacitySlider() {
		delete quantity;
	}
};


struct CableTensionQuantity : Quantity {
	void setValue(float value) override {
		settings::cableTension = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::cableTension;
	}
	float getDefaultValue() override {
		return 0.5;
	}
	float getDisplayValue() override {
		return getValue() * 100;
	}
	void setDisplayValue(float displayValue) override {
		setValue(displayValue / 100);
	}
	std::string getLabel() override {
		return "Cable tension";
	}
	std::string getUnit() override {
		return "%";
	}
};
struct CableTensionSlider : ui::Slider {
	CableTensionSlider() {
		quantity = new CableTensionQuantity;
	}
	~CableTensionSlider() {
		delete quantity;
	}
};


struct RackBrightnessQuantity : Quantity {
	void setValue(float value) override {
		settings::rackBrightness = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::rackBrightness;
	}
	float getDefaultValue() override {
		return 1.0;
	}
	float getDisplayValue() override {
		return getValue() * 100;
	}
	void setDisplayValue(float displayValue) override {
		setValue(displayValue / 100);
	}
	std::string getUnit() override {
		return "%";
	}
	std::string getLabel() override {
		return "Room brightness";
	}
	int getDisplayPrecision() override {
		return 3;
	}
};
struct RackBrightnessSlider : ui::Slider {
	RackBrightnessSlider() {
		quantity = new RackBrightnessQuantity;
	}
	~RackBrightnessSlider() {
		delete quantity;
	}
};


struct HaloBrightnessQuantity : Quantity {
	void setValue(float value) override {
		settings::haloBrightness = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::haloBrightness;
	}
	float getDefaultValue() override {
		return 0.25;
	}
	float getDisplayValue() override {
		return getValue() * 100;
	}
	void setDisplayValue(float displayValue) override {
		setValue(displayValue / 100);
	}
	std::string getUnit() override {
		return "%";
	}
	std::string getLabel() override {
		return "Light bloom";
	}
	int getDisplayPrecision() override {
		return 3;
	}
};
struct HaloBrightnessSlider : ui::Slider {
	HaloBrightnessSlider() {
		quantity = new HaloBrightnessQuantity;
	}
	~HaloBrightnessSlider() {
		delete quantity;
	}
};


struct KnobScrollSensitivityQuantity : Quantity {
	void setValue(float value) override {
		value = math::clamp(value, getMinValue(), getMaxValue());
		settings::knobScrollSensitivity = std::pow(2.f, value);
	}
	float getValue() override {
		return std::log2(settings::knobScrollSensitivity);
	}
	float getMinValue() override {
		return std::log2(1e-4f);
	}
	float getMaxValue() override {
		return std::log2(1e-2f);
	}
	float getDefaultValue() override {
		return std::log2(1e-3f);
	}
	float getDisplayValue() override {
		return std::pow(2.f, getValue() - getDefaultValue());
	}
	void setDisplayValue(float displayValue) override {
		setValue(std::log2(displayValue) + getDefaultValue());
	}
	std::string getLabel() override {
		return "Scroll wheel knob sensitivity";
	}
	int getDisplayPrecision() override {
		return 2;
	}
};
struct KnobScrollSensitivitySlider : ui::Slider {
	KnobScrollSensitivitySlider() {
		quantity = new KnobScrollSensitivityQuantity;
	}
	~KnobScrollSensitivitySlider() {
		delete quantity;
	}
};


struct ViewButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		menu->addChild(createBoolPtrMenuItem("Show tooltips", "", &settings::tooltips));

		ZoomSlider* zoomSlider = new ZoomSlider;
		zoomSlider->box.size.x = 250.0;
		menu->addChild(zoomSlider);

		CableOpacitySlider* cableOpacitySlider = new CableOpacitySlider;
		cableOpacitySlider->box.size.x = 250.0;
		menu->addChild(cableOpacitySlider);

		CableTensionSlider* cableTensionSlider = new CableTensionSlider;
		cableTensionSlider->box.size.x = 250.0;
		menu->addChild(cableTensionSlider);

		RackBrightnessSlider* rackBrightnessSlider = new RackBrightnessSlider;
		rackBrightnessSlider->box.size.x = 250.0;
		menu->addChild(rackBrightnessSlider);

		HaloBrightnessSlider* haloBrightnessSlider = new HaloBrightnessSlider;
		haloBrightnessSlider->box.size.x = 250.0;
		menu->addChild(haloBrightnessSlider);

		double frameRate = APP->window->getMonitorRefreshRate() / settings::frameSwapInterval;
		menu->addChild(createSubmenuItem("Frame rate", string::f("%.0f Hz", frameRate), [=](ui::Menu* menu) {
			for (int i = 1; i <= 6; i++) {
				double frameRate = APP->window->getMonitorRefreshRate() / i;
				menu->addChild(createCheckMenuItem(string::f("%.0f Hz", frameRate), "",
					[=]() {return settings::frameSwapInterval == i;},
					[=]() {settings::frameSwapInterval = i;}
				));
			}
		}));

		bool fullscreen = APP->window->isFullScreen();
		std::string fullscreenText = "F11";
		if (fullscreen)
			fullscreenText += " " CHECKMARK_STRING;
		menu->addChild(createMenuItem("Fullscreen", fullscreenText, [=]() {
			APP->window->setFullScreen(!fullscreen);
		}));

		menu->addChild(new ui::MenuSeparator);

		menu->addChild(createBoolPtrMenuItem("Lock cursor while dragging params", "", &settings::allowCursorLock));

		static const std::vector<std::string> knobModeLabels = {
			"Linear",
			"Scaled linear",
			"Absolute rotary",
			"Relative rotary",
		};
		static const std::vector<int> knobModes = {0, 2, 3};
		menu->addChild(createSubmenuItem("Knob mode", knobModeLabels[settings::knobMode], [=](ui::Menu* menu) {
			for (int knobMode : knobModes) {
				menu->addChild(createCheckMenuItem(knobModeLabels[knobMode], "",
					[=]() {return settings::knobMode == knobMode;},
					[=]() {settings::knobMode = (settings::KnobMode) knobMode;}
				));
			}
		}));

		menu->addChild(createBoolPtrMenuItem("Scroll wheel knob control", "", &settings::knobScroll));

		KnobScrollSensitivitySlider* knobScrollSensitivitySlider = new KnobScrollSensitivitySlider;
		knobScrollSensitivitySlider->box.size.x = 250.0;
		menu->addChild(knobScrollSensitivitySlider);

		menu->addChild(createBoolPtrMenuItem("Lock module positions", "", &settings::lockModules));

		menu->addChild(createBoolPtrMenuItem("Auto-squeeze modules when dragging", "", &settings::squeezeModules));
	}
};


////////////////////
// Engine
////////////////////


struct SampleRateItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		// Auto sample rate
		std::string rightText;
		if (settings::sampleRate == 0) {
			float sampleRate = APP->engine->getSampleRate();
			rightText += string::f("(%g kHz) ", sampleRate / 1000.f);
		}
		menu->addChild(createCheckMenuItem("Auto", rightText,
			[=]() {return settings::sampleRate == 0;},
			[=]() {settings::sampleRate = 0;}
		));

		// Power-of-2 oversample times 44.1kHz or 48kHz
		for (int i = -2; i <= 4; i++) {
			for (int j = 0; j < 2; j++) {
				float oversample = std::pow(2.f, i);
				float sampleRate = (j == 0) ? 44100.f : 48000.f;
				sampleRate *= oversample;

				std::string text = string::f("%g kHz", sampleRate / 1000.f);
				std::string rightText;
				if (oversample > 1.f) {
					rightText += string::f("(%.0fx)", oversample);
				}
				else if (oversample < 1.f) {
					rightText += string::f("(1/%.0fx)", 1.f / oversample);
				}
				menu->addChild(createCheckMenuItem(text, rightText,
					[=]() {return settings::sampleRate == sampleRate;},
					[=]() {settings::sampleRate = sampleRate;}
				));
			}
		}
		return menu;
	}
};


struct EngineButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		std::string cpuMeterText = "F3";
		if (settings::cpuMeter)
			cpuMeterText += " " CHECKMARK_STRING;
		menu->addChild(createMenuItem("Performance meters", cpuMeterText, [=]() {
			settings::cpuMeter ^= true;
		}));

		menu->addChild(createMenuItem<SampleRateItem>("Sample rate", RIGHT_ARROW));

		menu->addChild(createSubmenuItem("Threads", string::f("%d", settings::threadCount), [=](ui::Menu* menu) {
			// BUG This assumes SMT is enabled.
			int cores = system::getLogicalCoreCount() / 2;

			for (int i = 1; i <= 2 * cores; i++) {
				std::string rightText;
				if (i == cores)
					rightText += "(most modules)";
				else if (i == 1)
					rightText += "(lowest CPU usage)";
				menu->addChild(createCheckMenuItem(string::f("%d", i), rightText,
					[=]() {return settings::threadCount == i;},
					[=]() {settings::threadCount = i;}
				));
			}
		}));
	}
};


////////////////////
// Plugins
////////////////////


struct AccountPasswordField : ui::PasswordField {
	ui::MenuItem* logInItem;
	void onAction(const ActionEvent& e) override {
		logInItem->doAction();
	}
};


struct LogInItem : ui::MenuItem {
	ui::TextField* emailField;
	ui::TextField* passwordField;

	void onAction(const ActionEvent& e) override {
		std::string email = emailField->text;
		std::string password = passwordField->text;
		std::thread t([=] {
			library::logIn(email, password);
			library::checkUpdates();
		});
		t.detach();
		e.unconsume();
	}

	void step() override {
		text = "Log in";
		rightText = library::loginStatus;
		MenuItem::step();
	}
};


struct SyncUpdatesItem : ui::MenuItem {
	void step() override {
		if (library::updateStatus != "") {
			text = library::updateStatus;
		}
		else if (library::isSyncing) {
			text = "Updating...";
		}
		else if (!library::hasUpdates()) {
			text = "Up-to-date";
		}
		else {
			text = "Update all";
		}

		disabled = library::isSyncing || !library::hasUpdates();
		MenuItem::step();
	}

	void onAction(const ActionEvent& e) override {
		std::thread t([=] {
			library::syncUpdates();
		});
		t.detach();
		e.unconsume();
	}
};


struct SyncUpdateItem : ui::MenuItem {
	std::string slug;

	void setUpdate(const std::string& slug) {
		this->slug = slug;

		auto it = library::updateInfos.find(slug);
		if (it == library::updateInfos.end())
			return;
		library::UpdateInfo update = it->second;

		text = update.name;
	}

	ui::Menu* createChildMenu() override {
		auto it = library::updateInfos.find(slug);
		if (it == library::updateInfos.end())
			return NULL;
		library::UpdateInfo update = it->second;

		if (update.changelogUrl == "")
			return NULL;

		ui::Menu* menu = new ui::Menu;

		std::string changelogUrl = update.changelogUrl;
		menu->addChild(createMenuItem("Changelog", "", [=]() {
			system::openBrowser(changelogUrl);
		}));

		return menu;
	}

	void step() override {
		disabled = library::isSyncing;

		auto it = library::updateInfos.find(slug);
		if (it != library::updateInfos.end()) {
			library::UpdateInfo update = it->second;

			if (update.downloaded) {
				rightText = CHECKMARK_STRING;
				disabled = true;
			}
			else if (slug == library::updateSlug) {
				rightText = string::f("%.0f%%", library::updateProgress * 100.f);
			}
			else {
				rightText = "";
				plugin::Plugin* p = plugin::getPlugin(slug);
				if (p) {
					rightText += p->version + " → ";
				}
				rightText += update.version;
			}
		}

		MenuItem::step();
	}

	void onAction(const ActionEvent& e) override {
		std::thread t([=] {
			library::syncUpdate(slug);
		});
		t.detach();
		e.unconsume();
	}
};


struct LibraryMenu : ui::Menu {
	LibraryMenu() {
		refresh();
	}

	void step() override {
		// Refresh menu when appropriate
		if (library::refreshRequested) {
			library::refreshRequested = false;
			refresh();
		}
		Menu::step();
	}

	void refresh() {
		setChildMenu(NULL);
		clearChildren();

		if (settings::devMode) {
			addChild(createMenuLabel("Disabled in development mode"));
		}
		else if (!library::isLoggedIn()) {
			addChild(createMenuItem("Register VCV account", "", [=]() {
				system::openBrowser("https://vcvrack.com/login");
			}));

			ui::TextField* emailField = new ui::TextField;
			emailField->placeholder = "Email";
			emailField->box.size.x = 240.0;
			addChild(emailField);

			AccountPasswordField* passwordField = new AccountPasswordField;
			passwordField->placeholder = "Password";
			passwordField->box.size.x = 240.0;
			passwordField->nextField = emailField;
			emailField->nextField = passwordField;
			addChild(passwordField);

			LogInItem* logInItem = new LogInItem;
			logInItem->emailField = emailField;
			logInItem->passwordField = passwordField;
			passwordField->logInItem = logInItem;
			addChild(logInItem);
		}
		else {
			addChild(createMenuItem("Log out", "", [=]() {
				library::logOut();
			}));

			addChild(createMenuItem("Browse VCV Library", "", [=]() {
				system::openBrowser("https://library.vcvrack.com/");
			}));

			SyncUpdatesItem* syncItem = new SyncUpdatesItem;
			syncItem->text = "Update all";
			addChild(syncItem);

			if (!library::updateInfos.empty()) {
				addChild(new ui::MenuSeparator);
				addChild(createMenuLabel("Updates"));

				for (auto& pair : library::updateInfos) {
					SyncUpdateItem* updateItem = new SyncUpdateItem;
					updateItem->setUpdate(pair.first);
					addChild(updateItem);
				}
			}
		}
	}
};


struct LibraryButton : MenuButton {
	NotificationIcon* notification;

	LibraryButton() {
		notification = new NotificationIcon;
		addChild(notification);
	}

	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu<LibraryMenu>();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		// Check for updates when menu is opened
		std::thread t([&]() {
			system::setThreadName("Library");
			library::checkUpdates();
		});
		t.detach();
	}

	void step() override {
		notification->box.pos = math::Vec(0, 0);
		notification->visible = library::hasUpdates();

		// Popup when updates finish downloading
		if (library::restartRequested) {
			library::restartRequested = false;
			if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "All plugins have been downloaded. Close and re-launch Rack to load new updates.")) {
				APP->window->close();
			}
		}

		MenuButton::step();
	}
};


////////////////////
// Help
////////////////////


struct HelpButton : MenuButton {
	NotificationIcon* notification;

	HelpButton() {
		notification = new NotificationIcon;
		addChild(notification);
	}

	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		menu->addChild(createMenuItem("Tips", "", [=]() {
			APP->scene->addChild(tipWindowCreate());
		}));

		menu->addChild(createMenuItem("User manual", "F1", [=]() {
			system::openBrowser("https://vcvrack.com/manual");
		}));

		menu->addChild(createMenuItem("Support", "", [=]() {
			system::openBrowser("https://vcvrack.com/support");
		}));

		menu->addChild(createMenuItem("VCVRack.com", "", [=]() {
			system::openBrowser("https://vcvrack.com/");
		}));

		menu->addChild(new ui::MenuSeparator);

		menu->addChild(createMenuLabel(APP_NAME + " " + APP_EDITION_NAME + " " + APP_VERSION));

		menu->addChild(createMenuItem("Open user folder", "", [=]() {
			system::openDirectory(asset::user(""));
		}));

		menu->addChild(createMenuItem("Changelog", "", [=]() {
			system::openBrowser("https://github.com/VCVRack/Rack/blob/v2/CHANGELOG.md");
		}));

		if (library::isAppUpdateAvailable()) {
			menu->addChild(createMenuItem("Update " + APP_NAME, APP_VERSION + " → " + library::appVersion, [=]() {
				system::openBrowser(library::appDownloadUrl);
			}));
		}
		else if (!settings::autoCheckUpdates && !settings::devMode) {
			menu->addChild(createMenuItem("Check for " + APP_NAME + " update", "", [=]() {
				std::thread t([&]() {
					library::checkAppUpdate();
				});
				t.detach();
			}, false, true));
		}
	}

	void step() override {
		notification->box.pos = math::Vec(0, 0);
		notification->visible = library::isAppUpdateAvailable();
		MenuButton::step();
	}
};


////////////////////
// MenuBar
////////////////////


struct MeterLabel : ui::Label {
	int frameIndex = 0;
	double frameDurationTotal = 0.0;
	double frameDurationAvg = 0.0;
	double uiLastTime = 0.0;
	double uiLastThreadTime = 0.0;
	double uiFrac = 0.0;

	void step() override {
		// Compute frame rate
		double frameDuration = APP->window->getLastFrameDuration();
		frameDurationTotal += frameDuration;
		frameIndex++;
		if (frameDurationTotal >= 1.0) {
			frameDurationAvg = frameDurationTotal / frameIndex;
			frameDurationTotal = 0.0;
			frameIndex = 0;
		}

		// Compute UI thread CPU
		// double time = system::getTime();
		// double uiDuration = time - uiLastTime;
		// if (uiDuration >= 1.0) {
		// 	double threadTime = system::getThreadTime();
		// 	uiFrac = (threadTime - uiLastThreadTime) / uiDuration;
		// 	uiLastThreadTime = threadTime;
		// 	uiLastTime = time;
		// }

		double meterAverage = APP->engine->getMeterAverage();
		double meterMax = APP->engine->getMeterMax();
		text = string::f("%.1f fps  %.1f%% avg  %.1f%% max", 1.0 / frameDurationAvg, meterAverage * 100, meterMax * 100);
		Label::step();
	}
};


struct MenuBar : widget::OpaqueWidget {
	MeterLabel* meterLabel;

	MenuBar() {
		const float margin = 5;
		box.size.y = BND_WIDGET_HEIGHT + 2 * margin;

		ui::SequentialLayout* layout = new ui::SequentialLayout;
		layout->margin = math::Vec(margin, margin);
		layout->spacing = math::Vec(0, 0);
		addChild(layout);

		FileButton* fileButton = new FileButton;
		fileButton->text = "File";
		layout->addChild(fileButton);

		EditButton* editButton = new EditButton;
		editButton->text = "Edit";
		layout->addChild(editButton);

		ViewButton* viewButton = new ViewButton;
		viewButton->text = "View";
		layout->addChild(viewButton);

		EngineButton* engineButton = new EngineButton;
		engineButton->text = "Engine";
		layout->addChild(engineButton);

		LibraryButton* libraryButton = new LibraryButton;
		libraryButton->text = "Library";
		layout->addChild(libraryButton);

		HelpButton* helpButton = new HelpButton;
		helpButton->text = "Help";
		layout->addChild(helpButton);

		// ui::Label* titleLabel = new ui::Label;
		// titleLabel->color.a = 0.5;
		// layout->addChild(titleLabel);

		meterLabel = new MeterLabel;
		meterLabel->box.pos.y = margin;
		meterLabel->box.size.x = 300;
		meterLabel->alignment = ui::Label::RIGHT_ALIGNMENT;
		meterLabel->color.a = 0.5;
		addChild(meterLabel);
	}

	void draw(const DrawArgs& args) override {
		bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL);
		bndBevel(args.vg, 0.0, 0.0, box.size.x, box.size.y);

		Widget::draw(args);
	}

	void step() override {
		meterLabel->box.pos.x = box.size.x - meterLabel->box.size.x - 5;
		Widget::step();
	}
};


} // namespace menuBar


widget::Widget* createMenuBar() {
	menuBar::MenuBar* menuBar = new menuBar::MenuBar;
	return menuBar;
}


} // namespace app
} // namespace rack
