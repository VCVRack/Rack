
#include "Southpole.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_Southpole {

struct Fuse : Module {
	enum ParamIds {
		SWITCH1_PARAM,
		SWITCH2_PARAM,
		SWITCH3_PARAM,
		SWITCH4_PARAM,
	 	NUM_PARAMS 
	};

	enum InputIds {
		ARM1_INPUT,
		ARM2_INPUT,
		ARM3_INPUT,
		ARM4_INPUT,
		CLK_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
	 	NUM_OUTPUTS
	};
	
	enum LightIds {
		ARM1_LIGHT,
		ARM2_LIGHT,
		ARM3_LIGHT,
		ARM4_LIGHT,
		NUM_LIGHTS
	};

	bool gateMode;

	Fuse() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

        params.resize(NUM_PARAMS);
        inputs.resize(NUM_INPUTS);
        outputs.resize(NUM_OUTPUTS);
        lights.resize(NUM_LIGHTS);
	}

	void step() override;

  	SchmittTrigger clockTrigger;
  	SchmittTrigger resetTrigger;
  	SchmittTrigger armTrigger[4];
	PulseGenerator pulse[4];

	bool armed[4];
	bool gateOn[4];

	const unsigned maxsteps = 16;
	unsigned curstep = 0;


	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "gateMode", json_boolean( gateMode ));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *gateModeJ = json_object_get(rootJ, "gateMode");
		if (gateModeJ) {
			gateMode = json_boolean_value(gateModeJ);
		}
	}
};

void Fuse::step() {

  	bool nextStep = false;

	if (inputs[RESET_INPUT].active) {
		if (resetTrigger.process(inputs[RESET_INPUT].value)) {
			curstep = maxsteps;
			for (unsigned int i=0; i<4; i++) {
				armTrigger[i].reset();
				armed[i] = false;
				gateOn[i] = false;	
		  	}
		}
	}

	if (inputs[CLK_INPUT].active) {
		if (clockTrigger.process(inputs[CLK_INPUT].value)) {
			nextStep = true;
		}
	}

	if ( nextStep ) {
		curstep++;		
		if ( curstep >= maxsteps ) curstep = 0;

		if ( curstep % 4 == 0 ) {
			unsigned int i = curstep/4;
		 	gateOn[(i-1)%4] = false;

			if ( armed[i] ) {
				pulse[i].trigger(1e-3);
     			if ( gateMode ) gateOn[i] = true;
				armed[i] = false;
			}
		}		
    	//printf("%d %d\n",curstep,gateOn[curstep/4]);
	}

	for (unsigned int i=0; i<4; i++) {

		if ( params[SWITCH1_PARAM + i].value > 0. ) armed[i] = true;
		if ( armTrigger[i].process(inputs[ARM1_INPUT + i].normalize(0.))) armed[i] = true;
		
		lights[ARM1_LIGHT + i].setBrightness( armed[i] ? 1.0 : 0.0 );
			
		bool p = pulse[i].process(1.0 / engineGetSampleRate());		

		if (gateOn[i]) p = true;
		
		outputs[OUT1_OUTPUT + i].value =  p ? 10.0 : 0.0;
		
	}
};

struct FuseDisplay : TransparentWidget {

	Fuse *module;

	FuseDisplay() {}

	void draw(NVGcontext *vg) override {

		// Background
		NVGcolor backgroundColor = nvgRGB(0x30, 0x00, 0x10);
		NVGcolor borderColor = nvgRGB(0xd0, 0xd0, 0xd0);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.5);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		// Lights
		nvgStrokeColor(vg, nvgRGBA(0x7f, 0x00, 0x00, 0xff));
		nvgFillColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
		for ( unsigned y_ = 0; y_ < 16; y_++ ) {
			unsigned y = 15 - y_;
			nvgBeginPath(vg);
			nvgStrokeWidth(vg, 1.);
	    	nvgRect(vg, 3., y*box.size.y/18.+7.*floor(y/4.)+9., box.size.x-6., box.size.y/18.-6.);
			if (y_ <= module->curstep) nvgFill(vg);
			nvgStroke(vg);
		}

	}	
};

struct FuseWidget : ModuleWidget {
	Menu *createContextMenu() override;

   	FuseWidget(Fuse *module)  : ModuleWidget(module) {

		box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Fuse.svg")));
			addChild(panel);
		}

		{
			FuseDisplay *display = new FuseDisplay();
			display->module = module;
			display->box.pos = Vec( 32, 25.);
			display->box.size = Vec( 24., box.size.y-85. );
			addChild(display);
		}

		float y1 = 76;
		float yh = 73;
		float x1 = 5;	
		float x2 = 35;
		
		for(int i = 0; i < 4; i++)
		{
			addParam(ParamWidget::create<LEDButton>(Vec(x1+1, y1 + i*yh-22), module, Fuse::SWITCH1_PARAM + 3 - i, 0.0, 1.0, 0.0));
			addChild(ModuleLightWidget::create<MediumLight<YellowLight>>(Vec(x1+5, y1+ i*yh-18), module, Fuse::ARM1_LIGHT + 3 - i));
			addInput(Port::create<sp_Port>(Vec(x1, y1 + i*yh-45), Port::INPUT, module, Fuse::ARM1_INPUT + 3 - i));
			addOutput(Port::create<sp_Port>(Vec(x1, y1 + i*yh), Port::OUTPUT, module, Fuse::OUT1_OUTPUT + 3 - i));
		}

		addInput(Port::create<sp_Port>(Vec(x1, 330), Port::INPUT, module, Fuse::CLK_INPUT));
		addInput(Port::create<sp_Port>(Vec(x2, 330), Port::INPUT, module, Fuse::RESET_INPUT));
	}
};

struct FuseGateModeItem : MenuItem {
	Fuse *fuse;
	bool gateMode;
	void onAction(EventAction &e) override {
		fuse->gateMode = gateMode;
	}
	void step() override {
		rightText = (fuse->gateMode == gateMode) ? "✔" : "";
	}
};

Menu *FuseWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	Fuse *fuse = dynamic_cast<Fuse*>(module);
	assert(fuse);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Gate Mode";
	menu->addChild(modeLabel);

	FuseGateModeItem *triggerItem = new FuseGateModeItem();
	triggerItem->text = "Trigger";
	triggerItem->fuse = fuse;
	triggerItem->gateMode = false;
	menu->addChild(triggerItem);

	FuseGateModeItem *gateItem = new FuseGateModeItem();
	gateItem->text = "Gate";
	gateItem->fuse = fuse;
	gateItem->gateMode = true;
	menu->addChild(gateItem);

	return menu;
}

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Fuse) {
   Model *modelFuse 	= Model::create<Fuse,FuseWidget>(	 "Southpole", "Fuse", 		"Fuse - next pattern", SEQUENCER_TAG);
   return modelFuse;
}
