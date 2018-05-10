#include <thread>
#include "app.hpp"
#include "plugin.hpp"
#include "window.hpp"
#include "osdialog.h"


namespace rack {


struct RegisterButton : Button {
	void onAction(EventAction &e) override {
		std::thread t([&]() {
			systemOpenBrowser("https://vcvrack.com/");
		});
		t.detach();
	}
};


struct LogInButton : Button {
	TextField *emailField;
	TextField *passwordField;
	void onAction(EventAction &e) override {
		std::thread t(pluginLogIn, emailField->text, passwordField->text);
		t.detach();
		passwordField->text = "";
	}
};


struct StatusLabel : Label {
	void step() override {
		text = pluginGetLoginStatus();
	}
};


struct ManageButton : Button {
	void onAction(EventAction &e) override {
		std::thread t([&]() {
			systemOpenBrowser("https://vcvrack.com/plugins.html");
		});
		t.detach();
	}
};


struct SyncButton : Button {
	bool checked = false;
	/** Updates are available */
	bool available = false;
	/** Plugins have been updated */
	bool completed = false;

	void step() override {
		// Check for plugin update on first step()
		if (!checked) {
			std::thread t([this]() {
				if (pluginSync(true))
					available = true;
			});
			t.detach();
			checked = true;
		}
		// Display message if we've completed updates
		if (completed) {
			if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "All plugins have been updated. Close Rack and re-launch it to load new updates.")) {
				windowClose();
			}
			completed = false;
		}
	}
	void draw(NVGcontext *vg) override {
		Button::draw(vg);
		if (available) {
			// Notification circle
			nvgBeginPath(vg);
			nvgCircle(vg, 3, 3, 4.0);
			nvgFillColor(vg, nvgRGBf(1.0, 0.0, 0.0));
			nvgFill(vg);
			nvgStrokeColor(vg, nvgRGBf(0.5, 0.0, 0.0));
			nvgStroke(vg);
		}
	}
	void onAction(EventAction &e) override {
		available = false;
		std::thread t([this]() {
			if (pluginSync(false))
				completed = true;
		});
		t.detach();
	}
};


struct LogOutButton : Button {
	void onAction(EventAction &e) override {
		pluginLogOut();
	}
};


struct DownloadProgressBar : ProgressBar {
	void step() override {
		label = "Downloading";
		std::string name = pluginGetDownloadName();
		if (name != "")
			label += " " + name;
		setValue(100.0 * pluginGetDownloadProgress());
	}
};


struct CancelButton : Button {
	void onAction(EventAction &e) override {
		pluginCancelDownload();
	}
};


PluginManagerWidget::PluginManagerWidget() {
	box.size.y = BND_WIDGET_HEIGHT;

	{
		SequentialLayout *layout = Widget::create<SequentialLayout>(Vec(0, 0));
		layout->spacing = 5;
		loginWidget = layout;

		Button *registerButton = new RegisterButton();
		registerButton->box.size.x = 75;
		registerButton->text = "Register";
		loginWidget->addChild(registerButton);

		TextField *emailField = new TextField();
		emailField->box.size.x = 175;
		emailField->placeholder = "Email";
		loginWidget->addChild(emailField);

		PasswordField *passwordField = new PasswordField();
		passwordField->box.size.x = 175;
		passwordField->placeholder = "Password";
		loginWidget->addChild(passwordField);

		LogInButton *logInButton = new LogInButton();
		logInButton->box.size.x = 100;
		logInButton->text = "Log in";
		logInButton->emailField = emailField;
		logInButton->passwordField = passwordField;
		loginWidget->addChild(logInButton);

		Label *label = new StatusLabel();
		loginWidget->addChild(label);

		addChild(loginWidget);
	}

	{
		SequentialLayout *layout = Widget::create<SequentialLayout>(Vec(0, 0));
		layout->spacing = 5;
		manageWidget = layout;

		Button *manageButton = new ManageButton();
		manageButton->box.size.x = 125;
		manageButton->text = "Manage plugins";
		manageWidget->addChild(manageButton);

		Button *syncButton = new SyncButton();
		syncButton->box.size.x = 125;
		syncButton->text = "Update plugins";
		manageWidget->addChild(syncButton);

		Button *logOutButton = new LogOutButton();
		logOutButton->box.size.x = 100;
		logOutButton->text = "Log out";
		manageWidget->addChild(logOutButton);

		addChild(manageWidget);
	}

	{
		SequentialLayout *layout = Widget::create<SequentialLayout>(Vec(0, 0));
		layout->spacing = 5;
		downloadWidget = layout;

		ProgressBar *downloadProgress = new DownloadProgressBar();
		downloadProgress->box.size.x = 300;
		downloadProgress->setLimits(0, 100);
		downloadProgress->unit = "%";
		downloadWidget->addChild(downloadProgress);

		// Button *cancelButton = new CancelButton();
		// cancelButton->box.size.x = 100;
		// cancelButton->text = "Cancel";
		// downloadWidget->addChild(cancelButton);

		addChild(downloadWidget);
	}
}

void PluginManagerWidget::step() {
	loginWidget->visible = false;
	manageWidget->visible = false;
	downloadWidget->visible = false;

	if (pluginIsDownloading())
		downloadWidget->visible = true;
	else if (pluginIsLoggedIn())
		manageWidget->visible = true;
	else
		loginWidget->visible = true;

	Widget::step();
}


} // namespace rack
