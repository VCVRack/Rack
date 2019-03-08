//***********************************************************************************************
//Four channel note viewer module for VCV Rack by Marc Boulé
//
//Based on code from the Fundamental and AudibleInstruments plugins by Andrew Belt 
//and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//***********************************************************************************************


#include "ImpromptuModular.hpp"

namespace rack_plugin_ImpromptuModular {

struct FourView : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(CV_INPUTS, 4),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CV_OUTPUTS, 4),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	// Need to save
	int panelTheme = 0;
	bool showSharp = true;

	// No need to save
	// none

	
	inline float quantize(float cv, bool enable) {
		return enable ? (roundf(cv * 12.0f) / 12.0f) : cv;
	}
	
	
	FourView() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}
	

	void onReset() override {
	}

	
	void onRandomize() override {
	}

	
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// showSharp
		json_object_set_new(rootJ, "showSharp", json_boolean(showSharp));
		
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// showSharp
		json_t *showSharpJ = json_object_get(rootJ, "showSharp");
		if (showSharpJ)
			showSharp = json_is_true(showSharpJ);
	}

	
	void step() override {
		for (int i = 0; i < 4; i++)
			outputs[CV_OUTPUTS + i].value = inputs[CV_INPUTS + i].value;
	}
};


struct FourViewWidget : ModuleWidget {
	FourView* module;

	struct NotesDisplayWidget : TransparentWidget {
		FourView* module;
		int baseIndex;
		std::shared_ptr<Font> font;
		char text[4];

		NotesDisplayWidget(Vec _pos, Vec _size, FourView* _module, int _baseIndex) {
			box.size = _size;
			box.pos = _pos.minus(_size.div(2));
			module = _module;
			baseIndex = _baseIndex;
			font = Font::load(assetPlugin(plugin, "res/fonts/Segment14.ttf"));
		}
		
		void cvToStr(int index2) {
			if (module->inputs[FourView::CV_INPUTS + baseIndex + index2].active) {
				float cvVal = module->inputs[FourView::CV_INPUTS + baseIndex + index2].value;
				printNote(cvVal, text, module->showSharp);
			}
			else
				snprintf(text, 4," - ");
		}

		void draw(NVGcontext *vg) override {
			NVGcolor textColor = prepareDisplay(vg, &box, 17);
			nvgFontFaceId(vg, font->handle);
			nvgTextLetterSpacing(vg, -1.5);

			for (int i = 0; i < 2; i++) {
				Vec textPos = Vec(7.0f + i * 46.0f, 23.4f);
				nvgFillColor(vg, nvgTransRGBA(textColor, displayAlpha));
				nvgText(vg, textPos.x, textPos.y, "~~~", NULL);
				nvgFillColor(vg, textColor);
				cvToStr(i);
				nvgText(vg, textPos.x, textPos.y, text, NULL);
			}
		}
	};


	struct PanelThemeItem : MenuItem {
		FourView *module;
		int theme;
		void onAction(EventAction &e) override {
			module->panelTheme = theme;
		}
		void step() override {
			rightText = (module->panelTheme == theme) ? "✔" : "";
		}
	};
	struct SharpItem : MenuItem {
		FourView *module;
		void onAction(EventAction &e) override {
			module->showSharp = !module->showSharp;
		}
	};
	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		FourView *module = dynamic_cast<FourView*>(this->module);
		assert(module);

		MenuLabel *themeLabel = new MenuLabel();
		themeLabel->text = "Panel Theme";
		menu->addChild(themeLabel);

		PanelThemeItem *lightItem = new PanelThemeItem();
		lightItem->text = lightPanelID;// ImpromptuModular.hpp
		lightItem->module = module;
		lightItem->theme = 0;
		menu->addChild(lightItem);

		PanelThemeItem *darkItem = new PanelThemeItem();
		darkItem->text = darkPanelID;// ImpromptuModular.hpp
		darkItem->module = module;
		darkItem->theme = 1;
		menu->addChild(darkItem);

		menu->addChild(new MenuLabel());// empty line
		
		MenuLabel *settingsLabel = new MenuLabel();
		settingsLabel->text = "Settings";
		menu->addChild(settingsLabel);
		
		SharpItem *shrpItem = MenuItem::create<SharpItem>("Sharp (unchecked is flat)", CHECKMARK(module->showSharp));
		shrpItem->module = module;
		menu->addChild(shrpItem);
		
		return menu;
	}	
	
	
	FourViewWidget(FourView *module) : ModuleWidget(module) {
		this->module = module;
		
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/FourView.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/FourView_dark.svg")));
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);

		// Screws
		addChild(createDynamicScrew<IMScrew>(Vec(15, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 0), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(15, 365), &module->panelTheme));
		addChild(createDynamicScrew<IMScrew>(Vec(box.size.x-30, 365), &module->panelTheme));

		const int centerX = box.size.x / 2;

		// Notes display
		NotesDisplayWidget *displayNotes12 = new NotesDisplayWidget(Vec(centerX, 66), Vec(99, 29), module, 0);
		addChild(displayNotes12);
		NotesDisplayWidget *displayNotes34 = new NotesDisplayWidget(Vec(centerX, 122), Vec(99, 29), module, 2);
		addChild(displayNotes34);

		static const int rowRulerPort = 193;
		static const int portSpacingY = 45;
		static const int portOffsetX = 28;
		
		for (int i = 0; i < 4; i++) {
			addInput(createDynamicPortCentered<IMPort>(Vec(centerX - portOffsetX, rowRulerPort + i * portSpacingY), Port::INPUT, module, FourView::CV_INPUTS + i, &module->panelTheme));	
			addOutput(createDynamicPortCentered<IMPort>(Vec(centerX + portOffsetX, rowRulerPort + i * portSpacingY), Port::OUTPUT, module, FourView::CV_OUTPUTS + i, &module->panelTheme));
		}
	}
};

} // namespace rack_plugin_ImpromptuModular

using namespace rack_plugin_ImpromptuModular;

RACK_PLUGIN_MODEL_INIT(ImpromptuModular, FourView) {
   Model *modelFourView = Model::create<FourView, FourViewWidget>("Impromptu Modular", "Four-View", "VIS - Four-View", VISUAL_TAG);
   return modelFourView;
}

/*CHANGE LOG

0.6.13:
created

*/

