#include "Fundamental.hpp"

struct Octave : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int octave = 0;

	Octave() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void onReset() override {
		octave = 0;
	}

	void onRandomize() override {
		octave = (randomu32() % 9) - 4;
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "octave", json_integer(octave));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *octaveJ = json_object_get(rootJ, "octave");
		if (octaveJ)
			octave = json_integer_value(octaveJ);
	}

	void step() override {
		float cv = inputs[CV_INPUT].value;
		cv += octave;
		outputs[CV_OUTPUT].value = cv;
	}
};


struct OctaveButton : OpaqueWidget {
	Octave *module;
	int octave;

	void draw(NVGcontext *vg) override {
		Vec c = box.size.div(2);

		if (module->octave == octave) {
			// Enabled
			nvgBeginPath(vg);
			nvgCircle(vg, c.x, c.y, mm2px(4.0/2));
			if (octave < 0)
				nvgFillColor(vg, COLOR_RED);
			else if (octave > 0)
				nvgFillColor(vg, COLOR_GREEN);
			else
				nvgFillColor(vg, colorAlpha(COLOR_WHITE, 0.33));
			nvgFill(vg);
		}
		else {
			// Disabled
			nvgBeginPath(vg);
			nvgCircle(vg, c.x, c.y, mm2px(4.0/2));
			nvgFillColor(vg, colorAlpha(COLOR_WHITE, 0.33));
			nvgFill(vg);

			nvgBeginPath(vg);
			nvgCircle(vg, c.x, c.y, mm2px(3.0/2));
			nvgFillColor(vg, COLOR_BLACK);
			nvgFill(vg);

			if (octave == 0) {
				nvgBeginPath(vg);
				nvgCircle(vg, c.x, c.y, mm2px(1.0/2));
				nvgFillColor(vg, colorAlpha(COLOR_WHITE, 0.33));
				nvgFill(vg);
			}
		}
	}

	void onMouseDown(EventMouseDown &e) override {
		if (e.button == 1) {
			module->octave = 0;
			e.target = this;
			e.consumed = true;
			return;
		}
		OpaqueWidget::onMouseDown(e);
	}

	void onDragEnter(EventDragEnter &e) override {
		OctaveButton *w = dynamic_cast<OctaveButton*>(e.origin);
		if (w) {
			module->octave = octave;
		}
	}
};


struct OctaveDisplay : OpaqueWidget {
	OctaveDisplay() {
		box.size = mm2px(Vec(15.240, 72.000));
	}

	void setModule(Octave *module) {
		clearChildren();

		const int octaves = 9;
		const float margin = mm2px(2.0);
		float height = box.size.y - 2*margin;
		for (int i = 0; i < octaves; i++) {
			OctaveButton *octaveButton = new OctaveButton();
			octaveButton->box.pos = Vec(0, height / octaves * i + margin);
			octaveButton->box.size = Vec(box.size.x, height / octaves);
			octaveButton->module = module;
			octaveButton->octave = 4 - i;
			addChild(octaveButton);
		}
	}

	void draw(NVGcontext *vg) override {
		// Background
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFill(vg);

		Widget::draw(vg);
	}
};


struct OctaveWidget : ModuleWidget {
	OctaveWidget(Octave *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Octave.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 97.253)), module, Octave::CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.253)), module, Octave::CV_OUTPUT));

		OctaveDisplay *octaveDisplay = new OctaveDisplay();
		octaveDisplay->box.pos = mm2px(Vec(0.0, 14.584));
		octaveDisplay->setModule(module);
		addChild(octaveDisplay);
	}
};

RACK_PLUGIN_MODEL_INIT(Fundamental, Octave) {
   Model *modelOctave = createModel<Octave, OctaveWidget>("Fundamental", "Octave", "Octave", UTILITY_TAG);
   return modelOctave;
}
