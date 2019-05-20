#include "app/MenuBar.hpp"
#include "window.hpp"
#include "engine/Engine.hpp"
#include "asset.hpp"
#include "ui/Button.hpp"
#include "ui/MenuItem.hpp"
#include "ui/SequentialLayout.hpp"
#include "ui/Slider.hpp"
#include "ui/TextField.hpp"
#include "ui/PasswordField.hpp"
#include "ui/ProgressBar.hpp"
#include "app.hpp"
#include "settings.hpp"
#include "helpers.hpp"
#include "system.hpp"
#include "plugin.hpp"
#include "patch.hpp"
#include <thread>


namespace rack {
namespace app {


struct MenuButton : ui::Button {
	void step() override {
		box.size.x = bndLabelWidth(APP->window->vg, -1, text.c_str()) + 1.0;
		Widget::step();
	}
	void draw(const DrawArgs &args) override {
		BNDwidgetState state = BND_DEFAULT;
		if (APP->event->hoveredWidget == this)
			state = BND_HOVER;
		if (APP->event->draggedWidget == this)
			state = BND_ACTIVE;
		bndMenuItem(args.vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());
	}
};

////////////////////
// File
////////////////////

struct NewItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->patch->resetDialog();
	}
};

struct OpenItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->patch->loadDialog();
	}
};

struct SaveItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->patch->saveDialog();
	}
};

struct SaveAsItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->patch->saveAsDialog();
	}
};

struct SaveTemplateItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->patch->saveTemplateDialog();
	}
};

struct RevertItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->patch->revertDialog();
	}
};

struct QuitItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->window->close();
	}
};

struct FileButton : MenuButton {
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		NewItem *newItem = new NewItem;
		newItem->text = "New";
		newItem->rightText = RACK_MOD_CTRL_NAME "+N";
		menu->addChild(newItem);

		OpenItem *openItem = new OpenItem;
		openItem->text = "Open";
		openItem->rightText = RACK_MOD_CTRL_NAME "+O";
		menu->addChild(openItem);

		SaveItem *saveItem = new SaveItem;
		saveItem->text = "Save";
		saveItem->rightText = RACK_MOD_CTRL_NAME "+S";
		menu->addChild(saveItem);

		SaveAsItem *saveAsItem = new SaveAsItem;
		saveAsItem->text = "Save as";
		saveAsItem->rightText = RACK_MOD_CTRL_NAME "+Shift+S";
		menu->addChild(saveAsItem);

		SaveTemplateItem *saveTemplateItem = new SaveTemplateItem;
		saveTemplateItem->text = "Save template";
		menu->addChild(saveTemplateItem);

		RevertItem *revertItem = new RevertItem;
		revertItem->text = "Revert";
		menu->addChild(revertItem);

		QuitItem *quitItem = new QuitItem;
		quitItem->text = "Quit";
		quitItem->rightText = RACK_MOD_CTRL_NAME "+Q";
		menu->addChild(quitItem);
	}
};

////////////////////
// Edit
////////////////////

struct UndoItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->history->undo();
	}
};

struct RedoItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->history->redo();
	}
};

struct DisconnectCablesItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->patch->disconnectDialog();
	}
};

struct EditButton : MenuButton {
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		UndoItem *undoItem = new UndoItem;
		undoItem->text = "Undo " + APP->history->getUndoName();
		undoItem->rightText = RACK_MOD_CTRL_NAME "+Z";
		undoItem->disabled = !APP->history->canUndo();
		menu->addChild(undoItem);

		RedoItem *redoItem = new RedoItem;
		redoItem->text = "Redo " + APP->history->getRedoName();
		redoItem->rightText = RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME "+Z";
		redoItem->disabled = !APP->history->canRedo();
		menu->addChild(redoItem);

		DisconnectCablesItem *disconnectCablesItem = new DisconnectCablesItem;
		disconnectCablesItem->text = "Disconnect cables";
		menu->addChild(disconnectCablesItem);
	}
};

////////////////////
// View
////////////////////

struct ZoomQuantity : Quantity {
	void setValue(float value) override {
		settings::zoom = value;
	}
	float getValue() override {
		return settings::zoom;
	}
	float getMinValue() override {return -2.0;}
	float getMaxValue() override {return 2.0;}
	float getDefaultValue() override {return 0.0;}
	float getDisplayValue() override {return std::round(std::pow(2.f, getValue()) * 100);}
	void setDisplayValue(float displayValue) override {setValue(std::log2(displayValue / 100));}
	std::string getLabel() override {return "Zoom";}
	std::string getUnit() override {return "%";}
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
	float getDefaultValue() override {return 0.5;}
	float getDisplayValue() override {return getValue() * 100;}
	void setDisplayValue(float displayValue) override {setValue(displayValue / 100);}
	std::string getLabel() override {return "Cable opacity";}
	std::string getUnit() override {return "%";}
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
	float getDefaultValue() override {return 0.5;}
	std::string getLabel() override {return "Cable tension";}
	int getDisplayPrecision() override {return 2;}
};

struct CableTensionSlider : ui::Slider {
	CableTensionSlider() {
		quantity = new CableTensionQuantity;
	}
	~CableTensionSlider() {
		delete quantity;
	}
};

struct ParamTooltipItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		settings::paramTooltip ^= true;
	}
};

struct LockModulesItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		settings::lockModules ^= true;
	}
};

struct FullscreenItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->window->setFullScreen(!APP->window->isFullScreen());
	}
};

struct ViewButton : MenuButton {
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		ParamTooltipItem *paramTooltipItem = new ParamTooltipItem;
		paramTooltipItem->text = "Parameter tooltips";
		paramTooltipItem->rightText = CHECKMARK(settings::paramTooltip);
		menu->addChild(paramTooltipItem);

		LockModulesItem *lockModulesItem = new LockModulesItem;
		lockModulesItem->text = "Lock modules";
		lockModulesItem->rightText = CHECKMARK(settings::lockModules);
		menu->addChild(lockModulesItem);

		ZoomSlider *zoomSlider = new ZoomSlider;
		zoomSlider->box.size.x = 200.0;
		menu->addChild(zoomSlider);

		CableOpacitySlider *cableOpacitySlider = new CableOpacitySlider;
		cableOpacitySlider->box.size.x = 200.0;
		menu->addChild(cableOpacitySlider);

		CableTensionSlider *cableTensionSlider = new CableTensionSlider;
		cableTensionSlider->box.size.x = 200.0;
		menu->addChild(cableTensionSlider);

		FullscreenItem *fullscreenItem = new FullscreenItem;
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
	void onAction(const event::Action &e) override {
		settings::cpuMeter ^= true;
	}
};

struct EnginePauseItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		APP->engine->setPaused(!APP->engine->isPaused());
	}
};

struct SampleRateValueItem : ui::MenuItem {
	float sampleRate;
	void onAction(const event::Action &e) override {
		settings::sampleRate = sampleRate;
		APP->engine->setPaused(false);
	}
};

struct SampleRateItem : ui::MenuItem {
	ui::Menu *createChildMenu() override {
		ui::Menu *menu = new ui::Menu;

		EnginePauseItem *enginePauseItem = new EnginePauseItem;
		enginePauseItem->text = "Pause";
		enginePauseItem->rightText = CHECKMARK(APP->engine->isPaused());
		menu->addChild(enginePauseItem);

		for (int i = 0; i <= 4; i++) {
			for (int j = 0; j < 2; j++) {
				int oversample = 1 << i;
				float sampleRate = (j == 0) ? 44100.f : 48000.f;
				sampleRate *= oversample;

				SampleRateValueItem *item = new SampleRateValueItem;
				item->sampleRate = sampleRate;
				item->text = string::f("%g kHz", sampleRate / 1000.0);
				if (oversample > 1)
					item->rightText += string::f("(%dx)", oversample);
				item->rightText += " ";
				item->rightText += CHECKMARK(settings::sampleRate == sampleRate);
				menu->addChild(item);
			}
		}
		return menu;
	}
};

struct RealTimeItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		settings::realTime ^= true;
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
	void onAction(const event::Action &e) override {
		settings::threadCount = threadCount;
	}
};

struct ThreadCountItem : ui::MenuItem {
	ui::Menu *createChildMenu() override {
		ui::Menu *menu = new ui::Menu;

		RealTimeItem *realTimeItem = new RealTimeItem;
		realTimeItem->text = "Real-time priority";
		realTimeItem->rightText = CHECKMARK(settings::realTime);
		menu->addChild(realTimeItem);

		int coreCount = system::getLogicalCoreCount();
		for (int i = 1; i <= coreCount; i++) {
			ThreadCountValueItem *item = new ThreadCountValueItem;
			item->setThreadCount(i);
			menu->addChild(item);
		}
		return menu;
	}
};

struct EngineButton : MenuButton {
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		CpuMeterItem *cpuMeterItem = new CpuMeterItem;
		cpuMeterItem->text = "CPU timer";
		cpuMeterItem->rightText = CHECKMARK(settings::cpuMeter);
		menu->addChild(cpuMeterItem);

		SampleRateItem *sampleRateItem = new SampleRateItem;
		sampleRateItem->text = "Sample rate";
		sampleRateItem->rightText = RIGHT_ARROW;
		menu->addChild(sampleRateItem);

		ThreadCountItem *threadCount = new ThreadCountItem;
		threadCount->text = "Threads";
		threadCount->rightText = RIGHT_ARROW;
		menu->addChild(threadCount);
	}
};

////////////////////
// Plugins
////////////////////

static bool isLoggingIn = false;

struct RegisterItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		std::thread t([]() {
			system::openBrowser("https://vcvrack.com/");
		});
		t.detach();
	}
};

struct AccountEmailField : ui::TextField {
	ui::TextField *passwordField;
	void onSelectKey(const event::SelectKey &e) override {
		if (e.action == GLFW_PRESS && e.key == GLFW_KEY_TAB) {
			APP->event->setSelected(passwordField);
			e.consume(this);
		}

		if (!e.getTarget())
			ui::TextField::onSelectKey(e);
	}
};

struct AccountPasswordField : ui::PasswordField {
	ui::MenuItem *logInItem;
	void onSelectKey(const event::SelectKey &e) override {
		if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
			logInItem->doAction();
			e.consume(this);
		}

		if (!e.getTarget())
			ui::PasswordField::onSelectKey(e);
	}
};

struct LogInItem : ui::MenuItem {
	ui::TextField *emailField;
	ui::TextField *passwordField;
	void onAction(const event::Action &e) override {
		isLoggingIn = true;
		std::string email = emailField->text;
		std::string password = passwordField->text;
		std::thread t([=]() {
			plugin::logIn(email, password);
			isLoggingIn = false;
		});
		t.detach();
		e.consume(NULL);
	}

	void step() override {
		disabled = isLoggingIn;
		text = "Log in";
		rightText = plugin::loginStatus;
	}
};

struct ManageItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		std::thread t([]() {
			system::openBrowser("https://vcvrack.com/plugins.html");
		});
		t.detach();
	}
};

struct SyncItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		std::thread t([=]() {
			plugin::syncUpdates();
		});
		t.detach();
	}
};

struct UpdateItem : ui::MenuItem {
	std::string changelogUrl;
	void onAction(const event::Action &e) override {
		std::thread t([=]() {
			system::openBrowser(changelogUrl);
		});
		t.detach();
	}
};

#if 0
struct SyncButton : ui::Button {
	bool checked = false;
	/** Updates are available */
	bool available = false;
	/** Plugins have been updated */
	bool completed = false;

	void step() override {
		// Check for plugin update on first step()
		if (!checked) {
			std::thread t([this]() {
				if (plugin::sync(true))
					available = true;
			});
			t.detach();
			checked = true;
		}
		// Display message if we've completed updates
		if (completed) {
			if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "All plugins have been updated. Close Rack and re-launch it to load new updates.")) {
				APP->window->close();
			}
			completed = false;
		}
	}
	void onAction(const event::Action &e) override {
		available = false;
		std::thread t([this]() {
			if (plugin::sync(false))
				completed = true;
		});
		t.detach();
	}
};
#endif

struct LogOutItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		plugin::logOut();
	}
};

struct DownloadQuantity : Quantity {
	float getValue() override {
		return plugin::downloadProgress;
	}

	float getDisplayValue() override {
		return getValue() * 100.f;
	}

	int getDisplayPrecision() override {return 0;}

	std::string getLabel() override {
		return "Downloading " + plugin::downloadName;
	}

	std::string getUnit() override {return "%";}
};

struct PluginsMenu : ui::Menu {
	int state = 0;

	PluginsMenu() {
		refresh();
	}

	void step() override {
		Menu::step();
	}

	void refresh() {
		clearChildren();

		{
			ui::MenuLabel *disabledLable = new ui::MenuLabel;
			disabledLable->text = "Server not yet available";
			addChild(disabledLable);
			return;
		}

		if (plugin::isLoggedIn()) {
			ManageItem *manageItem = new ManageItem;
			manageItem->text = "Manage";
			addChild(manageItem);

			LogOutItem *logOutItem = new LogOutItem;
			logOutItem->text = "Log out";
			addChild(logOutItem);

			SyncItem *syncItem = new SyncItem;
			syncItem->text = "Update all";
			syncItem->disabled = plugin::updates.empty();
			addChild(syncItem);

			if (!plugin::updates.empty()) {
				addChild(new ui::MenuEntry);

				ui::MenuLabel *updatesLabel = new ui::MenuLabel;
				updatesLabel->text = "Updates (click for changelog)";
				addChild(updatesLabel);

				for (const plugin::Update &update : plugin::updates) {
					UpdateItem *updateItem = new UpdateItem;
					updateItem->text = update.pluginSlug;
					plugin::Plugin *p = plugin::getPlugin(update.pluginSlug);
					if (p) {
						updateItem->rightText += "v" + p->version + " → ";
					}
					updateItem->rightText += "v" + update.version;
					updateItem->changelogUrl = update.changelogUrl;
					updateItem->disabled = update.changelogUrl.empty();
					addChild(updateItem);
				}
			}
		}
		else {
			RegisterItem *registerItem = new RegisterItem;
			registerItem->text = "Register VCV account";
			addChild(registerItem);

			AccountEmailField *emailField = new AccountEmailField;
			emailField->placeholder = "Email";
			emailField->box.size.x = 220.0;
			addChild(emailField);

			AccountPasswordField *passwordField = new AccountPasswordField;
			passwordField->placeholder = "Password";
			passwordField->box.size.x = 220.0;
			emailField->passwordField = passwordField;
			addChild(passwordField);

			LogInItem *logInItem = new LogInItem;
			logInItem->emailField = emailField;
			logInItem->passwordField = passwordField;
			passwordField->logInItem = logInItem;
			addChild(logInItem);
		}
	}
};

struct PluginsButton : MenuButton {
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu<PluginsMenu>();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;
	}

	void draw(const DrawArgs &args) override {
		MenuButton::draw(args);

		if (0) {
			// Notification circle
			nvgBeginPath(args.vg);
			nvgCircle(args.vg, 4, 2, 4.0);
			nvgFillColor(args.vg, nvgRGBf(1.0, 0.0, 0.0));
			nvgFill(args.vg);
			nvgStrokeColor(args.vg, nvgRGBf(0.5, 0.0, 0.0));
			nvgStroke(args.vg);
		}
	}
};

////////////////////
// Help
////////////////////

struct ManualItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		std::thread t(system::openBrowser, "https://vcvrack.com/manual/");
		t.detach();
	}
};

struct WebsiteItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		std::thread t(system::openBrowser, "https://vcvrack.com/");
		t.detach();
	}
};

struct CheckVersionItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		settings::checkVersion ^= true;
	}
};

struct UserFolderItem : ui::MenuItem {
	void onAction(const event::Action &e) override {
		std::thread t(system::openFolder, asset::user(""));
		t.detach();
	}
};

struct HelpButton : MenuButton {
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		ManualItem *manualItem = new ManualItem;
		manualItem->text = "Manual";
		manualItem->rightText = "F1";
		menu->addChild(manualItem);

		WebsiteItem *websiteItem = new WebsiteItem;
		websiteItem->text = "VCVRack.com";
		menu->addChild(websiteItem);

		CheckVersionItem *checkVersionItem = new CheckVersionItem;
		checkVersionItem->text = "Check version on launch";
		checkVersionItem->rightText = CHECKMARK(settings::checkVersion);
		menu->addChild(checkVersionItem);

		UserFolderItem *folderItem = new UserFolderItem;
		folderItem->text = "Open user folder";
		menu->addChild(folderItem);
	}
};

////////////////////
// MenuBar
////////////////////

MenuBar::MenuBar() {
	const float margin = 5;
	box.size.y = BND_WIDGET_HEIGHT + 2*margin;

	ui::SequentialLayout *layout = new ui::SequentialLayout;
	layout->box.pos = math::Vec(margin, margin);
	layout->spacing = math::Vec(0, 0);
	addChild(layout);

	FileButton *fileButton = new FileButton;
	fileButton->text = "File";
	layout->addChild(fileButton);

	EditButton *editButton = new EditButton;
	editButton->text = "Edit";
	layout->addChild(editButton);

	ViewButton *viewButton = new ViewButton;
	viewButton->text = "View";
	layout->addChild(viewButton);

	EngineButton *engineButton = new EngineButton;
	engineButton->text = "Engine";
	layout->addChild(engineButton);

	PluginsButton *pluginsButton = new PluginsButton;
	pluginsButton->text = "Plugins";
	layout->addChild(pluginsButton);

	HelpButton *helpButton = new HelpButton;
	helpButton->text = "Help";
	layout->addChild(helpButton);
}

void MenuBar::draw(const DrawArgs &args) {
	bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL);
	bndBevel(args.vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(args);
}


} // namespace app
} // namespace rack
