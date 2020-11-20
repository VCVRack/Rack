#include <thread>

#include <app/TipWindow.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Label.hpp>
#include <ui/Button.hpp>
#include <ui/MenuItem.hpp>
#include <ui/SequentialLayout.hpp>
#include <settings.hpp>
#include <system.hpp>


namespace rack {
namespace app {


struct TipOverlay : widget::OpaqueWidget {
	void step() override {
		box = parent->box.zeroPos();
		OpaqueWidget::step();
	}

	void onButton(const event::Button& e) override {
		OpaqueWidget::onButton(e);
		if (e.getTarget() != this)
			return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			hide();
			e.consume(this);
		}
	}
};


struct UrlButton : ui::Button {
	std::string url;
	void onAction(const event::Action& e) override {
		std::thread t([=] {
			system::openBrowser(url);
		});
		t.detach();
	}
};


struct TipInfo {
	std::string text;
	std::string linkText;
	std::string linkUrl;
};


static std::vector<TipInfo> tipInfos = {
	{"To add a module to the rack, right-click an empty rack space or press Enter. Click and drag a module from the Module Browser into the desired rack space.\n\nYou can force-move modules by holding " RACK_MOD_CTRL_NAME " while dragging it.", "", ""}, // reviewed
	{"Pan around the rack by using the scroll bars, dragging while holding the middle mouse button, or pressing the arrow keys. Arrow key panning speed can be modified by holding " RACK_MOD_CTRL_NAME ", " RACK_MOD_SHIFT_NAME ", or " RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME ".\n\nZoom in and out using the View menu, " RACK_MOD_CTRL_NAME "+scroll, or " RACK_MOD_CTRL_NAME "+= / " RACK_MOD_CTRL_NAME "+-.", "", ""}, // reviewed
	// {"Want to use VCV Rack as a plugin in your DAW? VCV Rack for DAWs is available now as a 64-bit VST 2 plugin for Ableton Live, Cubase, FL Studio, Reason, Studio One, REAPER, Bitwig, and more. Other plugin formats coming soon.", "Learn more", "https://vcvrack.com/RackForDAWs"}, // reviewed
	{"You can use Rack fullscreen by selecting View > Fullscreen or pressing F11.\n\nIn fullscreen mode, the menu bar and scroll bars are hidden. This is ideal for screen recording with VCV Recorder.", "Get VCV Recorder", "https://vcvrack.com/Recorder"}, // reviewed
	{"You can browse over 2400 modules on the VCV Library.", "VCV Library", "https://library.vcvrack.com/"},
	{"Some plugin developers accept donations for their work. Right-click a module panel and select Info > Donate.\n\nYou can support VCV Rack by purchasing VCV plugins.", "VCV Library", "https://library.vcvrack.com/"}, // reviewed
	{"You can learn more about VCV Rack by browsing the official Rack manual.", "VCV Rack manual", "https://vcvrack.com/manual/"},
	{"Follow VCV Rack on Twitter for new modules, product announcements, and development news.", "Twitter @vcvrack", "https://twitter.com/vcvrack"}, // reviewed
	{"Did you know that patch cables in Rack can carry up to 16 signals? You can use this to easily build polyphonic patches with modules with the \"Polyphonic\" tag. Cables carrying more than 1 signal appear thicker than normal cables. To try out polyphony, add the VCV MIDI-CV module to your patch, right-click its panel, and select your desired number of polyphonic channels.", "Learn more about polyphony in VCV Rack", "https://vcvrack.com/manual/Polyphony"}, // reviewed
	{"Know C++ programming and want to create your own modules for Rack? Developing Rack modules is a great way to learn digital signal processing and quickly test your ideas with an easy-to-learn platform.\n\nDownload the Rack SDK and follow the official tutorial to get started.", "Plugin Development Tutorial", "https://vcvrack.com/manual/PluginDevelopmentTutorial"}, // reviewed
	{"Confused about how to use a particular module? Right-click its panel and choose Info > User manual.\n\nYou can also open the module's Info menu to view the module's tags, website, VCV Library entry, and changelog.", "", ""}, // reviewed
	{"Did you know that ModularGrid is interconnected with the VCV Library? If a Eurorack version of a Rack module is available, right-click its panel and choose Info > ModularGrid, or click the \"ModularGrid\" link on its VCV Library page.\nOn ModularGrid.net, you can click the \"Available for VCV Rack\" link if a hardware module has a virtual Rack version.", "Example: Grayscale Permutation on ModularGrid", "https://www.modulargrid.net/e/grayscale-permutation-18hp"}, // reviewed
	// {"", "", ""},
};


struct TipWindow : widget::OpaqueWidget {
	ui::Label* label;
	UrlButton* linkButton;

	TipWindow() {
		float margin = 10;
		float buttonWidth = 80;
		box.size.x = buttonWidth*5 + margin*6;

		ui::Label* header = new ui::Label;
		header->box.pos.x = margin;
		header->box.pos.y = 20;
		// header->box.size.x = box.size.x - margin*2;
		header->box.size.y = 20;
		header->fontSize = 20;
		header->text = "Welcome to VCV Rack v" + APP_VERSION;
		addChild(header);

		label = new ui::Label;
		label->box.pos.x = margin;
		label->box.pos.y = header->box.getBottom() + margin;
		label->box.size.y = 80;
		label->box.size.x = box.size.x - margin*2;
		addChild(label);

		linkButton = new UrlButton;
		linkButton->box.pos.x = margin;
		linkButton->box.pos.y = label->box.getBottom() + margin;
		linkButton->box.size.x = box.size.x - margin*2;
		addChild(linkButton);

		ui::SequentialLayout* buttonLayout = new ui::SequentialLayout;
		buttonLayout->box.pos.x = margin;
		buttonLayout->box.pos.y = linkButton->box.getBottom() + margin;
		buttonLayout->box.size.x = box.size.x - margin*2;
		buttonLayout->spacing = math::Vec(margin, margin);
		addChild(buttonLayout);

		struct ShowButton : ui::Button {
			void step() override {
				text = settings::showTipsOnLaunch ? "Don't show at startup" : "Show tips at startup";
			}
			void onAction(const event::Action& e) override {
				settings::showTipsOnLaunch ^= true;
			}
		};
		ShowButton* showButton = new ShowButton;
		showButton->box.size.x = buttonWidth * 2 + margin;
		buttonLayout->addChild(showButton);

		struct PreviousButton : ui::Button {
			TipWindow* tipWindow;
			void onAction(const event::Action& e) override {
				tipWindow->advanceTip(-1);
			}
		};
		PreviousButton* prevButton = new PreviousButton;
		prevButton->box.size.x = buttonWidth;
		prevButton->text = "Previous";
		prevButton->tipWindow = this;
		buttonLayout->addChild(prevButton);

		struct NextButton : ui::Button {
			TipWindow* tipWindow;
			void onAction(const event::Action& e) override {
				tipWindow->advanceTip();
			}
		};
		NextButton* nextButton = new NextButton;
		nextButton->box.size.x = buttonWidth;
		nextButton->text = "Next";
		nextButton->tipWindow = this;
		buttonLayout->addChild(nextButton);

		struct CloseButton : ui::Button {
			TipWindow* tipWindow;
			void onAction(const event::Action& e) override {
				tipWindow->getParent()->hide();
			}
		};
		CloseButton* closeButton = new CloseButton;
		closeButton->box.size.x = buttonWidth;
		closeButton->text = "Close";
		closeButton->tipWindow = this;
		buttonLayout->addChild(closeButton);

		buttonLayout->box.size.y = closeButton->box.size.y;
		box.size.y = buttonLayout->box.getBottom() + margin;

		// When the TipWindow is created, choose the next tip
		advanceTip();
	}

	void advanceTip(int delta = 1) {
		// Increment tip index
		settings::tipIndex = math::eucMod(settings::tipIndex + delta, (int) tipInfos.size());

		TipInfo& tipInfo = tipInfos[settings::tipIndex];
		label->text = tipInfo.text;
		linkButton->setVisible(tipInfo.linkText != "");
		linkButton->text = tipInfo.linkText;
		linkButton->url = tipInfo.linkUrl;
	}

	void step() override {
		box.pos = parent->box.size.minus(box.size).div(2);
		OpaqueWidget::step();
	}

	void draw(const DrawArgs& args) override {
		bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, 0);
		Widget::draw(args);
	}

	void onShow(const event::Show& e) override {
		advanceTip();
		OpaqueWidget::onShow(e);
	}
};


widget::Widget* tipWindowCreate() {
	TipOverlay* overlay = new TipOverlay;

	TipWindow* tipWindow = new TipWindow;
	overlay->addChild(tipWindow);

	return overlay;
}


} // namespace app
} // namespace rack
