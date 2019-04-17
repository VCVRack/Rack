#include "app/Toolbar.hpp"
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
		bndMenuItem(args.vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());
	}
};


struct NewItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->patch->resetDialog();
	}
};


struct OpenItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->patch->loadDialog();
	}
};


struct SaveItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->patch->saveDialog();
	}
};


struct SaveAsItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->patch->saveAsDialog();
	}
};


struct SaveTemplateItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->patch->saveTemplateDialog();
	}
};


struct RevertItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->patch->revertDialog();
	}
};


struct DisconnectCablesItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->patch->disconnectDialog();
	}
};


struct QuitItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->window->close();
	}
};


struct FileButton : MenuButton {
	void onAction(const widget::ActionEvent &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		NewItem *newItem = new NewItem;
		newItem->text = "New";
		newItem->rightText = WINDOW_MOD_CTRL_NAME "+N";
		menu->addChild(newItem);

		OpenItem *openItem = new OpenItem;
		openItem->text = "Open";
		openItem->rightText = WINDOW_MOD_CTRL_NAME "+O";
		menu->addChild(openItem);

		SaveItem *saveItem = new SaveItem;
		saveItem->text = "Save";
		saveItem->rightText = WINDOW_MOD_CTRL_NAME "+S";
		menu->addChild(saveItem);

		SaveAsItem *saveAsItem = new SaveAsItem;
		saveAsItem->text = "Save as";
		saveAsItem->rightText = WINDOW_MOD_CTRL_NAME "+Shift+S";
		menu->addChild(saveAsItem);

		SaveTemplateItem *saveTemplateItem = new SaveTemplateItem;
		saveTemplateItem->text = "Save template";
		menu->addChild(saveTemplateItem);

		RevertItem *revertItem = new RevertItem;
		revertItem->text = "Revert";
		menu->addChild(revertItem);

		DisconnectCablesItem *disconnectCablesItem = new DisconnectCablesItem;
		disconnectCablesItem->text = "Disconnect cables";
		menu->addChild(disconnectCablesItem);

		QuitItem *quitItem = new QuitItem;
		quitItem->text = "Quit";
		quitItem->rightText = WINDOW_MOD_CTRL_NAME "+Q";
		menu->addChild(quitItem);
	}
};


struct UndoItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->history->undo();
	}
};


struct RedoItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->history->redo();
	}
};


struct EditButton : MenuButton {
	void onAction(const widget::ActionEvent &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		UndoItem *undoItem = new UndoItem;
		undoItem->text = "Undo " + APP->history->getUndoName();
		undoItem->rightText = WINDOW_MOD_CTRL_NAME "+Z";
		undoItem->disabled = !APP->history->canUndo();
		menu->addChild(undoItem);

		RedoItem *redoItem = new RedoItem;
		redoItem->text = "Redo " + APP->history->getRedoName();
		redoItem->rightText = WINDOW_MOD_CTRL_NAME "+" WINDOW_MOD_SHIFT_NAME "+Z";
		redoItem->disabled = !APP->history->canRedo();
		menu->addChild(redoItem);
	}
};


struct ZoomQuantity : Quantity {
	void setValue(float value) override {
		settings::zoom = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::zoom;
	}
	float getMinValue() override {return 0.25;}
	float getMaxValue() override {return 2.0;}
	float getDefaultValue() override {return 1.0;}
	float getDisplayValue() override {return std::round(getValue() * 100);}
	void setDisplayValue(float displayValue) override {setValue(displayValue / 100);}
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


struct CpuMeterItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		settings::cpuMeter ^= true;
	}
};


struct ParamTooltipItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		settings::paramTooltip ^= true;
	}
};


struct LockModulesItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		settings::lockModules ^= true;
	}
};


struct EnginePauseItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->engine->setPaused(!APP->engine->isPaused());
	}
};


struct SampleRateValueItem : ui::MenuItem {
	float sampleRate;
	void onAction(const widget::ActionEvent &e) override {
		APP->engine->setSampleRate(sampleRate);
		APP->engine->setPaused(false);
	}
};


struct SampleRateItem : ui::MenuItem {
	ui::Menu *createChildMenu() override {
		ui::Menu *menu = new ui::Menu;

		EnginePauseItem *enginePauseItem = new EnginePauseItem;
		enginePauseItem->text = "Pause engine";
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
				item->rightText += CHECKMARK(APP->engine->getSampleRate() == sampleRate);
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
		rightText = CHECKMARK(APP->engine->getThreadCount() == threadCount);
	}
	void onAction(const widget::ActionEvent &e) override {
		APP->engine->setThreadCount(threadCount);
	}
};


struct ThreadCount : ui::MenuItem {
	ui::Menu *createChildMenu() override {
		ui::Menu *menu = new ui::Menu;

		int coreCount = system::getLogicalCoreCount();
		for (int i = 1; i <= coreCount; i++) {
			ThreadCountValueItem *item = new ThreadCountValueItem;
			item->setThreadCount(i);
			menu->addChild(item);
		}
		return menu;
	}
};


struct FullscreenItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		APP->window->setFullScreen(!APP->window->isFullScreen());
	}
};


struct SettingsButton : MenuButton {
	void onAction(const widget::ActionEvent &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		ParamTooltipItem *paramTooltipItem = new ParamTooltipItem;
		paramTooltipItem->text = "Parameter tooltips";
		paramTooltipItem->rightText = CHECKMARK(settings::paramTooltip);
		menu->addChild(paramTooltipItem);

		CpuMeterItem *cpuMeterItem = new CpuMeterItem;
		cpuMeterItem->text = "CPU timer";
		cpuMeterItem->rightText = CHECKMARK(settings::cpuMeter);
		menu->addChild(cpuMeterItem);

		LockModulesItem *lockModulesItem = new LockModulesItem;
		lockModulesItem->text = "Lock modules";
		lockModulesItem->rightText = CHECKMARK(settings::lockModules);
		menu->addChild(lockModulesItem);

		SampleRateItem *sampleRateItem = new SampleRateItem;
		sampleRateItem->text = "Engine sample rate";
		sampleRateItem->rightText = RIGHT_ARROW;
		menu->addChild(sampleRateItem);

		ThreadCount *threadCount = new ThreadCount;
		threadCount->text = "Thread count";
		threadCount->rightText = RIGHT_ARROW;
		menu->addChild(threadCount);

		FullscreenItem *fullscreenItem = new FullscreenItem;
		fullscreenItem->text = "Fullscreen";
		fullscreenItem->rightText = "F11";
		if (APP->window->isFullScreen())
			fullscreenItem->rightText = CHECKMARK_STRING " " + fullscreenItem->rightText;
		menu->addChild(fullscreenItem);

		ZoomSlider *zoomSlider = new ZoomSlider;
		zoomSlider->box.size.x = 200.0;
		menu->addChild(zoomSlider);

		CableOpacitySlider *cableOpacitySlider = new CableOpacitySlider;
		cableOpacitySlider->box.size.x = 200.0;
		menu->addChild(cableOpacitySlider);

		CableTensionSlider *cableTensionSlider = new CableTensionSlider;
		cableTensionSlider->box.size.x = 200.0;
		menu->addChild(cableTensionSlider);
	}
};


struct RegisterItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		std::thread t([&]() {
			system::openBrowser("https://vcvrack.com/");
		});
		t.detach();
	}
};


struct AccountEmailField : ui::TextField {
	ui::TextField *passwordField;
	void onSelectKey(const widget::SelectKeyEvent &e) override {
		if (e.action == GLFW_PRESS && e.key == GLFW_KEY_TAB) {
			APP->event->selectedWidget = passwordField;
			e.consume(this);
		}

		if (!e.getConsumed())
			ui::TextField::onSelectKey(e);
	}
};


struct AccountPasswordField : ui::PasswordField {
	ui::MenuItem *logInItem;
	void onSelectKey(const widget::SelectKeyEvent &e) override {
		if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
			logInItem->doAction();
			e.consume(this);
		}

		if (!e.getConsumed())
			ui::PasswordField::onSelectKey(e);
	}
};


struct LogInItem : ui::MenuItem {
	ui::TextField *emailField;
	ui::TextField *passwordField;
	void onAction(const widget::ActionEvent &e) override {
		std::string email = emailField->text;
		std::string password = passwordField->text;
		std::thread t([&, email, password]() {
			plugin::logIn(email, password);
		});
		t.detach();
	}
};


struct ManageItem : ui::MenuItem {
	ManageItem() {
	}
	void onAction(const widget::ActionEvent &e) override {
		std::thread t([&]() {
			system::openBrowser("https://vcvrack.com/plugins.html");
		});
		t.detach();
	}
};


struct SyncItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
	}
};


// struct SyncButton : ui::Button {
// 	bool checked = false;
// 	/** Updates are available */
// 	bool available = false;
// 	/** Plugins have been updated */
// 	bool completed = false;

// 	void step() override {
// 		// Check for plugin update on first step()
// 		if (!checked) {
// 			std::thread t([this]() {
// 				if (plugin::sync(true))
// 					available = true;
// 			});
// 			t.detach();
// 			checked = true;
// 		}
// 		// Display message if we've completed updates
// 		if (completed) {
// 			if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "All plugins have been updated. Close Rack and re-launch it to load new updates.")) {
// 				APP->window->close();
// 			}
// 			completed = false;
// 		}
// 	}
// 	void onAction(const widget::ActionEvent &e) override {
// 		available = false;
// 		std::thread t([this]() {
// 			if (plugin::sync(false))
// 				completed = true;
// 		});
// 		t.detach();
// 	}
// };


struct LogOutItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
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


struct PluginsButton : MenuButton {
	void onAction(const widget::ActionEvent &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		// TODO Design dialog box for plugin syncing
		if (plugin::isDownloading) {
			ui::ProgressBar *downloadProgressBar = new ui::ProgressBar;
			downloadProgressBar->quantity = new DownloadQuantity;
			menu->addChild(downloadProgressBar);
		}
		else if (plugin::isLoggedIn()) {
			ManageItem *manageItem = new ManageItem;
			manageItem->text = "Manage plugins";
			menu->addChild(manageItem);

			SyncItem *syncItem = new SyncItem;
			syncItem->text = "Sync plugins";
			syncItem->disabled = true;
			menu->addChild(syncItem);

			LogOutItem *logOutItem = new LogOutItem;
			logOutItem->text = "Log out";
			menu->addChild(logOutItem);
		}
		else {
			RegisterItem *registerItem = new RegisterItem;
			registerItem->text = "Register VCV account";
			menu->addChild(registerItem);

			AccountEmailField *emailField = new AccountEmailField;
			emailField->placeholder = "Email";
			emailField->box.size.x = 200.0;
			menu->addChild(emailField);

			AccountPasswordField *passwordField = new AccountPasswordField;
			passwordField->placeholder = "Password";
			passwordField->box.size.x = 200.0;
			emailField->passwordField = passwordField;
			menu->addChild(passwordField);

			LogInItem *logInItem = new LogInItem;
			logInItem->text = "Log in";
			logInItem->emailField = emailField;
			logInItem->passwordField = passwordField;
			passwordField->logInItem = logInItem;
			menu->addChild(logInItem);
		}
	}

	void draw(const DrawArgs &args) override {
		MenuButton::draw(args);
		// if (1) {
		// 	// Notification circle
		// 	nvgBeginPath(args.vg);
		// 	nvgCircle(args.vg, box.size.x - 3, 3, 4.0);
		// 	nvgFillColor(args.vg, nvgRGBf(1.0, 0.0, 0.0));
		// 	nvgFill(args.vg);
		// 	nvgStrokeColor(args.vg, nvgRGBf(0.5, 0.0, 0.0));
		// 	nvgStroke(args.vg);
		// }
	}
};


struct ManualItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		std::thread t(system::openBrowser, "https://vcvrack.com/manual/");
		t.detach();
	}
};


struct WebsiteItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		std::thread t(system::openBrowser, "https://vcvrack.com/");
		t.detach();
	}
};


struct CheckVersionItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		settings::checkVersion ^= true;
	}
};


struct UserFolderItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override {
		std::thread t(system::openFolder, asset::user(""));
		t.detach();
	}
};


struct HelpButton : MenuButton {
	void onAction(const widget::ActionEvent &e) override {
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


Toolbar::Toolbar() {
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

	SettingsButton *settingsButton = new SettingsButton;
	settingsButton->text = "Settings";
	layout->addChild(settingsButton);

	PluginsButton *pluginsButton = new PluginsButton;
	pluginsButton->text = "Plugins";
	layout->addChild(pluginsButton);

	HelpButton *helpButton = new HelpButton;
	helpButton->text = "Help";
	layout->addChild(helpButton);
}

void Toolbar::draw(const DrawArgs &args) {
	bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL);
	bndBevel(args.vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(args);
}


} // namespace app
} // namespace rack
