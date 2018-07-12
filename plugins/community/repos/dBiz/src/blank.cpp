//// code suggested by Hetrick/////

#include "dBiz.hpp"

namespace rack_plugin_dBiz {

#define NUM_PANELS 5

struct dBizBlank : Module
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

	dBizBlank() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

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

struct dBizBlankWidget : ModuleWidget
{
	SVGPanel *panel1;
	SVGPanel *panel2;
	SVGPanel *panel3;
	SVGPanel *panel4;
	SVGPanel *panel5;
	dBizBlankWidget(dBizBlank *module);
	void step() override;
	Menu *createContextMenu() override;
};

dBizBlankWidget::dBizBlankWidget(dBizBlank *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    panel1 = new SVGPanel();
    panel1->box.size = box.size;
    panel1->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/dBizBlank3.svg")));
    addChild(panel1);

    panel2 = new SVGPanel();
    panel2->box.size = box.size;
    panel2->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/dBizBlank7.svg")));
    addChild(panel2);

    panel3 = new SVGPanel();
    panel3->box.size = box.size;
    panel3->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/dBizBlank5.svg")));
    addChild(panel3);

    panel4 = new SVGPanel();
    panel4->box.size = box.size;
    panel4->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/dBizBlank8.svg")));
    addChild(panel4);

    panel5 = new SVGPanel();
    panel5->box.size = box.size;
    panel5->setBackground(SVG::load(assetPlugin(plugin, "res/Blanks/dBizBlank1.svg")));
    addChild(panel5);

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
}

void dBizBlankWidget::step()
{
	dBizBlank *blank = dynamic_cast<dBizBlank*>(module);
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
	dBizBlank *blank;
	void onAction(EventAction &e) override { blank->panel = 0; }
	void step() override {
		rightText = (blank->panel == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel2Item : MenuItem
{
	dBizBlank *blank;
	void onAction(EventAction &e) override { blank->panel = 1; }
	void step() override {
		rightText = (blank->panel == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel3Item : MenuItem
{
	dBizBlank *blank;
	void onAction(EventAction &e) override { blank->panel = 2; }
	void step() override {
		rightText = (blank->panel == 2) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel4Item : MenuItem
{
	dBizBlank *blank;
	void onAction(EventAction &e) override { blank->panel = 3; }
	void step() override {
		rightText = (blank->panel == 3) ? "✔" : "";
		MenuItem::step();
	}
};

struct Panel5Item : MenuItem
{
	dBizBlank *blank;
	void onAction(EventAction &e) override { blank->panel = 4; }
	void step() override {
		rightText = (blank->panel == 4) ? "✔" : "";
		MenuItem::step();
	}
};

Menu *dBizBlankWidget::createContextMenu()
{
	Menu *menu = ModuleWidget::createContextMenu();

	dBizBlank *blank = dynamic_cast<dBizBlank*>(module);
	assert(blank);

    menu->addChild(construct<MenuEntry>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Panels Art"));
	menu->addChild(construct<Panel1Item>(&Panel1Item::text, "DeepBlue", &Panel1Item::blank, blank));
	menu->addChild(construct<Panel2Item>(&Panel2Item::text, "Flat Volume", &Panel2Item::blank, blank));
	menu->addChild(construct<Panel3Item>(&Panel3Item::text, "Circles", &Panel3Item::blank, blank));
    menu->addChild(construct<Panel4Item>(&Panel4Item::text, "Dark Wave",     &Panel4Item::blank, blank));
    menu->addChild(construct<Panel5Item>(&Panel5Item::text, "Clouds Pattern",    &Panel5Item::blank, blank));

	return menu;
}

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, dBizBlank) {
   Model *modeldBizBlank = Model::create<dBizBlank, dBizBlankWidget>("dBiz", "Blank", "Blank", BLANK_TAG);
   return modeldBizBlank;
}
