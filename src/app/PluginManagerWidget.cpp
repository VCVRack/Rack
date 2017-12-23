#include <thread>
#include "app.hpp"
#include "plugin.hpp"
#include "gui.hpp"
#include "../ext/osdialog/osdialog.h"


namespace rack {


struct SyncButton : Button {
	bool checked = false;
	bool available = false;
	bool completed = false;

	void step() override {
		if (!checked) {
			std::thread t([this]() {
				if (pluginSync(true))
					available = true;
			});
			t.detach();
			checked = true;
		}
		if (completed) {
			if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "All plugins have been updated. Close Rack and re-launch it to load new updates.")) {
				guiClose();
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


PluginManagerWidget::PluginManagerWidget() {
	box.size.y = BND_WIDGET_HEIGHT;
	float margin = 5;

	{
		loginWidget = new Widget();
		Vec pos = Vec(0, 0);

		struct RegisterButton : Button {
			void onAction(EventAction &e) override {
				std::thread t(openBrowser, "https://vcvrack.com/");
				t.detach();
			}
		};
		Button *registerButton = new RegisterButton();
		registerButton->box.pos = pos;
		registerButton->box.size.x = 75;
		registerButton->text = "Register";
		loginWidget->addChild(registerButton);
		pos.x += registerButton->box.size.x;

		pos.x += margin;
		TextField *emailField = new TextField();
		emailField->box.pos = pos;
		emailField->box.size.x = 175;
		emailField->placeholder = "Email";
		loginWidget->addChild(emailField);
		pos.x += emailField->box.size.x;

		pos.x += margin;
		PasswordField *passwordField = new PasswordField();
		passwordField->box.pos = pos;
		passwordField->box.size.x = 175;
		passwordField->placeholder = "Password";
		loginWidget->addChild(passwordField);
		pos.x += passwordField->box.size.x;

		struct LogInButton : Button {
			TextField *emailField;
			TextField *passwordField;
			void onAction(EventAction &e) override {
				std::thread t(pluginLogIn, emailField->text, passwordField->text);
				t.detach();
				passwordField->text = "";
			}
		};
		pos.x += margin;
		LogInButton *logInButton = new LogInButton();
		logInButton->box.pos = pos;
		logInButton->box.size.x = 100;
		logInButton->text = "Log in";
		logInButton->emailField = emailField;
		logInButton->passwordField = passwordField;
		loginWidget->addChild(logInButton);
		pos.x += logInButton->box.size.x;

		struct StatusLabel : Label {
			void step() override {
				text = pluginGetLoginStatus();
			}
		};
		Label *label = new StatusLabel();
		label->box.pos = pos;
		loginWidget->addChild(label);

		addChild(loginWidget);
	}

	{
		manageWidget = new Widget();
		Vec pos = Vec(0, 0);

		struct ManageButton : Button {
			void onAction(EventAction &e) override {
				std::thread t(openBrowser, "https://vcvrack.com/");
				t.detach();
			}
		};
		Button *manageButton = new ManageButton();
		manageButton->box.pos = pos;
		manageButton->box.size.x = 125;
		manageButton->text = "Manage plugins";
		manageWidget->addChild(manageButton);
		pos.x += manageButton->box.size.x;

		pos.x += margin;
		Button *syncButton = new SyncButton();
		syncButton->box.pos = pos;
		syncButton->box.size.x = 125;
		syncButton->text = "Update plugins";
		manageWidget->addChild(syncButton);
		pos.x += syncButton->box.size.x;

		struct LogOutButton : Button {
			void onAction(EventAction &e) override {
				pluginLogOut();
			}
		};
		pos.x += margin;
		Button *logOutButton = new LogOutButton();
		logOutButton->box.pos = pos;
		logOutButton->box.size.x = 100;
		logOutButton->text = "Log out";
		manageWidget->addChild(logOutButton);

		addChild(manageWidget);
	}

	{
		downloadWidget = new Widget();
		Vec pos = Vec(0, 0);

		struct DownloadProgressBar : ProgressBar {
			void step() override {
				label = "Downloading";
				std::string name = pluginGetDownloadName();
				if (name != "")
					label += " " + name;
				setValue(100.0 * pluginGetDownloadProgress());
			}
		};
		ProgressBar *downloadProgress = new DownloadProgressBar();
		downloadProgress->box.pos = pos;
		downloadProgress->box.size.x = 300;
		downloadProgress->setLimits(0, 100);
		downloadProgress->unit = "%";
		downloadWidget->addChild(downloadProgress);
		pos.x += downloadProgress->box.size.x;

		// struct CancelButton : Button {
		// 	void onAction(EventAction &e) override {
		// 		pluginCancelDownload();
		// 	}
		// };
		// pos.x += margin;
		// Button *logOutButton = new CancelButton();
		// logOutButton->box.pos = pos;
		// logOutButton->box.size.x = 100;
		// logOutButton->text = "Cancel";
		// downloadWidget->addChild(logOutButton);

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
