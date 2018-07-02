
#include "template_panels.hpp"

struct ThreeHPWidget : ModuleWidget {
  ThreeHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 3, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/ThreeHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));
  }
};

Model* modelThreeHP = Model::create<Module, ThreeHPWidget>("Bogaudio", "Bogaudio-ThreeHP", "3HP");


struct SixHPWidget : ModuleWidget {
  SixHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 6, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/SixHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));
  }
};

Model* modelSixHP = Model::create<Module, SixHPWidget>("Bogaudio", "Bogaudio-SixHP", "6HP");


struct EightHPWidget : ModuleWidget {
  EightHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 8, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/EightHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));
  }
};

Model* modelEightHP = Model::create<Module, EightHPWidget>("Bogaudio", "Bogaudio-EightHP", "8HP");


struct TenHPWidget : ModuleWidget {
	TenHPWidget(Module* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * 10, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/TenHP.svg")));
			addChild(panel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(0, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));
	}
};

Model* modelTenHP = Model::create<Module, TenHPWidget>("Bogaudio", "Bogaudio-TenHP", "10HP");


struct TwelveHPWidget : ModuleWidget {
  TwelveHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 12, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TwelveHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(0, 365)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));
  }
};

Model* modelTwelveHP = Model::create<Module, TwelveHPWidget>("Bogaudio", "Bogaudio-TwelveHP", "12HP");

struct ThirteenHPWidget : ModuleWidget {
  ThirteenHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 13, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/ThirteenHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(0, 365)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));
  }
};

Model* modelThirteenHP = Model::create<Module, ThirteenHPWidget>("Bogaudio", "Bogaudio-ThirteenHP", "13HP");


struct FifteenHPWidget : ModuleWidget {
  FifteenHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 15, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/FifteenHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
  }
};

Model* modelFifteenHP = Model::create<Module, FifteenHPWidget>("Bogaudio", "Bogaudio-FifteenHP", "15HP");


struct EighteenHPWidget : ModuleWidget {
  EighteenHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 18, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/EighteenHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
  }
};

Model* modelEighteenHP = Model::create<Module, EighteenHPWidget>("Bogaudio", "Bogaudio-EighteenHP", "18HP");


struct TwentyHPWidget : ModuleWidget {
  TwentyHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 20, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TwentyHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
  }
};

Model* modelTwentyHP = Model::create<Module, TwentyHPWidget>("Bogaudio", "Bogaudio-TwentyHP", "20HP");


struct TwentyTwoHPWidget : ModuleWidget {
  TwentyTwoHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 22, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TwentyTwoHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
  }
};

Model* modelTwentyTwoHP = Model::create<Module, TwentyTwoHPWidget>("Bogaudio", "Bogaudio-TwentyTwoHP", "22HP");


struct TwentyFiveHPWidget : ModuleWidget {
  TwentyFiveHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 25, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TwentyFiveHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
  }
};

Model* modelTwentyFiveHP = Model::create<Module, TwentyFiveHPWidget>("Bogaudio", "Bogaudio-TwentyFiveHP", "25HP");


struct ThirtyHPWidget : ModuleWidget {
  ThirtyHPWidget(Module* module) : ModuleWidget(module) {
  	box.size = Vec(RACK_GRID_WIDTH * 30, RACK_GRID_HEIGHT);

  	{
  		SVGPanel *panel = new SVGPanel();
  		panel->box.size = box.size;
  		panel->setBackground(SVG::load(assetPlugin(plugin, "res/ThirtyHP.svg")));
  		addChild(panel);
  	}

  	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
  }
};

Model* modelThirtyHP = Model::create<Module, ThirtyHPWidget>("Bogaudio", "Bogaudio-ThirtyHP", "30HP");
