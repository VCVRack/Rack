#include "Nullovnik6.hpp"

Nullovnik6::Nullovnik6() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Nullovnik6::step() {
}

///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

json_t *Nullovnik6::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

    return rootJ;
}

void Nullovnik6::fromJson(json_t *rootJ) {
	json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
	panelStyle = json_integer_value(j_panelStyle);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Nullovnik6Widget::Nullovnik6Widget(Nullovnik6 *module) : ModuleWidget(module){

	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelWidget *panel = new DynamicPanelWidget();
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Nullovnik6-Dark.svg")));
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Nullovnik6-Light.svg")));
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

struct Nullovnik6PanelStyleItem : MenuItem {
    Nullovnik6* module;
    int panelStyle;
    void onAction(EventAction &e) override {
        module->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (module->panelStyle == panelStyle) ? "✔" : "";
        MenuItem::step();
    }
};

void Nullovnik6Widget::appendContextMenu(Menu *menu) {
    Nullovnik6 *module = dynamic_cast<Nullovnik6*>(this->module);
    assert(module);

    // Panel style
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Frame of mind"));
    menu->addChild(construct<Nullovnik6PanelStyleItem>(&MenuItem::text, "Dark Calm Night",
    	&Nullovnik6PanelStyleItem::module, module, &Nullovnik6PanelStyleItem::panelStyle, 0));
    menu->addChild(construct<Nullovnik6PanelStyleItem>(&MenuItem::text, "Happy Bright Day",
    	&Nullovnik6PanelStyleItem::module, module, &Nullovnik6PanelStyleItem::panelStyle, 1));

}

////////////////////////////////////////////////////////////////////////////////////////////////////

RACK_PLUGIN_MODEL_INIT(Koralfx, Nullovnik6) {
   Model *modelNullovnik6 = Model::create<Nullovnik6, Nullovnik6Widget>("Koralfx-Modules", "Nullovnik6", "Nullovnik 6",
                                                                        BLANK_TAG);
   return modelNullovnik6;
}
