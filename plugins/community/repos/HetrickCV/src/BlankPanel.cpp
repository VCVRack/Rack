#include "HetrickCV.hpp"

#define NUM_PANELS 5

namespace rack_plugin_HetrickCV {

struct BlankPanel : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
		NUM_INPUTS
	};
	enum OutputIds
	{
		NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
	};

    int panel = 0;

	BlankPanel() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override {}

	void reset() override
    {
        panel = 0;
	}
    void randomize() override
    {
        panel = round(randomf() * (NUM_PANELS - 1.0f));
    }

    json_t *toJson() override
    {
		json_t *rootJ = json_object();
        json_object_set_new(rootJ, "panel", json_integer(panel));
		return rootJ;
	}
    void fromJson(json_t *rootJ) override
    {
		json_t *panelJ = json_object_get(rootJ, "panel");
		if (panelJ)
            panel = json_integer_value(panelJ);
	}
};


struct BlankPanelWidget : ModuleWidget
{
    SVGPanel *panel1;
	SVGPanel *panel2;
    SVGPanel *panel3;
	SVGPanel *panel4;
    SVGPanel *panel5;
    BlankPanelWidget(BlankPanel *module);
    void step() override;
	Menu *createContextMenu() override;
};

BlankPanelWidget::BlankPanelWidget(BlankPanel *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    panel1 = new SVGPanel();
    panel1->box.size = box.size;
    panel1->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/BlankPanel3.svg")));
    addChild(panel1);

    panel2 = new SVGPanel();
    panel2->box.size = box.size;
    panel2->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/BlankPanel7.svg")));
    addChild(panel2);

    panel3 = new SVGPanel();
    panel3->box.size = box.size;
    panel3->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/BlankPanel2.svg")));
    addChild(panel3);

    panel4 = new SVGPanel();
    panel4->box.size = box.size;
    panel4->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/BlankPanel8.svg")));
    addChild(panel4);

    panel5 = new SVGPanel();
    panel5->box.size = box.size;
    panel5->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/BlankPanel1.svg")));
    addChild(panel5);

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
}

void BlankPanelWidget::step()
{
	BlankPanel *blank = dynamic_cast<BlankPanel*>(module);
	assert(blank);

	panel1->visible = (blank->panel == 0);
	panel2->visible = (blank->panel == 1);
   panel3->visible = (blank->panel == 2);
	panel4->visible = (blank->panel == 3);
   panel5->visible = (blank->panel == 4);
   
	ModuleWidget::step();
}

struct Panel1Item : MenuItem
{
	BlankPanel *blank;
	void onAction(EventAction &e) override { blank->panel = 0; }
	void step() override {
		rightText = (blank->panel == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel2Item : MenuItem
{
	BlankPanel *blank;
	void onAction(EventAction &e) override { blank->panel = 1; }
	void step() override {
		rightText = (blank->panel == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel3Item : MenuItem
{
	BlankPanel *blank;
	void onAction(EventAction &e) override { blank->panel = 2; }
	void step() override {
		rightText = (blank->panel == 2) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel4Item : MenuItem
{
	BlankPanel *blank;
	void onAction(EventAction &e) override { blank->panel = 3; }
	void step() override {
		rightText = (blank->panel == 3) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel5Item : MenuItem
{
	BlankPanel *blank;
	void onAction(EventAction &e) override { blank->panel = 4; }
	void step() override {
		rightText = (blank->panel == 4) ? "✔" : "";
		MenuItem::step();
	}
};

Menu *BlankPanelWidget::createContextMenu()
{
	Menu *menu = ModuleWidget::createContextMenu();

	BlankPanel *blank = dynamic_cast<BlankPanel*>(module);
	assert(blank);

   menu->addChild(construct<MenuEntry>());
   menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Panel Art"));
	menu->addChild(construct<Panel1Item>(&Panel1Item::text, "Sideways Logo", &Panel1Item::blank, blank));
   menu->addChild(construct<Panel2Item>(&Panel2Item::text, "Bleeding Edge", &Panel2Item::blank, blank));
   menu->addChild(construct<Panel3Item>(&Panel3Item::text, "Hetrick Stack", &Panel3Item::blank, blank));
   menu->addChild(construct<Panel4Item>(&Panel4Item::text, "Simple CV",     &Panel4Item::blank, blank));
   menu->addChild(construct<Panel5Item>(&Panel5Item::text, "Plain Jane",    &Panel5Item::blank, blank));

	return menu;
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, BlankPanel) {
   Model *modelBlankPanel = Model::create<BlankPanel, BlankPanelWidget>("HetrickCV", "BlankPanel", "Blank Panel");
   return modelBlankPanel;
}
