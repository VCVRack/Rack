//***********************************************************************************************
//Blank-Panel Info for VCV Rack by Pierre Collard and Marc Boulé
//***********************************************************************************************

#include "Geodesics.hpp"

namespace rack_plugin_Geodesics {

struct BlankInfo : Module {

	int panelTheme = 0;


	BlankInfo() : Module(0, 0, 0, 0) {
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

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);
	}

	
	// Advances the module by 1 audio frame with duration 1.0 / engineGetSampleRate()
	void step() override {		
	}
};


struct BlankInfoWidget : ModuleWidget {

	struct PanelThemeItem : MenuItem {
		BlankInfo *module;
		int theme;
		void onAction(EventAction &e) override {
			module->panelTheme = theme;
		}
		void step() override {
			rightText = (module->panelTheme == theme) ? "✔" : "";
		}
	};
	Menu *createContextMenu() override {
		Menu *menu = ModuleWidget::createContextMenu();

		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		BlankInfo *module = dynamic_cast<BlankInfo*>(this->module);
		assert(module);

		MenuLabel *themeLabel = new MenuLabel();
		themeLabel->text = "Panel Theme";
		menu->addChild(themeLabel);

		PanelThemeItem *lightItem = new PanelThemeItem();
		lightItem->text = lightPanelID;// Geodesics.hpp
		lightItem->module = module;
		lightItem->theme = 0;
		menu->addChild(lightItem);

		PanelThemeItem *darkItem = new PanelThemeItem();
		darkItem->text = darkPanelID;// Geodesics.hpp
		darkItem->module = module;
		darkItem->theme = 1;
		//menu->addChild(darkItem);

		return menu;
	}	


	BlankInfoWidget(BlankInfo *module) : ModuleWidget(module) {
		// Main panel from Inkscape
        DynamicSVGPanel *panel = new DynamicSVGPanel();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/light/BlankInfo-01.svg")));
        //panel->addPanel(SVG::load(assetPlugin(plugin, "res/dark/BlankInfo-02.svg")));// no dark pannel for now
        box.size = panel->box.size;
        panel->mode = &module->panelTheme;
        addChild(panel);

		// Screws
		// part of svg panel, no code required
	}
};

} // namespace rack_plugin_Geodesics

using namespace rack_plugin_Geodesics;

RACK_PLUGIN_MODEL_INIT(Geodesics, BlankInfo) {
   Model *modelBlankInfo = Model::create<BlankInfo, BlankInfoWidget>("Geodesics", "Blank-Panel Info", "MISC - Blank-Panel Info", BLANK_TAG);
   return modelBlankInfo;
}
