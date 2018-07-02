#include "Nullovnik4.hpp"

Nullovnik4::Nullovnik4() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Nullovnik4::step() {
}

///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

json_t *Nullovnik4::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

    return rootJ;
}

void Nullovnik4::fromJson(json_t *rootJ) {
	json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
	panelStyle = json_integer_value(j_panelStyle);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Nullovnik4Widget::Nullovnik4Widget(Nullovnik4 *module) : ModuleWidget(module){

	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelWidget *panel = new DynamicPanelWidget();
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Nullovnik4-Dark.svg")));
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Nullovnik4-Light.svg")));
		box.size = panel->box.size;
		panel->mode = &module->panelStyle;
		addChild(panel);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Context Menu ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct Nullovnik4PanelStyleItem : MenuItem {
    Nullovnik4* module;
    int panelStyle;
    void onAction(EventAction &e) override {
        module->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (module->panelStyle == panelStyle) ? "✔" : "";
        MenuItem::step();
    }
};

void Nullovnik4Widget::appendContextMenu(Menu *menu) {
    Nullovnik4 *module = dynamic_cast<Nullovnik4*>(this->module);
    assert(module);

    // Panel style
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Frame of mind"));
    menu->addChild(construct<Nullovnik4PanelStyleItem>(&MenuItem::text, "Dark Calm Night",
    	&Nullovnik4PanelStyleItem::module, module, &Nullovnik4PanelStyleItem::panelStyle, 0));
    menu->addChild(construct<Nullovnik4PanelStyleItem>(&MenuItem::text, "Happy Bright Day",
    	&Nullovnik4PanelStyleItem::module, module, &Nullovnik4PanelStyleItem::panelStyle, 1));

}

////////////////////////////////////////////////////////////////////////////////////////////////////

RACK_PLUGIN_MODEL_INIT(Koralfx, Nullovnik4) {
   Model *modelNullovnik4 = Model::create<Nullovnik4, Nullovnik4Widget>("Koralfx-Modules", "Nullovnik4", "Nullovnik 4",
                                                                        BLANK_TAG);
   return modelNullovnik4;
}
