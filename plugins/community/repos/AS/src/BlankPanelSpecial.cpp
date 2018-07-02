// Original code from CV Hetrick Blank Panel

#include "AS.hpp"

#define NUM_PANELS 5

struct BlankPanelSpecial : Module
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

	BlankPanelSpecial() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override {}

	void reset() override
    {
        panel = 0;
	}
    void randomize() override
    {
        panel = round(randomUniform() * (NUM_PANELS - 1.0f));
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


struct BlankPanelSpecialWidget : ModuleWidget
{
    SVGPanel *panel1;
	SVGPanel *panel2;
    SVGPanel *panel3;
	SVGPanel *panel4;
    SVGPanel *panel5;

    BlankPanelSpecialWidget(BlankPanelSpecial *module);
    void step() override;
	Menu *createContextMenu() override;
};

BlankPanelSpecialWidget::BlankPanelSpecialWidget(BlankPanelSpecial *module) : ModuleWidget(module)
{
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    panel1 = new SVGPanel();
    panel1->box.size = box.size;
    panel1->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/as-BlankPanelSpecial0.svg")));
    addChild(panel1);

    panel2 = new SVGPanel();
    panel2->box.size = box.size;
    panel2->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/as-BlankPanelSpecial1.svg")));
    addChild(panel2);

    panel3 = new SVGPanel();
    panel3->box.size = box.size;
    panel3->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/as-BlankPanelSpecial2.svg")));
    addChild(panel3);

    panel4 = new SVGPanel();
    panel4->box.size = box.size;
    panel4->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/as-BlankPanelSpecial3.svg")));
    addChild(panel4);

    panel5 = new SVGPanel();
    panel5->box.size = box.size;
    panel5->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/as-BlankPanelSpecial4.svg")));
    addChild(panel5);


}

void BlankPanelSpecialWidget::step()
{
	BlankPanelSpecial *blank = dynamic_cast<BlankPanelSpecial*>(module);
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
	BlankPanelSpecial *blank;
	void onAction(EventAction &e) override { blank->panel = 0; }
	void step() override {
		rightText = (blank->panel == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel2Item : MenuItem
{
	BlankPanelSpecial *blank;
	void onAction(EventAction &e) override { blank->panel = 1; }
	void step() override {
		rightText = (blank->panel == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel3Item : MenuItem
{
	BlankPanelSpecial *blank;
	void onAction(EventAction &e) override { blank->panel = 2; }
	void step() override {
		rightText = (blank->panel == 2) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel4Item : MenuItem
{
	BlankPanelSpecial *blank;
	void onAction(EventAction &e) override { blank->panel = 3; }
	void step() override {
		rightText = (blank->panel == 3) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel5Item : MenuItem
{
	BlankPanelSpecial *blank;
	void onAction(EventAction &e) override { blank->panel = 4; }
	void step() override {
		rightText = (blank->panel == 4) ? "✔" : "";
		MenuItem::step();
	}
};

Menu *BlankPanelSpecialWidget::createContextMenu()
{
	Menu *menu = ModuleWidget::createContextMenu();

	BlankPanelSpecial *blank = dynamic_cast<BlankPanelSpecial*>(module);
	assert(blank);

    menu->addChild(construct<MenuEntry>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Not so blank panels"));
	menu->addChild(construct<Panel1Item>(&Panel1Item::text, "PANEL A", &Panel1Item::blank, blank));
    menu->addChild(construct<Panel2Item>(&Panel2Item::text, "PANEL B", &Panel2Item::blank, blank));
    menu->addChild(construct<Panel3Item>(&Panel3Item::text, "PANEL C", &Panel3Item::blank, blank));
    menu->addChild(construct<Panel4Item>(&Panel4Item::text, "PANEL D", &Panel4Item::blank, blank));
    menu->addChild(construct<Panel5Item>(&Panel5Item::text, "PANEL E", &Panel5Item::blank, blank));

	return menu;
}

RACK_PLUGIN_MODEL_INIT(AS, BlankPanelSpecial) {
   Model *modelBlankPanelSpecial = Model::create<BlankPanelSpecial, BlankPanelSpecialWidget>("AS", "BlankPanelSpecial", "Blank Panel Special");
   return modelBlankPanelSpecial;
}
