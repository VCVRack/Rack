#include <thread>
#include <utility>
#include <algorithm>

#include <osdialog.h>

#include <app/MenuBar.hpp>
#include <app/TipWindow.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Button.hpp>
#include <ui/MenuItem.hpp>
#include <ui/MenuSeparator.hpp>
#include <ui/SequentialLayout.hpp>
#include <ui/Slider.hpp>
#include <ui/TextField.hpp>
#include <ui/ProgressBar.hpp>
#include <ui/Label.hpp>
#include <engine/Engine.hpp>
#include <window/Window.hpp>
#include <asset.hpp>
#include <context.hpp>
#include <settings.hpp>
#include <helpers.hpp>
#include <system.hpp>
#include <plugin.hpp>
#include <patch.hpp>
#include <library.hpp>


namespace rack {
namespace app {
namespace menuBar {


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


////////////////////
// File
////////////////////


struct FileButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		menu->addChild(createMenuItem(string::translate("MenuBar.file.new"), widget::getKeyCommandName(GLFW_KEY_N, RACK_MOD_CTRL), []() {
			APP->patch->loadTemplateDialog();
		}));

		menu->addChild(createMenuItem(string::translate("MenuBar.file.open"), widget::getKeyCommandName(GLFW_KEY_O, RACK_MOD_CTRL), []() {
			APP->patch->loadDialog();
		}));

		menu->addChild(createSubmenuItem(string::translate("MenuBar.file.openRecent"), "", [](ui::Menu* menu) {
			for (const std::string& path : settings::recentPatchPaths) {
				std::string name = system::getStem(path);
				menu->addChild(createMenuItem(name, "", [=]() {
					APP->patch->loadPathDialog(path);
				}));
			}
		}, settings::recentPatchPaths.empty()));

		menu->addChild(createMenuItem(string::translate("MenuBar.file.save"), widget::getKeyCommandName(GLFW_KEY_S, RACK_MOD_CTRL), []() {
			APP->patch->saveDialog();
		}));

		menu->addChild(createMenuItem(string::translate("MenuBar.file.saveAs"), widget::getKeyCommandName(GLFW_KEY_S, RACK_MOD_CTRL | GLFW_MOD_SHIFT), []() {
			APP->patch->saveAsDialog();
		}));

		menu->addChild(createMenuItem(string::translate("MenuBar.file.saveCopy"), "", []() {
			APP->patch->saveAsDialog(false);
		}));

		menu->addChild(createMenuItem(string::translate("MenuBar.file.revert"), widget::getKeyCommandName(GLFW_KEY_O, RACK_MOD_CTRL | GLFW_MOD_SHIFT), []() {
			APP->patch->revertDialog();
		}, APP->patch->path == ""));

		menu->addChild(createMenuItem(string::translate("MenuBar.file.overwriteTemplate"), "", []() {
			APP->patch->saveTemplateDialog();
		}));

		menu->addChild(new ui::MenuSeparator);

		// Load selection
		menu->addChild(createMenuItem(string::translate("MenuBar.file.importSelection"), "", [=]() {
			APP->scene->rack->loadSelectionDialog();
		}, false, true));

		menu->addChild(new ui::MenuSeparator);

		menu->addChild(createMenuItem(string::translate("MenuBar.file.quit"), widget::getKeyCommandName(GLFW_KEY_Q, RACK_MOD_CTRL), []() {
			APP->window->close();
		}));
	}
};


////////////////////
// Edit
////////////////////


struct EditButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		struct UndoItem : ui::MenuItem {
			void step() override {
				bool canUndo = APP->history->canUndo();
				text = canUndo ? string::f(string::translate("MenuBar.edit.undoAction"), APP->history->getUndoName()) : string::translate("MenuBar.edit.undo");
				disabled = !canUndo;
				MenuItem::step();
			}
			void onAction(const ActionEvent& e) override {
				APP->history->undo();
			}
		};
		menu->addChild(createMenuItem<UndoItem>("", widget::getKeyCommandName(GLFW_KEY_Z, RACK_MOD_CTRL)));

		struct RedoItem : ui::MenuItem {
			void step() override {
				bool canRedo = APP->history->canRedo();
				text = canRedo ? string::f(string::translate("MenuBar.edit.redoAction"), APP->history->getRedoName()) : string::translate("MenuBar.edit.redo");
				disabled = !canRedo;
				MenuItem::step();
			}
			void onAction(const ActionEvent& e) override {
				APP->history->redo();
			}
		};
		menu->addChild(createMenuItem<RedoItem>("", widget::getKeyCommandName(GLFW_KEY_Z, RACK_MOD_CTRL | GLFW_MOD_SHIFT)));

		menu->addChild(createMenuItem(string::translate("MenuBar.edit.clearCables"), "", [=]() {
			APP->patch->disconnectDialog();
		}));

		menu->addChild(new ui::MenuSeparator);

		APP->scene->rack->appendSelectionContextMenu(menu);
	}
};


////////////////////
// View
////////////////////


struct ZoomQuantity : Quantity {
	void setValue(float value) override {
		APP->scene->rackScroll->setZoom(std::pow(2.f, value));
	}
	float getValue() override {
		return std::log2(APP->scene->rackScroll->getZoom());
	}
	float getMinValue() override {
		return -2.f;
	}
	float getMaxValue() override {
		return 2.f;
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
		return string::translate("MenuBar.view.zoom");
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
		return string::translate("MenuBar.view.cableOpacity");
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
	float getDisplayValue() override {
		return getValue() * 100;
	}
	void setDisplayValue(float displayValue) override {
		setValue(displayValue / 100);
	}
	std::string getLabel() override {
		return string::translate("MenuBar.view.cableTension");
	}
	std::string getUnit() override {
		return "%";
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


struct RackBrightnessQuantity : Quantity {
	void setValue(float value) override {
		settings::rackBrightness = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::rackBrightness;
	}
	float getDefaultValue() override {
		return 1.0;
	}
	float getDisplayValue() override {
		return getValue() * 100;
	}
	void setDisplayValue(float displayValue) override {
		setValue(displayValue / 100);
	}
	std::string getUnit() override {
		return "%";
	}
	std::string getLabel() override {
		return string::translate("MenuBar.view.roomBrightness");
	}
	int getDisplayPrecision() override {
		return 3;
	}
};
struct RackBrightnessSlider : ui::Slider {
	RackBrightnessSlider() {
		quantity = new RackBrightnessQuantity;
	}
	~RackBrightnessSlider() {
		delete quantity;
	}
};


struct HaloBrightnessQuantity : Quantity {
	void setValue(float value) override {
		settings::haloBrightness = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::haloBrightness;
	}
	float getDefaultValue() override {
		return 0.25;
	}
	float getDisplayValue() override {
		return getValue() * 100;
	}
	void setDisplayValue(float displayValue) override {
		setValue(displayValue / 100);
	}
	std::string getUnit() override {
		return "%";
	}
	std::string getLabel() override {
		return string::translate("MenuBar.view.lightBloom");
	}
	int getDisplayPrecision() override {
		return 3;
	}
};
struct HaloBrightnessSlider : ui::Slider {
	HaloBrightnessSlider() {
		quantity = new HaloBrightnessQuantity;
	}
	~HaloBrightnessSlider() {
		delete quantity;
	}
};


struct KnobScrollSensitivityQuantity : Quantity {
	void setValue(float value) override {
		value = math::clamp(value, getMinValue(), getMaxValue());
		settings::knobScrollSensitivity = std::pow(2.f, value);
	}
	float getValue() override {
		return std::log2(settings::knobScrollSensitivity);
	}
	float getMinValue() override {
		return std::log2(1e-4f);
	}
	float getMaxValue() override {
		return std::log2(1e-2f);
	}
	float getDefaultValue() override {
		return std::log2(1e-3f);
	}
	float getDisplayValue() override {
		return std::pow(2.f, getValue() - getDefaultValue());
	}
	void setDisplayValue(float displayValue) override {
		setValue(std::log2(displayValue) + getDefaultValue());
	}
	std::string getLabel() override {
		return string::translate("MenuBar.view.wheelSensitivity");
	}
	int getDisplayPrecision() override {
		return 2;
	}
};
struct KnobScrollSensitivitySlider : ui::Slider {
	KnobScrollSensitivitySlider() {
		quantity = new KnobScrollSensitivityQuantity;
	}
	~KnobScrollSensitivitySlider() {
		delete quantity;
	}
};


struct ViewButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		menu->addChild(createMenuLabel(string::translate("MenuBar.view.window")));

		bool fullscreen = APP->window->isFullScreen();
		std::string fullscreenText = widget::getKeyCommandName(GLFW_KEY_F11, 0);
		if (fullscreen)
			fullscreenText += " " CHECKMARK_STRING;
		menu->addChild(createMenuItem(string::translate("MenuBar.view.fullscreen"), fullscreenText, [=]() {
			APP->window->setFullScreen(!fullscreen);
		}));

		menu->addChild(createSubmenuItem(string::translate("MenuBar.view.frameRate"), string::f("%.0f Hz", settings::frameRateLimit), [=](ui::Menu* menu) {
			for (int i = 1; i <= 6; i++) {
				double frameRate = APP->window->getMonitorRefreshRate() / i;
				menu->addChild(createCheckMenuItem(string::f("%.0f Hz", frameRate), "",
					[=]() {return settings::frameRateLimit == frameRate;},
					[=]() {settings::frameRateLimit = frameRate;}
				));
			}
		}));

		static const std::vector<float> pixelRatios = {0, 1, 1.5, 2, 2.5, 3};
		std::vector<std::string> pixelRatioLabels;
		for (float pixelRatio : pixelRatios) {
			pixelRatioLabels.push_back(pixelRatio == 0.f ? string::translate("MenuBar.view.pixelRatio.auto") : string::f("%0.f%%", pixelRatio * 100.f));
		}
		menu->addChild(createIndexSubmenuItem(string::translate("MenuBar.view.pixelRatio"), pixelRatioLabels, [=]() -> size_t {
			auto it = std::find(pixelRatios.begin(), pixelRatios.end(), settings::pixelRatio);
			if (it == pixelRatios.end())
				return -1;
			return it - pixelRatios.begin();
		}, [=](size_t i) {
			settings::pixelRatio = pixelRatios[i];
		}));

		ZoomSlider* zoomSlider = new ZoomSlider;
		zoomSlider->box.size.x = 250.0;
		menu->addChild(zoomSlider);

		menu->addChild(createMenuItem(string::translate("MenuBar.view.zoomFit"), widget::getKeyCommandName(GLFW_KEY_F4, 0), [=]() {
			APP->scene->rackScroll->zoomToModules();
		}));

		menu->addChild(createIndexPtrSubmenuItem(string::translate("MenuBar.view.mouseWheelZoom"), {
			string::f(string::translate("MenuBar.view.mouseWheelZoom.scroll"), RACK_MOD_CTRL_NAME),
			string::f(string::translate("MenuBar.view.mouseWheelZoom.zoom"), RACK_MOD_CTRL_NAME)
		}, &settings::mouseWheelZoom));

		menu->addChild(new ui::MenuSeparator);
		menu->addChild(createMenuLabel(string::translate("MenuBar.view.appearance")));

		static const std::vector<std::string> uiThemes = {"dark", "light", "hcdark"};
		menu->addChild(createIndexSubmenuItem(string::translate("MenuBar.view.uiTheme"), {
			string::translate("MenuBar.view.appearance.dark"),
			string::translate("MenuBar.view.appearance.light"),
			string::translate("MenuBar.view.appearance.hcdark")
		}, [=]() -> size_t {
			auto it = std::find(uiThemes.begin(), uiThemes.end(), settings::uiTheme);
			if (it == uiThemes.end())
				return -1;
			return it - uiThemes.begin();
		}, [=](size_t i) {
			settings::uiTheme = uiThemes[i];
			ui::refreshTheme();
		}));

		menu->addChild(createBoolPtrMenuItem(string::translate("MenuBar.view.showTooltips"), "", &settings::tooltips));

		// Various sliders
		CableOpacitySlider* cableOpacitySlider = new CableOpacitySlider;
		cableOpacitySlider->box.size.x = 250.0;
		menu->addChild(cableOpacitySlider);

		CableTensionSlider* cableTensionSlider = new CableTensionSlider;
		cableTensionSlider->box.size.x = 250.0;
		menu->addChild(cableTensionSlider);

		RackBrightnessSlider* rackBrightnessSlider = new RackBrightnessSlider;
		rackBrightnessSlider->box.size.x = 250.0;
		menu->addChild(rackBrightnessSlider);

		HaloBrightnessSlider* haloBrightnessSlider = new HaloBrightnessSlider;
		haloBrightnessSlider->box.size.x = 250.0;
		menu->addChild(haloBrightnessSlider);

		// Cable colors
		menu->addChild(createSubmenuItem(string::translate("MenuBar.view.cableColors"), "", [=](ui::Menu* menu) {
			// TODO Subclass Menu to make an auto-refreshing list so user can Ctrl+click to keep menu open.

			// Add color items
			for (size_t i = 0; i < settings::cableColors.size(); i++) {
				NVGcolor color = settings::cableColors[i];
				std::string label = get(settings::cableLabels, i);
				std::string labelFallback = (label != "") ? label : string::f("Color #%lld", (long long) (i + 1));

				ui::ColorDotMenuItem* item = createSubmenuItem<ui::ColorDotMenuItem>(labelFallback, "", [=](ui::Menu* menu) {
					// Helper for launching color dialog
					auto selectColor = [](NVGcolor& color) -> bool {
						osdialog_color c = {
							uint8_t(color.r * 255.f),
							uint8_t(color.g * 255.f),
							uint8_t(color.b * 255.f),
							uint8_t(color.a * 255.f),
						};
						if (!osdialog_color_picker(&c, false))
							return false;
						color = nvgRGBA(c.r, c.g, c.b, c.a);
						return true;
					};

					menu->addChild(createMenuItem(string::translate("MenuBar.view.cableColors.setLabel"), "", [=]() {
						if (i >= settings::cableColors.size())
							return;
						char* s = osdialog_prompt(OSDIALOG_INFO, "", label.c_str());
						if (!s)
							return;
						settings::cableLabels.resize(settings::cableColors.size());
						settings::cableLabels[i] = s;
						free(s);
					}, false, true));

					menu->addChild(createMenuItem(string::translate("MenuBar.view.cableColors.setColor"), "", [=]() {
						if (i >= settings::cableColors.size())
							return;
						NVGcolor newColor = color;
						if (!selectColor(newColor))
							return;
						std::memcpy(&settings::cableColors[i], &newColor, sizeof(newColor));
					}, false, true));

					menu->addChild(createMenuItem(string::translate("MenuBar.view.cableColors.newColorAbove"), "", [=]() {
						if (i >= settings::cableColors.size())
							return;
						NVGcolor newColor = color;
						if (!selectColor(newColor))
							return;
						settings::cableLabels.resize(settings::cableColors.size());
						settings::cableColors.insert(settings::cableColors.begin() + i, newColor);
						settings::cableLabels.insert(settings::cableLabels.begin() + i, "");
					}, false, true));

					menu->addChild(createMenuItem(string::translate("MenuBar.view.cableColors.newColorBelow"), "", [=]() {
						if (i >= settings::cableColors.size())
							return;
						NVGcolor newColor = color;
						if (!selectColor(newColor))
							return;
						settings::cableLabels.resize(settings::cableColors.size());
						settings::cableColors.insert(settings::cableColors.begin() + i + 1, newColor);
						settings::cableLabels.insert(settings::cableLabels.begin() + i + 1, "");
					}, false, true));

					menu->addChild(createMenuItem(string::translate("MenuBar.view.cableColors.moveUp"), "", [=]() {
						if (i < 1 || i >= settings::cableColors.size())
							return;
						settings::cableLabels.resize(settings::cableColors.size());
						std::swap(settings::cableColors[i], settings::cableColors[i - 1]);
						std::swap(settings::cableLabels[i], settings::cableLabels[i - 1]);
					}, i < 1, true));

					menu->addChild(createMenuItem(string::translate("MenuBar.view.cableColors.moveDown"), "", [=]() {
						if (i + 1 >= settings::cableColors.size())
							return;
						settings::cableLabels.resize(settings::cableColors.size());
						std::swap(settings::cableColors[i], settings::cableColors[i + 1]);
						std::swap(settings::cableLabels[i], settings::cableLabels[i + 1]);
					}, i + 1 >= settings::cableColors.size()));

					menu->addChild(createMenuItem(string::translate("MenuBar.view.cableColors.delete"), "", [=]() {
						if (i >= settings::cableColors.size())
							return;
						settings::cableLabels.resize(settings::cableColors.size());
						settings::cableColors.erase(settings::cableColors.begin() + i);
						settings::cableLabels.erase(settings::cableLabels.begin() + i);
					}, settings::cableColors.size() <= 1, true));
				});
				item->color = color;
				menu->addChild(item);
			}

			menu->addChild(createBoolMenuItem(string::translate("MenuBar.view.cableColors.autoRotate"), "",
				[=]() -> bool {
					return settings::cableAutoRotate;
				},
				[=](bool s) {
					settings::cableAutoRotate = s;
				}
			));
			menu->addChild(createMenuItem(string::translate("MenuBar.view.cableColors.restoreFactory"), "", [=]() {
				if (!osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, string::translate("MenuBar.view.cableColors.overwriteFactory").c_str()))
					return;
				settings::resetCables();
			}, false, true));
		}));

		menu->addChild(new ui::MenuSeparator);
		menu->addChild(createMenuLabel(string::translate("MenuBar.view.parameters")));

		menu->addChild(createBoolPtrMenuItem(string::translate("MenuBar.view.lockCursor"), "", &settings::allowCursorLock));

		static const std::vector<std::string> knobModeLabels = {
			string::translate("MenuBar.view.knob.linear"),
			string::translate("MenuBar.view.knob.scaledLinear"),
			string::translate("MenuBar.view.knob.absRotary"),
			string::translate("MenuBar.view.knob.relRotary"),
		};
		static const std::vector<int> knobModes = {0, 2, 3};
		menu->addChild(createSubmenuItem(string::translate("MenuBar.view.knob"), knobModeLabels[settings::knobMode], [=](ui::Menu* menu) {
			for (int knobMode : knobModes) {
				menu->addChild(createCheckMenuItem(knobModeLabels[knobMode], "",
					[=]() {return settings::knobMode == knobMode;},
					[=]() {settings::knobMode = (settings::KnobMode) knobMode;}
				));
			}
		}));

		menu->addChild(createBoolPtrMenuItem(string::translate("MenuBar.view.knobScroll"), "", &settings::knobScroll));

		KnobScrollSensitivitySlider* knobScrollSensitivitySlider = new KnobScrollSensitivitySlider;
		knobScrollSensitivitySlider->box.size.x = 250.0;
		menu->addChild(knobScrollSensitivitySlider);

		menu->addChild(new ui::MenuSeparator);
		menu->addChild(createMenuLabel(string::translate("MenuBar.view.modules")));

		menu->addChild(createBoolPtrMenuItem(string::translate("MenuBar.view.lockModules"), "", &settings::lockModules));

		menu->addChild(createBoolPtrMenuItem(string::translate("MenuBar.view.squeezeModules"), "", &settings::squeezeModules));

		menu->addChild(createBoolPtrMenuItem(string::translate("MenuBar.view.preferDarkPanels"), "", &settings::preferDarkPanels));
	}
};


////////////////////
// Engine
////////////////////


struct SampleRateItem : ui::MenuItem {
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		// Auto sample rate
		std::string rightText;
		if (settings::sampleRate == 0) {
			float sampleRate = APP->engine->getSampleRate();
			rightText += string::f("(%g kHz) ", sampleRate / 1000.f);
		}
		menu->addChild(createCheckMenuItem(string::translate("MenuBar.engine.sampleRate.auto"), rightText,
			[=]() {return settings::sampleRate == 0;},
			[=]() {settings::sampleRate = 0;}
		));

		// Power-of-2 oversample times 44.1kHz or 48kHz
		for (int i = -2; i <= 4; i++) {
			for (int j = 0; j < 2; j++) {
				float oversample = std::pow(2.f, i);
				float sampleRate = (j == 0) ? 44100.f : 48000.f;
				sampleRate *= oversample;

				std::string text = string::f("%g kHz", sampleRate / 1000.f);
				std::string rightText;
				if (oversample > 1.f) {
					rightText += string::f("(%.0fx)", oversample);
				}
				else if (oversample < 1.f) {
					rightText += string::f("(1/%.0fx)", 1.f / oversample);
				}
				menu->addChild(createCheckMenuItem(text, rightText,
					[=]() {return settings::sampleRate == sampleRate;},
					[=]() {settings::sampleRate = sampleRate;}
				));
			}
		}
		return menu;
	}
};


struct EngineButton : MenuButton {
	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		std::string cpuMeterText = widget::getKeyCommandName(GLFW_KEY_F3, 0);
		if (settings::cpuMeter)
			cpuMeterText += " " CHECKMARK_STRING;
		menu->addChild(createMenuItem(string::translate("MenuBar.engine.cpuMeter"), cpuMeterText, [=]() {
			settings::cpuMeter ^= true;
		}));

		menu->addChild(createMenuItem<SampleRateItem>(string::translate("MenuBar.engine.sampleRate"), RIGHT_ARROW));

		menu->addChild(createSubmenuItem(string::translate("MenuBar.engine.threads"), string::f("%d", settings::threadCount), [=](ui::Menu* menu) {
			// BUG This assumes SMT is enabled.
			int cores = system::getLogicalCoreCount() / 2;

			for (int i = 1; i <= 2 * cores; i++) {
				std::string rightText;
				if (i == cores)
					rightText += string::translate("MenuBar.engine.threads.most");
				else if (i == 1)
					rightText += string::translate("MenuBar.engine.threads.lowest");
				menu->addChild(createCheckMenuItem(string::f("%d", i), rightText,
					[=]() {return settings::threadCount == i;},
					[=]() {settings::threadCount = i;}
				));
			}
		}));
	}
};


////////////////////
// Plugins
////////////////////


struct AccountPasswordField : ui::PasswordField {
	ui::MenuItem* logInItem;
	void onAction(const ActionEvent& e) override {
		logInItem->doAction();
	}
};


struct LogInItem : ui::MenuItem {
	ui::TextField* emailField;
	ui::TextField* passwordField;

	void onAction(const ActionEvent& e) override {
		std::string email = emailField->text;
		std::string password = passwordField->text;
		std::thread t([=] {
			library::logIn(email, password);
			library::checkUpdates();
		});
		t.detach();
		e.unconsume();
	}

	void step() override {
		text = string::translate("MenuBar.library.login");
		rightText = library::loginStatus;
		MenuItem::step();
	}
};


struct SyncUpdatesItem : ui::MenuItem {
	void step() override {
		if (library::updateStatus != "") {
			text = library::updateStatus;
		}
		else if (library::isSyncing) {
			text = string::translate("MenuBar.library.updating");
		}
		else if (!library::hasUpdates()) {
			text = string::translate("MenuBar.library.upToDate");
		}
		else {
			text = string::translate("MenuBar.library.updateAll");
		}

		disabled = library::isSyncing || !library::hasUpdates();
		MenuItem::step();
	}

	void onAction(const ActionEvent& e) override {
		std::thread t([=] {
			library::syncUpdates();
		});
		t.detach();
		e.unconsume();
	}
};


struct SyncUpdateItem : ui::MenuItem {
	std::string slug;

	void setUpdate(const std::string& slug) {
		this->slug = slug;

		auto it = library::updateInfos.find(slug);
		if (it == library::updateInfos.end())
			return;
		library::UpdateInfo update = it->second;

		text = update.name;
	}

	ui::Menu* createChildMenu() override {
		auto it = library::updateInfos.find(slug);
		if (it == library::updateInfos.end())
			return NULL;
		library::UpdateInfo update = it->second;

		ui::Menu* menu = new ui::Menu;

		if (update.minRackVersion != "") {
			menu->addChild(createMenuLabel(string::f(string::translate("MenuBar.library.requiresRack"), update.minRackVersion)));
		}

		if (update.changelogUrl != "") {
			std::string changelogUrl = update.changelogUrl;
			menu->addChild(createMenuItem(string::translate("MenuBar.library.changelog"), "", [=]() {
				system::openBrowser(changelogUrl);
			}));
		}

		if (menu->children.empty()) {
			delete menu;
			return NULL;
		}
		return menu;
	}

	void step() override {
		disabled = false;

		if (library::isSyncing)
			disabled = true;

		auto it = library::updateInfos.find(slug);
		if (it == library::updateInfos.end()) {
			disabled = true;
		}
		else {
			library::UpdateInfo update = it->second;

			if (update.minRackVersion != "")
				disabled = true;

			if (update.downloaded) {
				rightText = CHECKMARK_STRING;
				disabled = true;
			}
			else if (slug == library::updateSlug) {
				rightText = string::f("%.0f%%", library::updateProgress * 100.f);
			}
			else {
				rightText = "";
				plugin::Plugin* p = plugin::getPlugin(slug);
				if (p) {
					rightText += p->version + " → ";
				}
				rightText += update.version;
			}
		}

		MenuItem::step();
	}

	void onAction(const ActionEvent& e) override {
		std::thread t([=] {
			library::syncUpdate(slug);
		});
		t.detach();
		e.unconsume();
	}
};


struct LibraryMenu : ui::Menu {
	LibraryMenu() {
		refresh();
	}

	void step() override {
		// Refresh menu when appropriate
		if (library::refreshRequested) {
			library::refreshRequested = false;
			refresh();
		}
		Menu::step();
	}

	void refresh() {
		setChildMenu(NULL);
		clearChildren();

		if (settings::devMode) {
			addChild(createMenuLabel(string::translate("MenuBar.library.devMode")));
		}
		else if (!library::isLoggedIn()) {
			addChild(createMenuItem(string::translate("MenuBar.library.register"), "", [=]() {
				system::openBrowser("https://vcvrack.com/login");
			}));

			ui::TextField* emailField = new ui::TextField;
			emailField->placeholder = string::translate("MenuBar.library.email");
			emailField->box.size.x = 240.0;
			addChild(emailField);

			AccountPasswordField* passwordField = new AccountPasswordField;
			passwordField->placeholder = string::translate("MenuBar.library.password");
			passwordField->box.size.x = 240.0;
			passwordField->nextField = emailField;
			emailField->nextField = passwordField;
			addChild(passwordField);

			LogInItem* logInItem = new LogInItem;
			logInItem->emailField = emailField;
			logInItem->passwordField = passwordField;
			passwordField->logInItem = logInItem;
			addChild(logInItem);
		}
		else {
			addChild(createMenuItem(string::translate("MenuBar.library.logOut"), "", [=]() {
				library::logOut();
			}));

			addChild(createMenuItem(string::translate("MenuBar.library.account"), "", [=]() {
				system::openBrowser("https://vcvrack.com/account");
			}));

			addChild(createMenuItem(string::translate("MenuBar.library.browse"), "", [=]() {
				system::openBrowser("https://library.vcvrack.com/");
			}));

			SyncUpdatesItem* syncItem = new SyncUpdatesItem;
			syncItem->text = string::translate("MenuBar.library.updateAll");
			addChild(syncItem);

			if (!library::updateInfos.empty()) {
				addChild(new ui::MenuSeparator);
				addChild(createMenuLabel(string::translate("MenuBar.library.updates")));

				for (auto& pair : library::updateInfos) {
					SyncUpdateItem* updateItem = new SyncUpdateItem;
					updateItem->setUpdate(pair.first);
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

	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu<LibraryMenu>();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		// Check for updates when menu is opened
		if (!settings::devMode) {
			std::thread t([&]() {
				system::setThreadName(string::translate("MenuBar.library"));
				library::checkUpdates();
			});
			t.detach();
		}
	}

	void step() override {
		notification->box.pos = math::Vec(0, 0);
		notification->visible = library::hasUpdates();

		// Popup when updates finish downloading
		if (library::restartRequested) {
			library::restartRequested = false;
			if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, string::translate("MenuBar.library.restart").c_str())) {
				APP->window->close();
				settings::restart = true;
			}
		}

		MenuButton::step();
	}
};


////////////////////
// Help
////////////////////


struct HelpButton : MenuButton {
	NotificationIcon* notification;

	HelpButton() {
		notification = new NotificationIcon;
		addChild(notification);
	}

	void onAction(const ActionEvent& e) override {
		ui::Menu* menu = createMenu();
		menu->cornerFlags = BND_CORNER_TOP;
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));

		menu->addChild(createSubmenuItem("🌐 " + string::translate("MenuBar.help.language"), "", [=](ui::Menu* menu) {
			appendLanguageMenu(menu);
		}));

		menu->addChild(createMenuItem(string::translate("MenuBar.help.tips"), "", [=]() {
			APP->scene->addChild(tipWindowCreate());
		}));

		menu->addChild(createMenuItem(string::translate("MenuBar.help.manual"), widget::getKeyCommandName(GLFW_KEY_F1, 0), [=]() {
			system::openBrowser("https://vcvrack.com/manual");
		}));

		menu->addChild(createMenuItem(string::translate("MenuBar.help.support"), "", [=]() {
			system::openBrowser("https://vcvrack.com/support");
		}));

		menu->addChild(createMenuItem("VCVRack.com", "", [=]() {
			system::openBrowser("https://vcvrack.com/");
		}));

		menu->addChild(new ui::MenuSeparator);

		menu->addChild(createMenuItem(string::translate("MenuBar.help.userFolder"), "", [=]() {
			system::openDirectory(asset::user(""));
		}));

		menu->addChild(createMenuItem(string::translate("MenuBar.help.changelog"), "", [=]() {
			system::openBrowser("https://github.com/VCVRack/Rack/blob/v2/CHANGELOG.md");
		}));

		if (library::isAppUpdateAvailable()) {
			menu->addChild(createMenuItem(string::f(string::translate("MenuBar.help.update"), APP_NAME), APP_VERSION + " → " + library::appVersion, [=]() {
				system::openBrowser(library::appDownloadUrl);
			}));
		}
		else if (!settings::autoCheckUpdates && !settings::devMode) {
			menu->addChild(createMenuItem(string::f(string::translate("MenuBar.help.checkUpdate"), APP_NAME), "", [=]() {
				std::thread t([&]() {
					library::checkAppUpdate();
				});
				t.detach();
			}, false, true));
		}
	}

	void step() override {
		notification->box.pos = math::Vec(0, 0);
		notification->visible = library::isAppUpdateAvailable();
		MenuButton::step();
	}
};


////////////////////
// MenuBar
////////////////////


struct InfoLabel : ui::Label {
	int frameCount = 0;
	double frameDurationTotal = 0.0;
	double frameDurationAvg = NAN;
	// double uiLastTime = 0.0;
	// double uiLastThreadTime = 0.0;
	// double uiFrac = 0.0;

	void step() override {
		// Compute frame rate
		double frameDuration = APP->window->getLastFrameDuration();
		if (std::isfinite(frameDuration)) {
			frameDurationTotal += frameDuration;
			frameCount++;
		}
		if (frameDurationTotal >= 1.0) {
			frameDurationAvg = frameDurationTotal / frameCount;
			frameDurationTotal = 0.0;
			frameCount = 0;
		}

		// Compute UI thread CPU
		// double time = system::getTime();
		// double uiDuration = time - uiLastTime;
		// if (uiDuration >= 1.0) {
		// 	double threadTime = system::getThreadTime();
		// 	uiFrac = (threadTime - uiLastThreadTime) / uiDuration;
		// 	uiLastThreadTime = threadTime;
		// 	uiLastTime = time;
		// }

		text = "";

		if (box.size.x >= 460) {
			double fps = std::isfinite(frameDurationAvg) ? 1.0 / frameDurationAvg : 0.0;
			double meterAverage = APP->engine->getMeterAverage();
			double meterMax = APP->engine->getMeterMax();
			text += string::f(string::translate("MenuBar.infoLabel"), fps, meterAverage * 100, meterMax * 100);
			text += "     ";
		}

		text += APP_NAME + " " + APP_EDITION_NAME + " " + APP_VERSION + " " + APP_OS_NAME + " " + APP_CPU_NAME;

		Label::step();
	}
};


struct MenuBar : widget::OpaqueWidget {
	InfoLabel* infoLabel;

	MenuBar() {
		const float margin = 5;
		box.size.y = BND_WIDGET_HEIGHT + 2 * margin;

		ui::SequentialLayout* layout = new ui::SequentialLayout;
		layout->margin = math::Vec(margin, margin);
		layout->spacing = math::Vec(0, 0);
		addChild(layout);

		FileButton* fileButton = new FileButton;
		fileButton->text = string::translate("MenuBar.file");
		layout->addChild(fileButton);

		EditButton* editButton = new EditButton;
		editButton->text = string::translate("MenuBar.edit");
		layout->addChild(editButton);

		ViewButton* viewButton = new ViewButton;
		viewButton->text = string::translate("MenuBar.view");
		layout->addChild(viewButton);

		EngineButton* engineButton = new EngineButton;
		engineButton->text = string::translate("MenuBar.engine");
		layout->addChild(engineButton);

		LibraryButton* libraryButton = new LibraryButton;
		libraryButton->text = string::translate("MenuBar.library");
		layout->addChild(libraryButton);

		HelpButton* helpButton = new HelpButton;
		helpButton->text = string::translate("MenuBar.help");
		layout->addChild(helpButton);

		infoLabel = new InfoLabel;
		infoLabel->box.size.x = 600;
		infoLabel->alignment = ui::Label::RIGHT_ALIGNMENT;
		layout->addChild(infoLabel);
	}

	void draw(const DrawArgs& args) override {
		bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL);
		bndBevel(args.vg, 0.0, 0.0, box.size.x, box.size.y);

		Widget::draw(args);
	}

	void step() override {
		Widget::step();
		infoLabel->box.size.x = box.size.x - infoLabel->box.pos.x - 5;
		// Setting 50% alpha prevents Label from using the default UI theme color, so set the color manually here.
		infoLabel->color = color::alpha(bndGetTheme()->regularTheme.textColor, 0.5);
	}
};


} // namespace menuBar


widget::Widget* createMenuBar() {
	menuBar::MenuBar* menuBar = new menuBar::MenuBar;
	return menuBar;
}


void appendLanguageMenu(ui::Menu* menu) {
	for (const std::string& language : string::getLanguages()) {
		menu->addChild(createCheckMenuItem(string::translate("language", language), "", [=]() {
			return settings::language == language;
		}, [=]() {
			if (settings::language == language)
				return;
			settings::language = language;
			// Request restart
			std::string msg = string::f(string::translate("MenuBar.help.language.restart"), string::translate("language"));
			if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, msg.c_str())) {
				APP->window->close();
				settings::restart = true;
			}
		}));
	}
}


} // namespace app
} // namespace rack
