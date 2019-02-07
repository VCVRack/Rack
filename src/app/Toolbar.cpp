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
		widget::Widget::step();
	}
	void draw(const widget::DrawContext &ctx) override {
		bndMenuItem(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());
	}
};


struct NewItem : ui::MenuItem {
	NewItem() {
		text = "New";
		rightText = WINDOW_MOD_CTRL_NAME "+N";
	}
	void onAction(const event::Action &e) override {
		APP->patch->resetDialog();
	}
};


struct OpenItem : ui::MenuItem {
	OpenItem() {
		text = "Open";
		rightText = WINDOW_MOD_CTRL_NAME "+O";
	}
	void onAction(const event::Action &e) override {
		APP->patch->loadDialog();
	}
};


struct SaveItem : ui::MenuItem {
	SaveItem() {
		text = "Save";
		rightText = WINDOW_MOD_CTRL_NAME "+S";
	}
	void onAction(const event::Action &e) override {
		APP->patch->saveDialog();
	}
};


struct SaveAsItem : ui::MenuItem {
	SaveAsItem() {
		text = "Save as";
		rightText = WINDOW_MOD_CTRL_NAME "+Shift+S";
	}
	void onAction(const event::Action &e) override {
		APP->patch->saveAsDialog();
	}
};


struct SaveTemplateItem : ui::MenuItem {
	SaveTemplateItem() {
		text = "Save template";
	}
	void onAction(const event::Action &e) override {
		APP->patch->saveTemplateDialog();
	}
};


struct RevertItem : ui::MenuItem {
	RevertItem() {
		text = "Revert";
	}
	void onAction(const event::Action &e) override {
		APP->patch->revertDialog();
	}
};


struct DisconnectCablesItem : ui::MenuItem {
	DisconnectCablesItem() {
		text = "Disconnect cables";
	}
	void onAction(const event::Action &e) override {
		APP->patch->disconnectDialog();
	}
};


struct QuitItem : ui::MenuItem {
	QuitItem() {
		text = "Quit";
		rightText = WINDOW_MOD_CTRL_NAME "+Q";
	}
	void onAction(const event::Action &e) override {
		APP->window->close();
	}
};


struct FileButton : MenuButton {
	FileButton() {
		text = "File";
	}
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(new NewItem);
		menu->addChild(new OpenItem);
		menu->addChild(new SaveItem);
		menu->addChild(new SaveAsItem);
		menu->addChild(new SaveTemplateItem);
		menu->addChild(new RevertItem);
		menu->addChild(new DisconnectCablesItem);
		menu->addChild(new QuitItem);
	}
};


struct UndoItem : ui::MenuItem {
	UndoItem() {
		text = "Undo " + APP->history->getUndoName();
		rightText = WINDOW_MOD_CTRL_NAME "+Z";
		disabled = !APP->history->canUndo();
	}
	void onAction(const event::Action &e) override {
		APP->history->undo();
	}
};


struct RedoItem : ui::MenuItem {
	RedoItem() {
		text = "Redo " + APP->history->getRedoName();
		rightText = WINDOW_MOD_CTRL_NAME "+" WINDOW_MOD_SHIFT_NAME "+Z";
		disabled = !APP->history->canRedo();
	}
	void onAction(const event::Action &e) override {
		APP->history->redo();
	}
};


struct EditButton : MenuButton {
	EditButton() {
		text = "Edit";
	}
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(new UndoItem);
		menu->addChild(new RedoItem);
	}
};


struct ZoomQuantity : ui::Quantity {
	void setValue(float value) override {
		settings.zoom = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings.zoom;
	}
	float getMinValue() override {return 0.25;}
	float getMaxValue() override {return 2.0;}
	float getDefaultValue() override {return 1.0;}
	float getDisplayValue() override {return std::round(getValue() * 100);}
	void setDisplayValue(float displayValue) override {setValue(displayValue / 100);}
	std::string getLabel() override {return "Zoom";}
	std::string getUnit() override {return "%";}
};


struct CableOpacityQuantity : ui::Quantity {
	void setValue(float value) override {
		settings.cableOpacity = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings.cableOpacity;
	}
	float getDefaultValue() override {return 0.5;}
	float getDisplayValue() override {return getValue() * 100;}
	void setDisplayValue(float displayValue) override {setValue(displayValue / 100);}
	std::string getLabel() override {return "Cable opacity";}
	std::string getUnit() override {return "%";}
};



struct CableTensionQuantity : ui::Quantity {
	void setValue(float value) override {
		settings.cableTension = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings.cableTension;
	}
	float getDefaultValue() override {return 0.5;}
	std::string getLabel() override {return "Cable tension";}
	int getDisplayPrecision() override {return 2;}
};


struct CpuMeterItem : ui::MenuItem {
	CpuMeterItem() {
		text = "CPU meter";
		rightText = CHECKMARK(settings.cpuMeter);
	}
	void onAction(const event::Action &e) override {
		settings.cpuMeter ^= true;
	}
};


struct ParamTooltipItem : ui::MenuItem {
	ParamTooltipItem() {
		text = "Parameter tooltips";
		rightText = CHECKMARK(settings.paramTooltip);
	}
	void onAction(const event::Action &e) override {
		settings.paramTooltip ^= true;
	}
};


struct LockModulesItem : ui::MenuItem {
	LockModulesItem() {
		text = "Lock modules";
		rightText = CHECKMARK(settings.lockModules);
	}
	void onAction(const event::Action &e) override {
		settings.lockModules ^= true;
	}
};


struct EnginePauseItem : ui::MenuItem {
	EnginePauseItem() {
		text = "Pause engine";
		rightText = CHECKMARK(APP->engine->isPaused());
	}
	void onAction(const event::Action &e) override {
		APP->engine->setPaused(!APP->engine->isPaused());
	}
};


struct SampleRateValueItem : ui::MenuItem {
	float sampleRate;
	void setSampleRate(float sampleRate) {
		this->sampleRate = sampleRate;
		text = string::f("%.0f Hz", sampleRate);
		rightText = CHECKMARK(APP->engine->getSampleRate() == sampleRate);
	}
	void onAction(const event::Action &e) override {
		APP->engine->setSampleRate(sampleRate);
		APP->engine->setPaused(false);
	}
};


struct SampleRateItem : ui::MenuItem {
	SampleRateItem() {
		text = "Engine sample rate";
	}
	ui::Menu *createChildMenu() override {
		ui::Menu *menu = new ui::Menu;

		menu->addChild(new EnginePauseItem);

		for (int i = 0; i <= 4; i++) {
			int oversample = 1 << i;

			SampleRateValueItem *item = new SampleRateValueItem;
			item->setSampleRate(44100.f * oversample);
			if (oversample > 1)
				item->text += string::f(" (%dx)", oversample);
			menu->addChild(item);

			item = new SampleRateValueItem;
			item->setSampleRate(48000.f * oversample);
			if (oversample > 1)
				item->text += string::f(" (%dx)", oversample);
			menu->addChild(item);
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
			text += " (best performance)";
		else if (threadCount == 1)
			text += " (best efficiency)";
		rightText = CHECKMARK(APP->engine->getThreadCount() == threadCount);
	}
	void onAction(const event::Action &e) override {
		APP->engine->setThreadCount(threadCount);
	}
};


struct ThreadCount : ui::MenuItem {
	ThreadCount() {
		text = "Thread count";
	}
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
	FullscreenItem() {
		text = "Fullscreen";
		rightText = "F11";
		if (APP->window->isFullScreen())
			rightText = CHECKMARK_STRING " " + rightText;
	}
	void onAction(const event::Action &e) override {
		APP->window->setFullScreen(!APP->window->isFullScreen());
	}
};


struct SettingsButton : MenuButton {
	SettingsButton() {
		text = "Settings";
	}
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(new ParamTooltipItem);
		menu->addChild(new CpuMeterItem);
		menu->addChild(new LockModulesItem);
		menu->addChild(new SampleRateItem);
		menu->addChild(new ThreadCount);
		menu->addChild(new FullscreenItem);

		ui::Slider *zoomSlider = new ui::Slider;
		zoomSlider->box.size.x = 200.0;
		zoomSlider->quantity = new ZoomQuantity;
		menu->addChild(zoomSlider);

		ui::Slider *cableOpacitySlider = new ui::Slider;
		cableOpacitySlider->box.size.x = 200.0;
		cableOpacitySlider->quantity = new CableOpacityQuantity;
		menu->addChild(cableOpacitySlider);

		ui::Slider *cableTensionSlider = new ui::Slider;
		cableTensionSlider->box.size.x = 200.0;
		cableTensionSlider->quantity = new CableTensionQuantity;
		menu->addChild(cableTensionSlider);
	}
};


struct RegisterItem : ui::MenuItem {
	RegisterItem() {
		text = "Register VCV account";
	}
	void onAction(const event::Action &e) override {
		std::thread t([&]() {
			system::openBrowser("https://vcvrack.com/");
		});
		t.detach();
	}
};


struct AccountEmailField : ui::TextField {
	ui::TextField *passwordField;
	AccountEmailField() {
		placeholder = "Email";
	}
	void onSelectKey(const event::SelectKey &e) override {
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
	AccountPasswordField() {
		placeholder = "Password";
	}
	void onSelectKey(const event::SelectKey &e) override {
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
	LogInItem() {
		text = "Log in";
	}
	void onAction(const event::Action &e) override {
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
		text = "Manage plugins";
	}
	void onAction(const event::Action &e) override {
		std::thread t([&]() {
			system::openBrowser("https://vcvrack.com/plugins.html");
		});
		t.detach();
	}
};


struct SyncItem : ui::MenuItem {
	SyncItem() {
		text = "Sync plugins";
		disabled = true;
	}
	void onAction(const event::Action &e) override {
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
// 	void onAction(const event::Action &e) override {
// 		available = false;
// 		std::thread t([this]() {
// 			if (plugin::sync(false))
// 				completed = true;
// 		});
// 		t.detach();
// 	}
// };


struct LogOutItem : ui::MenuItem {
	LogOutItem() {
		text = "Log out";
	}
	void onAction(const event::Action &e) override {
		plugin::logOut();
	}
};


struct DownloadQuantity : ui::Quantity {
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
	PluginsButton() {
		text = "Plugins";
	}
	void onAction(const event::Action &e) override {
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
			menu->addChild(new ManageItem);
			menu->addChild(new SyncItem);
			menu->addChild(new LogOutItem);
		}
		else {
			menu->addChild(new RegisterItem);
			AccountEmailField *emailField = new AccountEmailField;
			emailField->box.size.x = 200.0;
			menu->addChild(emailField);
			AccountPasswordField *passwordField = new AccountPasswordField;
			passwordField->box.size.x = 200.0;
			emailField->passwordField = passwordField;
			menu->addChild(passwordField);
			LogInItem *logInItem = new LogInItem;
			logInItem->emailField = emailField;
			logInItem->passwordField = passwordField;
			passwordField->logInItem = logInItem;
			menu->addChild(logInItem);
		}
	}

	void draw(const widget::DrawContext &ctx) override {
		MenuButton::draw(ctx);
		// if (1) {
		// 	// Notification circle
		// 	nvgBeginPath(ctx.vg);
		// 	nvgCircle(ctx.vg, box.size.x - 3, 3, 4.0);
		// 	nvgFillColor(ctx.vg, nvgRGBf(1.0, 0.0, 0.0));
		// 	nvgFill(ctx.vg);
		// 	nvgStrokeColor(ctx.vg, nvgRGBf(0.5, 0.0, 0.0));
		// 	nvgStroke(ctx.vg);
		// }
	}
};


struct ManualItem : ui::MenuItem {
	ManualItem() {
		text = "Manual";
		rightText = "F1";
	}
	void onAction(const event::Action &e) override {
		std::thread t([&]() {
			system::openBrowser("https://vcvrack.com/manual/");
		});
		t.detach();
	}
};


struct WebsiteItem : ui::MenuItem {
	WebsiteItem() {
		text = "VCVRack.com";
	}
	void onAction(const event::Action &e) override {
		std::thread t([&]() {
			system::openBrowser("https://vcvrack.com/");
		});
		t.detach();
	}
};


struct CheckVersionItem : ui::MenuItem {
	CheckVersionItem() {
		text = "Check version on launch";
		rightText = CHECKMARK(settings.checkVersion);
	}
	void onAction(const event::Action &e) override {
		settings.checkVersion ^= true;
	}
};


struct HelpButton : MenuButton {
	HelpButton() {
		text = "Help";
	}
	void onAction(const event::Action &e) override {
		ui::Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(new ManualItem);
		menu->addChild(new WebsiteItem);
		menu->addChild(new CheckVersionItem);
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
	layout->addChild(fileButton);

	EditButton *editButton = new EditButton;
	layout->addChild(editButton);

	SettingsButton *settingsButton = new SettingsButton;
	layout->addChild(settingsButton);

	PluginsButton *pluginsButton = new PluginsButton;
	layout->addChild(pluginsButton);

	HelpButton *helpButton = new HelpButton;
	layout->addChild(helpButton);
}

void Toolbar::draw(const widget::DrawContext &ctx) {
	bndMenuBackground(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL);
	bndBevel(ctx.vg, 0.0, 0.0, box.size.x, box.size.y);

	widget::Widget::draw(ctx);
}


} // namespace app
} // namespace rack
