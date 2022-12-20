#include <thread>

#include <app/TipWindow.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/MenuOverlay.hpp>
#include <ui/Label.hpp>
#include <ui/Button.hpp>
#include <ui/OptionButton.hpp>
#include <ui/MenuItem.hpp>
#include <ui/SequentialLayout.hpp>
#include <settings.hpp>
#include <system.hpp>


namespace rack {
namespace app {


struct UrlButton : ui::Button {
	std::string url;
	void onAction(const ActionEvent& e) override {
		system::openBrowser(url);
	}
};


struct TipInfo {
	std::string text;
	std::string linkText;
	std::string linkUrl;
};


// Remember to use “smart quotes.”
static const std::vector<TipInfo> tipInfos = {
	{"To add a module to your patch, right-click an empty rack space or press Enter. Then click and drag a module from the Module Browser into the desired rack space.\n\nTo select multiple modules, click and drag on empty rack space.", "", ""},
	{"To move around your patch, use the scroll bars, drag while holding the middle mouse button, " RACK_MOD_ALT_NAME "+click and drag, or hold the arrow keys. Arrow key movement speed can be adjusted by holding " RACK_MOD_CTRL_NAME ", " RACK_MOD_SHIFT_NAME ", or " RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME ".\n\nTo zoom in and out, drag the Zoom slider in the View menu, hold " RACK_MOD_CTRL_NAME " and scroll, or press " RACK_MOD_CTRL_NAME "+= and " RACK_MOD_CTRL_NAME "+minus.", "", ""},
	{"You can use Rack in fullscreen mode by selecting “View > Fullscreen“ or pressing F11.\n\nIn fullscreen mode, the menu bar and scroll bars are hidden. This is ideal for screen recording with VCV Recorder.", "Get VCV Recorder", "https://vcvrack.com/Recorder"},
	{"You can browse thousands of modules on the VCV Library website.\n\nRegister for a VCV account, log into Rack using the Library menu, and browse the VCV Library to add or purchase modules. Keep all plugins up to date by clicking “Library > Update all”.", "VCV Library", "https://library.vcvrack.com/"},
	{"Some developers of free plugins accept donations. Right-click your favorite module's panel and select “Info > Donate”.\n\nYou can also donate via the module's VCV Library page.", "VCV Library", "https://library.vcvrack.com/"},
	{"Want to use VCV Rack in your DAW? VCV Rack Pro is available for VST2, VST3, Audio Unit, and CLAP hosts.\n\nSupported DAWs include Ableton Live, Cubase, FL Studio, Reason, Bitwig, Reaper, Mixbus, Studio One, Cakewalk, Logic Pro, and GarageBand.", "Learn more", "https://vcvrack.com/Rack"},
	{"You can learn more about VCV Rack by browsing the official manual.", "VCV Rack manual", "https://vcvrack.com/manual/"},
	{"Follow VCV Rack on Twitter for new module announcements, development news, and featured artists/music.", "Twitter @vcvrack", "https://twitter.com/vcvrack"},
	{"Patch cables in Rack can carry up to 16 signals. You can use this ability to build polyphonic patches using modules having the “Polyphonic” tag. Cables carrying more than 1 signal appear thicker than normal cables. To try out polyphony, add the VCV MIDI-to-CV module to your patch, right-click its panel, and select your desired number of polyphonic channels.", "Learn more about polyphony in VCV Rack", "https://vcvrack.com/manual/Polyphony"},
	{"Know C++ programming and want to create your own modules for Rack? Developing Rack modules is a great way to learn digital signal processing and quickly test your ideas with an easy-to-learn platform.\n\nDownload the Rack SDK and follow the development tutorial to get started.", "Plugin Development Tutorial", "https://vcvrack.com/manual/PluginDevelopmentTutorial"},
	{"Wondering how to use a particular module? Right-click its panel and choose “Info > User manual”.\n\nYou can also open the module's Info menu to view the module's tags, website, VCV Library page, and changelog, if available.", "", ""},
	{"Did you know that the VCV Library is integrated with ModularGrid? If a module is available as a hardware Eurorack module, right-click its panel and choose “Info > ModularGrid”, or click the “ModularGrid” link on its VCV Library page.\n\nOn ModularGrid.net, search for the VCV logo on certain module's entry pages.", "Example: Grayscale Permutation on ModularGrid", "https://www.modulargrid.net/e/grayscale-permutation-18hp"},
	{"When any context menu is open, you can " RACK_MOD_CTRL_NAME "+click a menu item to keep the menu open. This can be useful when browsing module presets or settings.", "", ""},
	// {"", "", ""},
};


struct TipWindow : widget::OpaqueWidget {
	ui::SequentialLayout* layout;
	ui::SequentialLayout* buttonLayout;
	ui::Label* label;
	UrlButton* linkButton;

	TipWindow() {
		box.size = math::Vec(550, 200);
		const float margin = 10;
		const float buttonWidth = 100;

		layout = new ui::SequentialLayout;
		layout->box.pos = math::Vec(0, 10);
		layout->box.size = box.size;
		layout->orientation = ui::SequentialLayout::VERTICAL_ORIENTATION;
		layout->margin = math::Vec(margin, margin);
		layout->spacing = math::Vec(margin, margin);
		layout->wrap = false;
		addChild(layout);

		ui::Label* header = new ui::Label;
		// header->box.size.x = box.size.x - 2*margin;
		header->box.size.y = 20;
		header->fontSize = 20;
		header->text = "Welcome to " + APP_NAME + " " + APP_EDITION_NAME + " " + APP_VERSION;
		layout->addChild(header);

		label = new ui::Label;
		label->box.size.y = 80;
		label->box.size.x = box.size.x - 2*margin;
		layout->addChild(label);

		// Container for link button so hiding it won't shift layout
		widget::Widget* linkPlaceholder = new widget::Widget;
		layout->addChild(linkPlaceholder);

		linkButton = new UrlButton;
		linkButton->box.size.x = box.size.x - 2*margin;
		linkPlaceholder->box.size = linkButton->box.size;
		linkPlaceholder->addChild(linkButton);

		buttonLayout = new ui::SequentialLayout;
		buttonLayout->box.size.x = box.size.x - 2*margin;
		buttonLayout->spacing = math::Vec(margin, margin);
		layout->addChild(buttonLayout);

		struct ShowQuantity : Quantity {
			void setValue(float value) override {
				settings::showTipsOnLaunch = (value > 0.f);
			}
			float getValue() override {
				return settings::showTipsOnLaunch ? 1.f : 0.f;
			}
		};
		static ShowQuantity showQuantity;

		ui::OptionButton* showButton = new ui::OptionButton;
		showButton->box.size.x = 200;
		showButton->text = "Show tips at startup";
		showButton->quantity = &showQuantity;
		buttonLayout->addChild(showButton);

		struct PreviousButton : ui::Button {
			TipWindow* tipWindow;
			void onAction(const ActionEvent& e) override {
				tipWindow->advanceTip(-1);
			}
		};
		PreviousButton* prevButton = new PreviousButton;
		prevButton->box.size.x = buttonWidth;
		prevButton->text = "◀  Previous";
		prevButton->tipWindow = this;
		buttonLayout->addChild(prevButton);

		struct NextButton : ui::Button {
			TipWindow* tipWindow;
			void onAction(const ActionEvent& e) override {
				tipWindow->advanceTip();
			}
		};
		NextButton* nextButton = new NextButton;
		nextButton->box.size.x = buttonWidth;
		nextButton->text = "▶  Next";
		nextButton->tipWindow = this;
		buttonLayout->addChild(nextButton);

		struct CloseButton : ui::Button {
			TipWindow* tipWindow;
			void onAction(const ActionEvent& e) override {
				tipWindow->getParent()->requestDelete();
			}
		};
		CloseButton* closeButton = new CloseButton;
		closeButton->box.size.x = buttonWidth;
		closeButton->text = "✖  Close";
		closeButton->tipWindow = this;
		buttonLayout->addChild(closeButton);

		buttonLayout->box.size.y = closeButton->box.size.y;

		// When the TipWindow is created, choose the next tip
		advanceTip();
	}

	void advanceTip(int delta = 1) {
		// Increment tip index
		settings::tipIndex = math::eucMod(settings::tipIndex + delta, (int) tipInfos.size());

		const TipInfo& tipInfo = tipInfos[settings::tipIndex];
		label->text = tipInfo.text;
		linkButton->setVisible(tipInfo.linkText != "");
		linkButton->text = tipInfo.linkText;
		linkButton->url = tipInfo.linkUrl;
	}

	void step() override {
		OpaqueWidget::step();

		box.pos = parent->box.size.minus(box.size).div(2).round();
	}

	void draw(const DrawArgs& args) override {
		bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, 0);
		Widget::draw(args);
	}
};


widget::Widget* tipWindowCreate() {
	ui::MenuOverlay* overlay = new ui::MenuOverlay;
	overlay->bgColor = nvgRGBAf(0, 0, 0, 0.33);

	TipWindow* tipWindow = new TipWindow;
	overlay->addChild(tipWindow);

	return overlay;
}


} // namespace app
} // namespace rack
