#include <thread>
#include "system.hpp"
#include "app/PluginManagerWidget.hpp"
#include "ui/SequentialLayout.hpp"
#include "ui/Button.hpp"
#include "ui/ProgressBar.hpp"
#include "ui/TextField.hpp"
#include "ui/PasswordField.hpp"
#include "ui/Label.hpp"
#include "plugin/PluginManager.hpp"
#include "context.hpp"
#include "window.hpp"
#include "helpers.hpp"
#include "osdialog.h"


namespace rack {


struct RegisterButton : Button {
	void onAction(event::Action &e) override {
		std::thread t([&]() {
			system::openBrowser("https://vcvrack.com/");
		});
		t.detach();
	}
};


struct LogInButton : Button {
	TextField *emailField;
	TextField *passwordField;
	void onAction(event::Action &e) override {
		std::thread t([&]() {
			context()->plugin->logIn(emailField->text, passwordField->text);
		});
		t.detach();
		passwordField->text = "";
	}
};


struct StatusLabel : Label {
	void step() override {
		text = context()->plugin->loginStatus;
	}
};


struct ManageButton : Button {
	void onAction(event::Action &e) override {
		std::thread t([&]() {
			system::openBrowser("https://vcvrack.com/plugins.html");
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
				if (context()->plugin->sync(true))
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
	void onAction(event::Action &e) override {
		available = false;
		std::thread t([this]() {
			if (context()->plugin->sync(false))
				completed = true;
		});
		t.detach();
	}
};


struct LogOutButton : Button {
	void onAction(event::Action &e) override {
		context()->plugin->logOut();
	}
};


struct DownloadQuantity : Quantity {
	float getValue() override {
		return context()->plugin->downloadProgress;
	}

	float getDisplayValue() override {
		return getValue() * 100.f;
	}

	int getDisplayPrecision() override {return 0;}

	std::string getLabel() override {
		return "Downloading " + context()->plugin->downloadName;
	}

	std::string getUnit() override {return "%";}
};


struct DownloadProgressBar : ProgressBar {
	DownloadProgressBar() {
		quantity = new DownloadQuantity;
	}
};


struct CancelButton : Button {
	void onAction(event::Action &e) override {
		context()->plugin->cancelDownload();
	}
};


PluginManagerWidget::PluginManagerWidget() {
	box.size.y = BND_WIDGET_HEIGHT;

	{
		SequentialLayout *layout = createWidget<SequentialLayout>(Vec(0, 0));
		layout->spacing = 5;
		loginWidget = layout;

		Button *registerButton = new RegisterButton;
		registerButton->box.size.x = 75;
		registerButton->text = "Register";
		loginWidget->addChild(registerButton);

		TextField *emailField = new TextField;
		emailField->box.size.x = 175;
		emailField->placeholder = "Email";
		loginWidget->addChild(emailField);

		PasswordField *passwordField = new PasswordField;
		passwordField->box.size.x = 175;
		passwordField->placeholder = "Password";
		loginWidget->addChild(passwordField);

		LogInButton *logInButton = new LogInButton;
		logInButton->box.size.x = 100;
		logInButton->text = "Log in";
		logInButton->emailField = emailField;
		logInButton->passwordField = passwordField;
		loginWidget->addChild(logInButton);

		Label *label = new StatusLabel;
		loginWidget->addChild(label);

		addChild(loginWidget);
	}

	{
		SequentialLayout *layout = createWidget<SequentialLayout>(Vec(0, 0));
		layout->spacing = 5;
		manageWidget = layout;

		Button *manageButton = new ManageButton;
		manageButton->box.size.x = 125;
		manageButton->text = "Manage plugins";
		manageWidget->addChild(manageButton);

		Button *syncButton = new SyncButton;
		syncButton->box.size.x = 125;
		syncButton->text = "Update plugins";
		manageWidget->addChild(syncButton);

		Button *logOutButton = new LogOutButton;
		logOutButton->box.size.x = 100;
		logOutButton->text = "Log out";
		manageWidget->addChild(logOutButton);

		addChild(manageWidget);
	}

	{
		SequentialLayout *layout = createWidget<SequentialLayout>(Vec(0, 0));
		layout->spacing = 5;
		downloadWidget = layout;

		ProgressBar *downloadProgress = new DownloadProgressBar;
		downloadProgress->box.size.x = 300;
		downloadWidget->addChild(downloadProgress);

		// Button *cancelButton = new CancelButton;
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

	if (context()->plugin->isDownloading)
		downloadWidget->visible = true;
	else if (context()->plugin->isLoggedIn())
		manageWidget->visible = true;
	else
		loginWidget->visible = true;

	Widget::step();
}


} // namespace rack
