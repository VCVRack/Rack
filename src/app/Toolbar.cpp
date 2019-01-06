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
#include "app/Scene.hpp"
#include "context.hpp"
#include "settings.hpp"
#include "helpers.hpp"
#include "system.hpp"
#include "plugin.hpp"
#include <thread>


namespace rack {


struct MenuButton : Button {
	void step() override {
		box.size.x = bndLabelWidth(context()->window->vg, -1, text.c_str());
		Widget::step();
	}
	void draw(NVGcontext *vg) override {
		bndMenuItem(vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());
	}
};


struct NewItem : MenuItem {
	NewItem() {
		text = "New";
		rightText = "(" WINDOW_MOD_KEY_NAME "+N)";
	}
	void onAction(const event::Action &e) override {
		context()->scene->rackWidget->reset();
	}
};


struct OpenItem : MenuItem {
	OpenItem() {
		text = "Open";
		rightText = "(" WINDOW_MOD_KEY_NAME "+O)";
	}
	void onAction(const event::Action &e) override {
		context()->scene->rackWidget->loadDialog();
	}
};


struct SaveItem : MenuItem {
	SaveItem() {
		text = "Save";
		rightText = "(" WINDOW_MOD_KEY_NAME "+S)";
	}
	void onAction(const event::Action &e) override {
		context()->scene->rackWidget->saveDialog();
	}
};


struct SaveAsItem : MenuItem {
	SaveAsItem() {
		text = "Save as";
		rightText = "(" WINDOW_MOD_KEY_NAME "+Shift+S)";
	}
	void onAction(const event::Action &e) override {
		context()->scene->rackWidget->saveAsDialog();
	}
};


struct SaveTemplateItem : MenuItem {
	SaveTemplateItem() {
		text = "Save template";
	}
	void onAction(const event::Action &e) override {
		context()->scene->rackWidget->saveTemplate();
	}
};


struct RevertItem : MenuItem {
	RevertItem() {
		text = "Revert";
	}
	void onAction(const event::Action &e) override {
		context()->scene->rackWidget->revert();
	}
};


struct DisconnectCablesItem : MenuItem {
	DisconnectCablesItem() {
		text = "Disconnect cables";
	}
	void onAction(const event::Action &e) override {
		context()->scene->rackWidget->disconnect();
	}
};


struct QuitItem : MenuItem {
	QuitItem() {
		text = "Quit";
		rightText = "(" WINDOW_MOD_KEY_NAME "+Q)";
	}
	void onAction(const event::Action &e) override {
		context()->window->close();
	}
};


struct FileButton : MenuButton {
	FileButton() {
		text = "File";
	}
	void onAction(const event::Action &e) override {
		Menu *menu = createMenu();
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
	float getDisplayValue() override {return getValue() * 100.0;}
	void setDisplayValue(float displayValue) override {setValue(displayValue / 100.0);}
	std::string getLabel() override {return "Zoom";}
	std::string getUnit() override {return "%";}
	int getDisplayPrecision() override {return 0;}
};


struct WireOpacityQuantity : Quantity {
	void setValue(float value) override {
		settings::wireOpacity = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::wireOpacity;
	}
	float getDefaultValue() override {return 0.5;}
	float getDisplayValue() override {return getValue() * 100.0;}
	void setDisplayValue(float displayValue) override {setValue(displayValue / 100.0);}
	std::string getLabel() override {return "Cable opacity";}
	std::string getUnit() override {return "%";}
	int getDisplayPrecision() override {return 0;}
};



struct WireTensionQuantity : Quantity {
	void setValue(float value) override {
		settings::wireTension = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::wireTension;
	}
	float getDefaultValue() override {return 0.5;}
	std::string getLabel() override {return "Cable tension";}
	int getDisplayPrecision() override {return 2;}
};


struct PowerMeterItem : MenuItem {
	PowerMeterItem() {
		text = "Power meter";
		rightText = CHECKMARK(settings::powerMeter);
	}
	void onAction(const event::Action &e) override {
		settings::powerMeter ^= true;
	}
};


struct LockModulesItem : MenuItem {
	LockModulesItem() {
		text = "Lock modules";
		rightText = CHECKMARK(settings::lockModules);
	}
	void onAction(const event::Action &e) override {
		settings::lockModules ^= true;
	}
};


struct EnginePauseItem : MenuItem {
	EnginePauseItem() {
		text = "Pause engine";
		rightText = CHECKMARK(context()->engine->paused);
	}
	void onAction(const event::Action &e) override {
		context()->engine->paused ^= true;
	}
};


struct SampleRateValueItem : MenuItem {
	float sampleRate;
	SampleRateValueItem(float sampleRate) {
		this->sampleRate = sampleRate;
		text = string::f("%.0f Hz", sampleRate);
		rightText = CHECKMARK(context()->engine->getSampleRate() == sampleRate);
	}
	void onAction(const event::Action &e) override {
		context()->engine->setSampleRate(sampleRate);
		context()->engine->paused = false;
	}
};


struct SampleRateItem : MenuItem {
	SampleRateItem() {
		text = "Engine sample rate";
	}
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		menu->addChild(new EnginePauseItem);

		std::vector<float> sampleRates = {44100, 48000, 88200, 96000, 176400, 192000};
		for (float sampleRate : sampleRates) {
			menu->addChild(new SampleRateValueItem(sampleRate));
		}
		return menu;
	}
};


struct SettingsButton : MenuButton {
	SettingsButton() {
		text = "Settings";
	}
	void onAction(const event::Action &e) override {
		Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		menu->addChild(new PowerMeterItem);
		menu->addChild(new LockModulesItem);
		menu->addChild(new SampleRateItem);

		Slider *zoomSlider = new Slider;
		zoomSlider->box.size.x = 200.0;
		zoomSlider->quantity = new ZoomQuantity;
		menu->addChild(zoomSlider);

		Slider *wireOpacitySlider = new Slider;
		wireOpacitySlider->box.size.x = 200.0;
		wireOpacitySlider->quantity = new WireOpacityQuantity;
		menu->addChild(wireOpacitySlider);

		Slider *wireTensionSlider = new Slider;
		wireTensionSlider->box.size.x = 200.0;
		wireTensionSlider->quantity = new WireTensionQuantity;
		menu->addChild(wireTensionSlider);
	}
};


struct RegisterItem : MenuItem {
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


struct AccountEmailField : TextField {
	TextField *passwordField;
	AccountEmailField() {
		placeholder = "Email";
	}
	void onSelectKey(const event::SelectKey &e) override {
		if (e.action == GLFW_PRESS && e.key == GLFW_KEY_TAB) {
			context()->event->selectedWidget = passwordField;
			e.consume(this);
			return;
		}
		TextField::onSelectKey(e);
	}
};


struct AccountPasswordField : PasswordField {
	MenuItem *logInItem;
	AccountPasswordField() {
		placeholder = "Password";
	}
	void onSelectKey(const event::SelectKey &e) override {
		if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
			logInItem->doAction();
			e.consume(this);
			return;
		}
		PasswordField::onSelectKey(e);
	}
};


struct LogInItem : MenuItem {
	TextField *emailField;
	TextField *passwordField;
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


struct ManageItem : MenuItem {
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


struct SyncItem : MenuItem {
	SyncItem() {
		text = "Sync plugins";
		disabled = true;
	}
	void onAction(const event::Action &e) override {
	}
};


// struct SyncButton : Button {
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
// 				context()->window->close();
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


struct LogOutItem : MenuItem {
	LogOutItem() {
		text = "Log out";
	}
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


struct PluginsButton : MenuButton {
	PluginsButton() {
		text = "Plugins";
	}
	void onAction(const event::Action &e) override {
		Menu *menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		// TODO Design dialog box for plugin syncing
		if (plugin::isDownloading) {
			ProgressBar *downloadProgressBar = new ProgressBar;
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

	void draw(NVGcontext *vg) override {
		MenuButton::draw(vg);
		// if (1) {
		// 	// Notification circle
		// 	nvgBeginPath(vg);
		// 	nvgCircle(vg, box.size.x - 3, 3, 4.0);
		// 	nvgFillColor(vg, nvgRGBf(1.0, 0.0, 0.0));
		// 	nvgFill(vg);
		// 	nvgStrokeColor(vg, nvgRGBf(0.5, 0.0, 0.0));
		// 	nvgStroke(vg);
		// }
	}
};


struct ManualItem : MenuItem {
	ManualItem() {
		text = "Manual";
	}
	void onAction(const event::Action &e) override {
		std::thread t([&]() {
			system::openBrowser("https://vcvrack.com/manual/");
		});
		t.detach();
	}
};


struct WebsiteItem : MenuItem {
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


struct CheckVersionItem : MenuItem {
	CheckVersionItem() {
		text = "Check version on launch";
		rightText = CHECKMARK(settings::checkVersion);
	}
	void onAction(const event::Action &e) override {
		settings::checkVersion ^= true;
	}
};


struct HelpButton : MenuButton {
	HelpButton() {
		text = "Help";
	}
	void onAction(const event::Action &e) override {
		Menu *menu = createMenu();
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

	SequentialLayout *layout = new SequentialLayout;
	layout->box.pos = math::Vec(margin, margin);
	layout->spacing = math::Vec(0, 0);
	addChild(layout);

	FileButton *fileButton = new FileButton;
	layout->addChild(fileButton);

	SettingsButton *settingsButton = new SettingsButton;
	layout->addChild(settingsButton);

	PluginsButton *pluginsButton = new PluginsButton;
	layout->addChild(pluginsButton);

	HelpButton *helpButton = new HelpButton;
	layout->addChild(helpButton);
}

void Toolbar::draw(NVGcontext *vg) {
	bndMenuBackground(vg, 0.0, 0.0, box.size.x, box.size.y, 0);
	bndBevel(vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(vg);
}


} // namespace rack
