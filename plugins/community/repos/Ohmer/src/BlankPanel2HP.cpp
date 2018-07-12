////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 2 HP module /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

namespace rack_plugin_Ohmer {

struct OhmerBlank2 : Module {
	enum ParamIds {
		NUM_PARAMS
	};	
	enum InputIds {
		NUM_INPUTS
	};	
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	unsigned int screwsPos = 0; // Screws setting (default is two screws: top/left and bottom/right).
	OhmerBlank2() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// Persistence for extra data (screws setting). This extra data belongs .vcv file (including "autosave.vcv") and while cloning module.

	json_t *toJson() override	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "screwsPos", json_integer(screwsPos));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override	{
		// Retrieving screws setting (when loading .vcv and cloning module).
		json_t *screwsPosJ = json_object_get(rootJ, "screwsPos");
		if (screwsPosJ)
			screwsPos = json_integer_value(screwsPosJ);
	}

};

void OhmerBlank2::step() {
}

struct OhmerBlank2Widget : ModuleWidget {
	// Silver Torx screws.
	SVGScrew *topLeftScrewSilver;
	SVGScrew *topRightScrewSilver;
	SVGScrew *bottomLeftScrewSilver;
	SVGScrew *bottomRightScrewSilver;
	//
	OhmerBlank2Widget(OhmerBlank2 *module);
	void step() override;
	Menu* createContextMenu() override;
};

OhmerBlank2Widget::OhmerBlank2Widget(OhmerBlank2 *module) : ModuleWidget(module) {
	box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	setPanel(SVG::load(assetPlugin(plugin, "res/Blank_2HP.svg")));
	// Top-left silver screw, may be visible or hidden, depending KlokSpid model.
	topLeftScrewSilver = Widget::create<Torx_Silver>(Vec(0, 0));
	addChild(topLeftScrewSilver);
	// Top-right silver screw, may be visible or hidden, depending KlokSpid model.
	topRightScrewSilver = Widget::create<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, 0));
	addChild(topRightScrewSilver);
	// Bottom-left silver screw, may be visible or hidden, depending KlokSpid model.
	bottomLeftScrewSilver = Widget::create<Torx_Silver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomLeftScrewSilver);
	// Bottom-right silver screw, may be visible or hidden, depending KlokSpid model.
	bottomRightScrewSilver = Widget::create<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomRightScrewSilver);
}

//// Screw setting, depending context-menu choice.

void OhmerBlank2Widget::step() {
	OhmerBlank2 *ohmerblank2 = dynamic_cast<OhmerBlank2*>(module);
	assert(ohmerblank2);
	// Silver Torx screws visible or hidden, depending screws setting from context-menu.
	topLeftScrewSilver->visible = (ohmerblank2->screwsPos == 0) || (ohmerblank2->screwsPos == 2);
	topRightScrewSilver->visible = (ohmerblank2->screwsPos == 1) || (ohmerblank2->screwsPos == 2);
	bottomLeftScrewSilver->visible = (ohmerblank2->screwsPos == 1) || (ohmerblank2->screwsPos == 2);
	bottomRightScrewSilver->visible = (ohmerblank2->screwsPos == 0) || (ohmerblank2->screwsPos == 2);
	ModuleWidget::step();
}

// First screw setting (2: top/left and bottom/right).
struct screwsTLBRMenuItem : MenuItem {
	OhmerBlank2 *ohmerblank2;
	void onAction(EventAction &e) override {
		ohmerblank2->screwsPos = 0;
	}
	void step() override {
		rightText = (ohmerblank2->screwsPos == 0) ? "✔" : "";
		MenuItem::step();
	}
};

// Second screw setting (2: top/right and bottom/left).
struct screwsTRBLMenuItem : MenuItem {
	OhmerBlank2 *ohmerblank2;
	void onAction(EventAction &e) override {
		ohmerblank2->screwsPos = 1;
	}
	void step() override {
		rightText = (ohmerblank2->screwsPos == 1) ? "✔" : "";
		MenuItem::step();
	}
};

// Third screw setting (4: all corners).
struct screwsCornersMenuItem : MenuItem {
	OhmerBlank2 *ohmerblank2;
	void onAction(EventAction &e) override {
		ohmerblank2->screwsPos = 2;
	}
	void step() override {
		rightText = (ohmerblank2->screwsPos == 2) ? "✔" : "";
		MenuItem::step();
	}
};

// Context-menu construction.

Menu* OhmerBlank2Widget::createContextMenu() {
	Menu* menu = ModuleWidget::createContextMenu();
	OhmerBlank2 *ohmerblank2 = dynamic_cast<OhmerBlank2*>(module);
	assert(ohmerblank2);
	menu->addChild(construct<MenuEntry>());
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Screws number & arrangement:"));
	menu->addChild(construct<screwsTLBRMenuItem>(&screwsTLBRMenuItem::text, "2: top/left, bottom/right", &screwsTLBRMenuItem::ohmerblank2, ohmerblank2));
	menu->addChild(construct<screwsTRBLMenuItem>(&screwsTRBLMenuItem::text, "2: top/right, bottom/left", &screwsTRBLMenuItem::ohmerblank2, ohmerblank2));
	menu->addChild(construct<screwsCornersMenuItem>(&screwsCornersMenuItem::text, "Four screws", &screwsCornersMenuItem::ohmerblank2, ohmerblank2));
	return menu;
}

} // namespace rack_plugin_Ohmer

using namespace rack_plugin_Ohmer;

RACK_PLUGIN_MODEL_INIT(Ohmer, BlankPanel2) {
   Model *modelBlankPanel2 = Model::create<OhmerBlank2, OhmerBlank2Widget>("Ohmer Modules", "OhmerBlank2", "2 HP Blank Panel", BLANK_TAG);
   return modelBlankPanel2;
}
