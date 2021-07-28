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
#include <ui/PasswordField.hpp>
#include <ui/ProgressBar.hpp>
#include <ui/Label.hpp>
#include <engine/Engine.hpp>
#include <Window.hpp>
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

struct UrlItem : ui::MenuItem {
	std::string url;
	void onAction(const ActionEvent& e) override {
		system::openBrowser(url);
	}
};

struct DirItem : ui::MenuItem {
	std::string path;
	void onAction(const ActionEvent& e) override {
		system::openDirectory(path);
	}
};

////////////////////
// File
////////////////////

struct FileButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(createMenuItem("New", RACK_MOD_CTRL_NAME "+N", []() {
			APP->patch->loadTemplateDialog();
		}));

		menu->addChild(createMenuItem("Open", RACK_MOD_CTRL_NAME "+O", []() {
			APP->patch->loadDialog();
		}));

		ui::MenuItem* openRecentItem = createSubmenuItem("Open recent", [](ui::Menu* menu) {
			for (const std::string& path : settings::recentPatchPaths) {
				std::string name = system::getStem(path);
				menu->addChild(createMenuItem(name, "", [=]() {
					APP->patch->loadPathDialog(path);
				}));
			}
		});
		openRecentItem->disabled = settings::recentPatchPaths.empty();
		menu->addChild(openRecentItem);

		menu->addChild(createMenuItem("Save", RACK_MOD_CTRL_NAME "+S", []() {
			APP->patch->saveDialog();
		}));

		menu->addChild(createMenuItem("Save as", RACK_MOD_CTRL_NAME "+Shift+S", []() {
			APP->patch->saveAsDialog();
		}));

		menu->addChild(createMenuItem("Save template", "", []() {
			APP->patch->saveTemplateDialog();
		}));

		ui::MenuItem* revertItem = createMenuItem("Revert", RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME "+O", []() {
			APP->patch->revertDialog();
		});
		revertItem->disabled = (APP->patch->path == "");
		menu->addChild(revertItem);

		menu->addChild(new ui::MenuSeparator);

		menu->addChild(createMenuItem("Quit", RACK_MOD_CTRL_NAME "+Q", []() {
			APP->window->close();
		}));
	}
};

////////////////////
// Edit
////////////////////

struct UndoItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		APP->history->undo();
	}
};

struct RedoItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		APP->history->redo();
	}
};

struct DisconnectCablesItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		APP->patch->disconnectDialog();
	}
};

struct AllowCursorLockItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		settings::allowCursorLock ^= true;
	}
};

struct KnobModeValueItem : ui::MenuItem {
	settings::KnobMode knobMode;
	void onAction(const ActionEvent& e) override {
		settings::knobMode = knobMode;
	}
};

struct KnobModeItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		static const std::vector<std::pair<settings::KnobMode, std::string>> knobModes = {
			{settings::KNOB_MODE_LINEAR, "Linear"},
			{settings::KNOB_MODE_SCALED_LINEAR, "Scaled linear"},
			{settings::KNOB_MODE_ROTARY_ABSOLUTE, "Absolute rotary"},
			{settings::KNOB_MODE_ROTARY_RELATIVE, "Relative rotary"},
		};
		for (const auto& pair : knobModes) {
			KnobModeValueItem* item = new KnobModeValueItem;
			item->knobMode = pair.first;
			item->text = pair.second;
			item->rightText = CHECKMARK(settings::knobMode == pair.first);
			menu->addChild(item);
		}
		return menu;
	}
};

struct KnobScrollItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		settings::knobScroll ^= true;
	}
};

struct LockModulesItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		settings::lockModules ^= true;
	}
};

struct EditButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		UndoItem* undoItem = new UndoItem;
		undoItem->text = "Undo " + APP->history->getUndoName();
		undoItem->rightText = RACK_MOD_CTRL_NAME "+Z";
		undoItem->disabled = !APP->history->canUndo();
		menu->addChild(undoItem);

		RedoItem* redoItem = new RedoItem;
		redoItem->text = "Redo " + APP->history->getRedoName();
		redoItem->rightText = RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME "+Z";
		redoItem->disabled = !APP->history->canRedo();
		menu->addChild(redoItem);

		DisconnectCablesItem* disconnectCablesItem = new DisconnectCablesItem;
		disconnectCablesItem->text = "Clear cables";
		menu->addChild(disconnectCablesItem);

		menu->addChild(new ui::MenuSeparator);

		AllowCursorLockItem* allowCursorLockItem = new AllowCursorLockItem;
		allowCursorLockItem->text = "Lock cursor when dragging parameters";
		allowCursorLockItem->rightText = CHECKMARK(settings::allowCursorLock);
		menu->addChild(allowCursorLockItem);

		KnobModeItem* knobModeItem = new KnobModeItem;
		knobModeItem->text = "Knob mode";
		knobModeItem->rightText = RIGHT_ARROW;
		menu->addChild(knobModeItem);

		KnobScrollItem* knobScrollItem = new KnobScrollItem;
		knobScrollItem->text = "Scroll wheel knob control";
		knobScrollItem->rightText = CHECKMARK(settings::knobScroll);
		menu->addChild(knobScrollItem);

		LockModulesItem* lockModulesItem = new LockModulesItem;
		lockModulesItem->text = "Lock module positions";
		lockModulesItem->rightText = CHECKMARK(settings::lockModules);
		menu->addChild(lockModulesItem);
	}
};

////////////////////
// View
////////////////////

struct ZoomQuantity : Quantity {
	void setValue(float value) override {
		settings::zoom = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::zoom;
	}
	float getMinValue() override {
		return settings::zoomMin;
	}
	float getMaxValue() override {
		return settings::zoomMax;
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
	std::string getLabel() override {
		return "Cable tension";
	}
	int getDisplayPrecision() override {
		return 2;
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

struct TooltipsItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		settings::tooltips ^= true;
	}
};

struct FrameRateValueItem : ui::MenuItem {
	int frameSwapInterval;
	void onAction(const ActionEvent& e) override {
		settings::frameSwapInterval = frameSwapInterval;
	}
};

struct FrameRateItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		for (int i = 1; i <= 6; i++) {
			double frameRate = APP->window->getMonitorRefreshRate() / i;

			FrameRateValueItem* item = new FrameRateValueItem;
			item->frameSwapInterval = i;
			item->text = string::f("%.0f Hz", frameRate);
			item->rightText += CHECKMARK(settings::frameSwapInterval == i);
			menu->addChild(item);
		}
		return menu;
	}
};

struct FullscreenItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		APP->window->setFullScreen(!APP->window->isFullScreen());
	}
};

struct ViewButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		TooltipsItem* tooltipsItem = new TooltipsItem;
		tooltipsItem->text = "Show tooltips";
		tooltipsItem->rightText = CHECKMARK(settings::tooltips);
		menu->addChild(tooltipsItem);

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

		FrameRateItem* frameRateItem = new FrameRateItem;
		frameRateItem->text = "Frame rate";
		frameRateItem->rightText = RIGHT_ARROW;
		menu->addChild(frameRateItem);

		FullscreenItem* fullscreenItem = new FullscreenItem;
		fullscreenItem->text = "Fullscreen";
		fullscreenItem->rightText = "F11";
		if (APP->window->isFullScreen())
			fullscreenItem->rightText = CHECKMARK_STRING " " + fullscreenItem->rightText;
		menu->addChild(fullscreenItem);
	}
};

////////////////////
// Engine
////////////////////

struct CpuMeterItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		settings::cpuMeter ^= true;
	}
};

struct SampleRateValueItem : ui::MenuItem {
	float sampleRate;
	void onAction(const ActionEvent& e) override {
		settings::sampleRate = sampleRate;
	}
};

struct SampleRateItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		SampleRateValueItem* autoItem = new SampleRateValueItem;
		autoItem->sampleRate = 0;
		autoItem->text = "Auto";
		if (settings::sampleRate == 0) {
			float sampleRate = APP->engine->getSampleRate();
			autoItem->rightText = string::f("(%g kHz) ", sampleRate / 1000.f);
			autoItem->rightText += CHECKMARK_STRING;
		}
		menu->addChild(autoItem);

		for (int i = -2; i <= 4; i++) {
			for (int j = 0; j < 2; j++) {
				float oversample = std::pow(2.f, i);
				float sampleRate = (j == 0) ? 44100.f : 48000.f;
				sampleRate *= oversample;

				SampleRateValueItem* item = new SampleRateValueItem;
				item->sampleRate = sampleRate;
				item->text = string::f("%g kHz", sampleRate / 1000.f);
				if (oversample > 1.f) {
					item->rightText += string::f("(%.0fx)", oversample);
				}
				else if (oversample < 1.f) {
					item->rightText += string::f("(1/%.0fx)", 1.f / oversample);
				}
				item->rightText += " ";
				item->rightText += CHECKMARK(settings::sampleRate == sampleRate);
				menu->addChild(item);
			}
		}
		return menu;
	}
};

struct ThreadCountValueItem : ui::MenuItem {
	int threadCount;
	void setThreadCount(int threadCount) {
		this->threadCount = threadCount;
		text = string::f("%d", threadCount);
		if (threadCount == system::getLogicalCoreCount() / 2)
			text += " (most modules)";
		else if (threadCount == 1)
			text += " (lowest CPU usage)";
		rightText = CHECKMARK(settings::threadCount == threadCount);
	}
	void onAction(const ActionEvent& e) override {
		settings::threadCount = threadCount;
	}
};

struct ThreadCountItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		int coreCount = system::getLogicalCoreCount();
		for (int i = 1; i <= coreCount; i++) {
			ThreadCountValueItem* item = new ThreadCountValueItem;
			item->setThreadCount(i);
			menu->addChild(item);
		}
		return menu;
	}
};

struct EngineButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		CpuMeterItem* cpuMeterItem = new CpuMeterItem;
		cpuMeterItem->text = "Performance meters";
		cpuMeterItem->rightText = "F3 ";
		cpuMeterItem->rightText += CHECKMARK(settings::cpuMeter);
		menu->addChild(cpuMeterItem);

		SampleRateItem* sampleRateItem = new SampleRateItem;
		sampleRateItem->text = "Sample rate";
		sampleRateItem->rightText = RIGHT_ARROW;
		menu->addChild(sampleRateItem);

		ThreadCountItem* threadCount = new ThreadCountItem;
		threadCount->text = "Threads";
		threadCount->rightText = RIGHT_ARROW;
		menu->addChild(threadCount);
	}
};

////////////////////
// Plugins
////////////////////

static bool isLoggingIn = false;

struct AccountEmailField : ui::TextField {
	ui::TextField* passwordField;

	void onSelectKey(const SelectKeyEvent& e) override {
		if (e.action == GLFW_PRESS && e.key == GLFW_KEY_TAB) {
			APP->event->setSelected(passwordField);
			e.consume(this);
		}

		if (!e.getTarget())
			ui::TextField::onSelectKey(e);
	}
};

struct AccountPasswordField : ui::PasswordField {
	ui::MenuItem* logInItem;

	void onSelectKey(const SelectKeyEvent& e) override {
		if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
			logInItem->doAction();
			e.consume(this);
		}

		if (!e.getTarget())
			ui::PasswordField::onSelectKey(e);
	}
};

struct LogInItem : ui::MenuItem {
	ui::TextField* emailField;
	ui::TextField* passwordField;

	void onAction(const ActionEvent& e) override {
		isLoggingIn = true;
		std::string email = emailField->text;
		std::string password = passwordField->text;
		std::thread t([=] {
			library::logIn(email, password);
			isLoggingIn = false;
		});
		t.detach();
		e.unconsume();
	}

	void step() override {
		disabled = isLoggingIn;
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

		UrlItem* changelogUrl = new UrlItem;
		changelogUrl->text = "Changelog";
		changelogUrl->url = update.changelogUrl;
		menu->addChild(changelogUrl);

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
					rightText += "v" + p->version + " → ";
				}
				rightText += "v" + update.version;
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


struct CheckUpdatesItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		std::thread t([&] {
			library::checkUpdates();
		});
		t.detach();
	}
};


struct LogOutItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		library::logOut();
	}
};


struct LibraryMenu : ui::Menu {
	bool loggedIn = false;

	LibraryMenu() {
		refresh();
	}

	void step() override {
		// Refresh menu when appropriate
		if (!loggedIn && library::isLoggedIn())
			refresh();
		Menu::step();
	}

	void refresh() {
		setChildMenu(NULL);
		clearChildren();

		if (settings::devMode) {
			addChild(createMenuLabel("Disabled in development mode"));
		}
		else if (!library::isLoggedIn()) {
			UrlItem* registerItem = new UrlItem;
			registerItem->text = "Register VCV account";
			registerItem->url = "https://vcvrack.com/login";
			addChild(registerItem);

			AccountEmailField* emailField = new AccountEmailField;
			emailField->placeholder = "Email";
			emailField->box.size.x = 240.0;
			addChild(emailField);

			AccountPasswordField* passwordField = new AccountPasswordField;
			passwordField->placeholder = "Password";
			passwordField->box.size.x = 240.0;
			emailField->passwordField = passwordField;
			addChild(passwordField);

			LogInItem* logInItem = new LogInItem;
			logInItem->emailField = emailField;
			logInItem->passwordField = passwordField;
			passwordField->logInItem = logInItem;
			addChild(logInItem);
		}
		else {
			loggedIn = true;

			LogOutItem* logOutItem = new LogOutItem;
			logOutItem->text = "Log out";
			addChild(logOutItem);

			UrlItem* manageItem = new UrlItem;
			manageItem->text = "Browse VCV Library";
			manageItem->url = "https://library.vcvrack.com/";
			addChild(manageItem);

			SyncUpdatesItem* syncItem = new SyncUpdatesItem;
			syncItem->text = "Update all";
			addChild(syncItem);

			if (!library::updateInfos.empty()) {
				addChild(new ui::MenuSeparator);

				ui::MenuLabel* updatesLabel = new ui::MenuLabel;
				updatesLabel->text = "Updates";
				addChild(updatesLabel);

				for (auto& pair : library::updateInfos) {
					SyncUpdateItem* updateItem = new SyncUpdateItem;
					updateItem->setUpdate(pair.first);
					addChild(updateItem);
				}
			}
			else if (!settings::autoCheckUpdates) {
				CheckUpdatesItem* checkUpdatesItem = new CheckUpdatesItem;
				checkUpdatesItem->text = "Check for updates";
				addChild(checkUpdatesItem);
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
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;
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

struct AppUpdateItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		UrlItem* changelogItem = new UrlItem;
		changelogItem->text = "Changelog";
		changelogItem->url = library::appChangelogUrl;
		menu->addChild(changelogItem);

		return menu;
	}

	void onAction(const ActionEvent& e) override {
		system::openBrowser(library::appDownloadUrl);
		APP->window->close();
	}
};


struct CheckAppUpdateItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		std::thread t([&]() {
			library::checkAppUpdate();
		});
		t.detach();
	}
};


struct TipItem : ui::MenuItem {
	void onAction(const ActionEvent& e) override {
		APP->scene->addChild(tipWindowCreate());
	}
};


struct HelpButton : MenuButton {
	NotificationIcon* notification;

	HelpButton() {
		notification = new NotificationIcon;
		addChild(notification);
	}

	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		TipItem* tipItem = new TipItem;
		tipItem->text = "Tips";
		menu->addChild(tipItem);

		UrlItem* manualItem = new UrlItem;
		manualItem->text = "Manual";
		manualItem->rightText = "F1";
		manualItem->url = "https://vcvrack.com/manual/";
		menu->addChild(manualItem);

		UrlItem* websiteItem = new UrlItem;
		websiteItem->text = "VCVRack.com";
		websiteItem->url = "https://vcvrack.com/";
		menu->addChild(websiteItem);

		menu->addChild(new ui::MenuSeparator);

		if (library::isAppUpdateAvailable()) {
			AppUpdateItem* appUpdateItem = new AppUpdateItem;
			appUpdateItem->text = "Update " + APP_NAME;
			appUpdateItem->rightText = APP_VERSION + " → " + library::appVersion;
			menu->addChild(appUpdateItem);
		}
		else if (!settings::autoCheckUpdates && !settings::devMode) {
			CheckAppUpdateItem* checkAppUpdateItem = new CheckAppUpdateItem;
			checkAppUpdateItem->text = "Check for " + APP_NAME + " update";
			menu->addChild(checkAppUpdateItem);
		}

		DirItem* dirItem = new DirItem;
		dirItem->text = "Open user folder";
		dirItem->path = asset::user("");
		menu->addChild(dirItem);

		menu->addChild(new ui::MenuSeparator);

		menu->addChild(createMenuLabel(APP_VARIANT));

		menu->addChild(createMenuLabel(APP_VERSION));
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

		double meterAverage = APP->engine->getMeterAverage();
		double meterMax = APP->engine->getMeterMax();
		text = string::f("%.1f fps / %.1f%% avg / %.1f%% max", 1.0 / frameDurationAvg, meterAverage * 100, meterMax * 100);
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

		ui::Label* alphaLabel = new ui::Label;
		alphaLabel->color.a = 0.5;
		alphaLabel->text = "Pre-alpha build. Not for release.";
		layout->addChild(alphaLabel);

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
