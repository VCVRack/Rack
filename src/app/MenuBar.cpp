#include <app/MenuBar.hpp>
#include <window.hpp>
#include <engine/Engine.hpp>
#include <asset.hpp>
#include <ui/Button.hpp>
#include <ui/MenuItem.hpp>
#include <ui/SequentialLayout.hpp>
#include <ui/Slider.hpp>
#include <ui/TextField.hpp>
#include <ui/PasswordField.hpp>
#include <ui/ProgressBar.hpp>
#include <app.hpp>
#include <settings.hpp>
#include <helpers.hpp>
#include <system.hpp>
#include <plugin.hpp>
#include <patch.hpp>
#include <updater.hpp>
#include <osdialog.h>
#include <thread>


namespace rack {
namespace app {


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
	void onAction(const event::Action& e) override {
		std::thread t([ = ] {
			system::openBrowser(url);
		});
		t.detach();
	}
};

struct FolderItem : ui::MenuItem {
	std::string path;
	void onAction(const event::Action& e) override {
		std::thread t([ = ] {
			system::openFolder(path);
		});
		t.detach();
	}
};

////////////////////
// File
////////////////////

struct NewItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->patch->resetDialog();
	}
};

struct OpenItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->patch->loadDialog();
	}
};

struct SaveItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->patch->saveDialog();
	}
};

struct SaveAsItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->patch->saveAsDialog();
	}
};

struct SaveTemplateItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->patch->saveTemplateDialog();
	}
};

struct RevertItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->patch->revertDialog();
	}
};

struct QuitItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->window->close();
	}
};

struct FileButton : MenuButton {
	void onAction(const event::Action& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		NewItem* newItem = new NewItem;
		newItem->text = "New";
		newItem->rightText = RACK_MOD_CTRL_NAME "+N";
		menu->addChild(newItem);

		OpenItem* openItem = new OpenItem;
		openItem->text = "Open";
		openItem->rightText = RACK_MOD_CTRL_NAME "+O";
		menu->addChild(openItem);

		SaveItem* saveItem = new SaveItem;
		saveItem->text = "Save";
		saveItem->rightText = RACK_MOD_CTRL_NAME "+S";
		menu->addChild(saveItem);

		SaveAsItem* saveAsItem = new SaveAsItem;
		saveAsItem->text = "Save as";
		saveAsItem->rightText = RACK_MOD_CTRL_NAME "+Shift+S";
		menu->addChild(saveAsItem);

		SaveTemplateItem* saveTemplateItem = new SaveTemplateItem;
		saveTemplateItem->text = "Save template";
		menu->addChild(saveTemplateItem);

		RevertItem* revertItem = new RevertItem;
		revertItem->text = "Revert";
		revertItem->rightText = RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME "+O";
		menu->addChild(revertItem);

		QuitItem* quitItem = new QuitItem;
		quitItem->text = "Quit";
		quitItem->rightText = RACK_MOD_CTRL_NAME "+Q";
		menu->addChild(quitItem);
	}
};

////////////////////
// Edit
////////////////////

struct UndoItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->history->undo();
	}
};

struct RedoItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->history->redo();
	}
};

struct DisconnectCablesItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->patch->disconnectDialog();
	}
};

struct EditButton : MenuButton {
	void onAction(const event::Action& e) override {
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
	float getMinValue() override {
		return -2.0;
	}
	float getMaxValue() override {
		return 2.0;
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

struct ParamTooltipItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		settings::paramTooltip ^= true;
	}
};

struct LockModulesItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		settings::lockModules ^= true;
	}
};

struct CursorLockItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		settings::allowCursorLock ^= true;
	}
};

struct FullscreenItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->window->setFullScreen(!APP->window->isFullScreen());
	}
};

struct ViewButton : MenuButton {
	void onAction(const event::Action& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		ParamTooltipItem* paramTooltipItem = new ParamTooltipItem;
		paramTooltipItem->text = "Parameter tooltips";
		paramTooltipItem->rightText = CHECKMARK(settings::paramTooltip);
		menu->addChild(paramTooltipItem);

		LockModulesItem* lockModulesItem = new LockModulesItem;
		lockModulesItem->text = "Lock modules";
		lockModulesItem->rightText = CHECKMARK(settings::lockModules);
		menu->addChild(lockModulesItem);

		CursorLockItem* cursorLockItem = new CursorLockItem;
		cursorLockItem->text = "Lock cursor while dragging";
		cursorLockItem->rightText = CHECKMARK(settings::allowCursorLock);
		menu->addChild(cursorLockItem);

		ZoomSlider* zoomSlider = new ZoomSlider;
		zoomSlider->box.size.x = 200.0;
		menu->addChild(zoomSlider);

		CableOpacitySlider* cableOpacitySlider = new CableOpacitySlider;
		cableOpacitySlider->box.size.x = 200.0;
		menu->addChild(cableOpacitySlider);

		CableTensionSlider* cableTensionSlider = new CableTensionSlider;
		cableTensionSlider->box.size.x = 200.0;
		menu->addChild(cableTensionSlider);

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
	void onAction(const event::Action& e) override {
		settings::cpuMeter ^= true;
	}
};

struct EnginePauseItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		APP->engine->setPaused(!APP->engine->isPaused());
	}
};

struct SampleRateValueItem : ui::MenuItem {
	float sampleRate;
	void onAction(const event::Action& e) override {
		settings::sampleRate = sampleRate;
		APP->engine->setPaused(false);
	}
};

struct SampleRateItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		EnginePauseItem* enginePauseItem = new EnginePauseItem;
		enginePauseItem->text = "Pause";
		enginePauseItem->rightText = CHECKMARK(APP->engine->isPaused());
		menu->addChild(enginePauseItem);

		for (int i = 0; i <= 4; i++) {
			for (int j = 0; j < 2; j++) {
				int oversample = 1 << i;
				float sampleRate = (j == 0) ? 44100.f : 48000.f;
				sampleRate *= oversample;

				SampleRateValueItem* item = new SampleRateValueItem;
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
	void onAction(const event::Action& e) override {
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
	void onAction(const event::Action& e) override {
		settings::threadCount = threadCount;
	}
};

struct ThreadCountItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		RealTimeItem* realTimeItem = new RealTimeItem;
		realTimeItem->text = "Real-time priority";
		realTimeItem->rightText = CHECKMARK(settings::realTime);
		menu->addChild(realTimeItem);

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
	void onAction(const event::Action& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		CpuMeterItem* cpuMeterItem = new CpuMeterItem;
		cpuMeterItem->text = "CPU meter";
		cpuMeterItem->rightText = CHECKMARK(settings::cpuMeter);
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
	void onSelectKey(const event::SelectKey& e) override {
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

	void onSelectKey(const event::SelectKey& e) override {
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
	void onAction(const event::Action& e) override {
		isLoggingIn = true;
		std::string email = emailField->text;
		std::string password = passwordField->text;
		std::thread t([ = ] {
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
		MenuItem::step();
	}
};

struct SyncItem : ui::MenuItem {
	void step() override {
		disabled = true;
		if (plugin::updateStatus != "") {
			text = plugin::updateStatus;
		}
		else if (plugin::isSyncing()) {
			text = "Updating...";
		}
		else if (!plugin::hasUpdates()) {
			text = "Up-to-date";
		}
		else {
			text = "Update all";
			disabled = false;
		}
		MenuItem::step();
	}

	void onAction(const event::Action& e) override {
		std::thread t([ = ] {
			plugin::syncUpdates();
		});
		t.detach();
		e.consume(NULL);
	}
};

struct PluginSyncItem : ui::MenuItem {
	plugin::Update* update;

	void setUpdate(plugin::Update* update) {
		this->update = update;
		text = update->pluginName;
		plugin::Plugin* p = plugin::getPlugin(update->pluginSlug);
		if (p) {
			rightText += "v" + p->version + " → ";
		}
		rightText += "v" + update->version;
	}

	ui::Menu* createChildMenu() override {
		if (update->changelogUrl != "") {
			ui::Menu* menu = new ui::Menu;

			UrlItem* changelogUrl = new UrlItem;
			changelogUrl->text = "Changelog";
			changelogUrl->url = update->changelogUrl;
			menu->addChild(changelogUrl);

			return menu;
		}
		return NULL;
	}

	void step() override {
		disabled = plugin::isSyncing();
		if (update->progress >= 1) {
			rightText = CHECKMARK_STRING;
			disabled = true;
		}
		else if (update->progress > 0) {
			rightText = string::f("%.0f%%", update->progress * 100.f);
		}
		MenuItem::step();
	}

	void onAction(const event::Action& e) override {
		std::thread t([ = ] {
			plugin::syncUpdate(update);
		});
		t.detach();
		e.consume(NULL);
	}
};

struct LogOutItem : ui::MenuItem {
	void onAction(const event::Action& e) override {
		plugin::logOut();
	}
};

struct LibraryMenu : ui::Menu {
	bool loggedIn = false;

	LibraryMenu() {
		refresh();
	}

	void step() override {
		// Refresh menu when appropriate
		if (!loggedIn && plugin::isLoggedIn())
			refresh();
		Menu::step();
	}

	void refresh() {
		setChildMenu(NULL);
		clearChildren();

		if (settings::devMode) {
			addChild(createMenuLabel("Disabled in development mode"));
		}
		else if (!plugin::isLoggedIn()) {
			UrlItem* registerItem = new UrlItem;
			registerItem->text = "Register VCV account";
			registerItem->url = "https://vcvrack.com/";
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

			UrlItem* manageItem = new UrlItem;
			manageItem->text = "Manage plugins";
			manageItem->url = "https://vcvrack.com/plugins.html";
			addChild(manageItem);

			LogOutItem* logOutItem = new LogOutItem;
			logOutItem->text = "Log out";
			addChild(logOutItem);

			SyncItem* syncItem = new SyncItem;
			syncItem->text = "Update all";
			addChild(syncItem);

			if (plugin::hasUpdates()) {
				addChild(new ui::MenuEntry);

				ui::MenuLabel* updatesLabel = new ui::MenuLabel;
				updatesLabel->text = "Updates";
				addChild(updatesLabel);

				for (plugin::Update& update : plugin::updates) {
					PluginSyncItem* updateItem = new PluginSyncItem;
					updateItem->setUpdate(&update);
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

	void onAction(const event::Action& e) override {
		ui::Menu* menu = createMenu<LibraryMenu>();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;
	}

	void step() override {
		notification->box.pos = math::Vec(0, 0);
		notification->visible = plugin::hasUpdates();

		// Popup when updates finish downloading
		if (plugin::restartRequested) {
			plugin::restartRequested = false;
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

struct UpdateItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		UrlItem* changelogUrl = new UrlItem;
		changelogUrl->text = "Changelog";
		changelogUrl->url = updater::changelogUrl;
		menu->addChild(changelogUrl);

		return menu;
	}

	void step() override {
		if (updater::progress > 0) {
			rightText = string::f("%.0f%%", updater::progress * 100.f);
		}
		MenuItem::step();
	}

	void onAction(const event::Action& e) override {
		std::thread t([ = ] {
			updater::update();
		});
		t.detach();
		e.consume(NULL);
	}
};


struct HelpButton : MenuButton {
	NotificationIcon* notification;

	HelpButton() {
		notification = new NotificationIcon;
		addChild(notification);
	}

	void onAction(const event::Action& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		if (updater::isUpdateAvailable()) {
			UpdateItem* updateItem = new UpdateItem;
			updateItem->text = "Update " + APP_NAME;
			updateItem->rightText = APP_VERSION + " → " + updater::version;
			menu->addChild(updateItem);
		}

		UrlItem* manualItem = new UrlItem;
		manualItem->text = "Manual";
		manualItem->rightText = "F1";
		manualItem->url = "https://vcvrack.com/manual/";
		menu->addChild(manualItem);

		UrlItem* websiteItem = new UrlItem;
		websiteItem->text = "VCVRack.com";
		websiteItem->url = "https://vcvrack.com/";
		menu->addChild(websiteItem);

		FolderItem* folderItem = new FolderItem;
		folderItem->text = "Open user folder";
		folderItem->path = asset::user("");
		menu->addChild(folderItem);
	}

	void step() override {
		notification->box.pos = math::Vec(0, 0);
		notification->visible = updater::isUpdateAvailable();
		MenuButton::step();
	}
};

////////////////////
// MenuBar
////////////////////

void MenuBar::draw(const DrawArgs& args) {
	bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL);
	bndBevel(args.vg, 0.0, 0.0, box.size.x, box.size.y);

	Widget::draw(args);
}


MenuBar* createMenuBar() {
	MenuBar* menuBar = new MenuBar;

	const float margin = 5;
	menuBar->box.size.y = BND_WIDGET_HEIGHT + 2 * margin;

	ui::SequentialLayout* layout = new ui::SequentialLayout;
	layout->box.pos = math::Vec(margin, margin);
	layout->spacing = math::Vec(0, 0);
	menuBar->addChild(layout);

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

	return menuBar;
}


} // namespace app
} // namespace rack
