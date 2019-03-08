#include "JWModules.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_JW_Modules {

struct Cat : Module {
	enum ParamIds {
		BOWL_PARAM,
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

	int catY = 0;
	bool goingDown = true;

	bool invert = true;
	bool neg5ToPos5 = false;
	Cat() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	
	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "invert", json_boolean(invert));
		json_object_set_new(rootJ, "neg5ToPos5", json_boolean(neg5ToPos5));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *invertJ = json_object_get(rootJ, "invert");
		if (invertJ){ invert = json_is_true(invertJ); }

		json_t *neg5ToPos5J = json_object_get(rootJ, "neg5ToPos5");
		if (neg5ToPos5J){ neg5ToPos5 = json_is_true(neg5ToPos5J); }
	}

};

struct CatWidget : ModuleWidget {
	CatWidget(Cat *module);
	void step();
	Widget* widgetToMove;
	Widget* hairballs[10];
	Menu *createContextMenu();
};

void CatWidget::step() {
	Cat *cat = dynamic_cast<Cat*>(module);
	widgetToMove->box.pos.y = cat->catY;

	
	if(cat->goingDown){
		cat->catY+=2;
		if(cat->catY > 250){
			cat->goingDown = false;
		}
	} else {
		cat->catY-=2;
		if(cat->catY < 15){
			cat->goingDown = true;
		}
	}

	for(int i=0; i<10; i++){
		if(hairballs[i]->box.pos.y > box.size.y*1.5 && !bool(cat->params[Cat::BOWL_PARAM].value)){
			hairballs[i]->box.pos.y = widgetToMove->box.pos.y;
		} else {
			hairballs[i]->box.pos.y += randomUniform()*10;
		}
	}
};

CatWidget::CatWidget(Cat *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*4, RACK_GRID_HEIGHT);

	LightPanel *panel = new LightPanel();
	panel->box.size = box.size;
	addChild(panel);

	widgetToMove = Widget::create<CatScrew>(Vec(5, 250));
	addChild(widgetToMove);
	addChild(Widget::create<Screw_J>(Vec(16, 1)));
	addChild(Widget::create<Screw_J>(Vec(16, 365)));
	addChild(Widget::create<Screw_W>(Vec(box.size.x-29, 1)));
	addChild(Widget::create<Screw_W>(Vec(box.size.x-29, 365)));

	addParam(ParamWidget::create<BowlSwitch>(Vec(5, 300), module, Cat::BOWL_PARAM, 0.0, 1.0, 0.0));

	for(int i=0; i<10; i++){
		hairballs[i] = Widget::create<HairballScrew>(Vec(randomUniform()*7, widgetToMove->box.pos.y));
		addChild(hairballs[i]);
	}
}

struct InvertMenuItem : MenuItem {
	Cat *cat;
	void onAction(EventAction &e) override {
		cat->invert = !cat->invert;
	}
	void step() override {
		rightText = cat->invert ? "✔" : "";
	}
};

struct Neg5MenuItem : MenuItem {
	Cat *cat;
	void onAction(EventAction &e) override {
		cat->neg5ToPos5 = !cat->neg5ToPos5;
	}
	void step() override {
		rightText = cat->neg5ToPos5 ? "✔" : "";
	}
};

Menu *CatWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();
	Cat *cat = dynamic_cast<Cat*>(module);

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	InvertMenuItem *invertMenuItem = new InvertMenuItem();
	invertMenuItem->text = "Invert";
	invertMenuItem->cat = cat;
	menu->addChild(invertMenuItem);

	Neg5MenuItem *neg5MenuItem = new Neg5MenuItem();
	neg5MenuItem->text = "-5 to +5";
	neg5MenuItem->cat = cat;
	menu->addChild(neg5MenuItem);

	return menu;
}

} // namespace rack_plugin_JW_Modules

using namespace rack_plugin_JW_Modules;

RACK_PLUGIN_MODEL_INIT(JW_Modules, Cat) {
   Model *modelCat = Model::create<Cat, CatWidget>("JW-Modules", "0Cat", "0Cat (WARNING: Removes Wires)", VISUAL_TAG);
   return modelCat;
}
