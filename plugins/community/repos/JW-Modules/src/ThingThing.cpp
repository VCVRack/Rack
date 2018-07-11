#include <string.h>
#include "JWModules.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_JW_Modules {

struct ThingThingBall {
	NVGcolor color;
};

struct ThingThing : Module {
	enum ParamIds {
		BALL_RAD_PARAM,
		ZOOM_MULT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		BALL_RAD_INPUT,
		ZOOM_MULT_INPUT,
		ANG_INPUT,
		NUM_INPUTS = ANG_INPUT + 5
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	ThingThingBall *balls = new ThingThingBall[5];
	float atten[5] = {1, 1, 1, 1, 1};
	// float atten[5] = {0.0, 0.25, 0.5, 0.75, 1};

	ThingThing() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		balls[0].color = nvgRGB(255, 255, 255);//white
		balls[1].color = nvgRGB(255, 151, 9);//orange
		balls[2].color = nvgRGB(255, 243, 9);//yellow
		balls[3].color = nvgRGB(144, 26, 252);//purple
		balls[4].color = nvgRGB(25, 150, 252);//blue
	}
	~ThingThing() {
		delete [] balls;
	}

	void step() override {};
	void reset() override {}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {}
};

struct ThingThingDisplay : Widget {
	ThingThing *module;
	ThingThingDisplay(){}

	void draw(NVGcontext *vg) override {
		//background
		nvgFillColor(vg, nvgRGB(20, 30, 33));
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFill(vg);

		float ballRadius = module->params[ThingThing::BALL_RAD_PARAM].value;
		if(module->inputs[ThingThing::BALL_RAD_INPUT].active){
			ballRadius += rescalefjw(module->inputs[ThingThing::BALL_RAD_INPUT].value, -5.0, 5.0, 0.0, 30.0);
		}

		float zoom = module->params[ThingThing::ZOOM_MULT_PARAM].value;
		if(module->inputs[ThingThing::ZOOM_MULT_INPUT].active){
			ballRadius += rescalefjw(module->inputs[ThingThing::ZOOM_MULT_INPUT].value, -5.0, 5.0, 1.0, 50.0);
		}

      float x[5];
      float y[5];
      float angle[5];

      for(int i=0; i<5; i++){
         angle[i] = i==0 ? 0 : (module->inputs[ThingThing::ANG_INPUT+i].value + angle[i-1]) * module->atten[i];
			x[i] = i==0 ? 0 : sinf(rescalefjw(angle[i], -5, 5, -2*M_PI + M_PI/2.0f, 2*M_PI + M_PI/2.0f)) * zoom;
			y[i] = i==0 ? 0 : cosf(rescalefjw(angle[i], -5, 5, -2*M_PI + M_PI/2.0f, 2*M_PI + M_PI/2.0f)) * zoom;
      }

		/////////////////////// LINES ///////////////////////
		nvgSave(vg);
		nvgTranslate(vg, box.size.x * 0.5, box.size.y * 0.5);
		for(int i=0; i<5; i++){
			nvgTranslate(vg, x[i], y[i]);
			nvgStrokeColor(vg, nvgRGB(255, 255, 255));
			if(i>0){
				nvgStrokeWidth(vg, 1);
				nvgBeginPath(vg);
				nvgMoveTo(vg, 0, 0);
				nvgLineTo(vg, -x[i], -y[i]);
				nvgStroke(vg);
			}
		}
		nvgRestore(vg);

		/////////////////////// BALLS ///////////////////////
		nvgSave(vg);
		nvgTranslate(vg, box.size.x * 0.5, box.size.y * 0.5);
		for(int i=0; i<5; i++){
			nvgTranslate(vg, x[i], y[i]);
			nvgStrokeColor(vg, module->balls[i].color);
			nvgFillColor(vg, module->balls[i].color);
			nvgStrokeWidth(vg, 2);
			nvgBeginPath(vg);
			nvgCircle(vg, 0, 0, ballRadius);
			nvgFill(vg);
			nvgStroke(vg);
		}
		nvgRestore(vg);
	}
};


struct ThingThingWidget : ModuleWidget {
	ThingThingWidget(ThingThing *module);
};

ThingThingWidget::ThingThingWidget(ThingThing *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*20, RACK_GRID_HEIGHT);

	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/ThingThing.svg")));
	addChild(panel);

	ThingThingDisplay *display = new ThingThingDisplay();
	display->module = module;
	display->box.pos = Vec(0, 0);
	display->box.size = Vec(box.size.x, RACK_GRID_HEIGHT);
	addChild(display);

	addChild(Widget::create<Screw_J>(Vec(265, 365)));
	addChild(Widget::create<Screw_W>(Vec(280, 365)));

	for(int i=0; i<4; i++){
		addInput(Port::create<TinyPJ301MPort>(Vec(5+(20*i), 360), Port::INPUT, module, ThingThing::ANG_INPUT+i+1));
	}
	
	addInput(Port::create<TinyPJ301MPort>(Vec(140, 360), Port::INPUT, module, ThingThing::BALL_RAD_INPUT));
	addParam(ParamWidget::create<JwTinyKnob>(Vec(155, 360), module, ThingThing::BALL_RAD_PARAM, 0.0, 30.0, 10.0));

	addInput(Port::create<TinyPJ301MPort>(Vec(190, 360), Port::INPUT, module, ThingThing::ZOOM_MULT_INPUT));
	addParam(ParamWidget::create<JwTinyKnob>(Vec(205, 360), module, ThingThing::ZOOM_MULT_PARAM, 1.0, 200.0, 20.0));
}

} // namespace rack_plugin_JW_Modules

using namespace rack_plugin_JW_Modules;

RACK_PLUGIN_MODEL_INIT(JW_Modules, ThingThing) {
   Model *modelThingThing = Model::create<ThingThing, ThingThingWidget>("JW-Modules", "ThingThing", "Thing Thing", VISUAL_TAG);
   return modelThingThing;
}
