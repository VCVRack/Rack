#include <string.h>
#include "JWModules.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_JW_Modules {

#define BUFFER_SIZE 512

struct MinMax : Module {
	enum ParamIds {
		TIME_PARAM,
		TRIG_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,
		Y_INPUT,
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};

	float bufferX[BUFFER_SIZE] = {};
	float bufferY[BUFFER_SIZE] = {};
	int bufferIndex = 0;
	float frameIndex = 0;

	SchmittTrigger sumTrigger;
	SchmittTrigger extTrigger;
	bool lissajous = false;
	SchmittTrigger resetTrigger;

	MinMax() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "lissajous", json_integer((int) lissajous));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "lissajous");
		if (sumJ)
			lissajous = json_integer_value(sumJ);

	}

	void reset() override {
		lissajous = false;
	}
};


void MinMax::step() {
	// Compute time
	float deltaTime = powf(2.0, params[TIME_PARAM].value);
	int frameCount = (int)ceilf(deltaTime * engineGetSampleRate());

	// Add frame to buffer
	if (bufferIndex < BUFFER_SIZE) {
		if (++frameIndex > frameCount) {
			frameIndex = 0;
			bufferX[bufferIndex] = inputs[X_INPUT].value;
			bufferY[bufferIndex] = inputs[Y_INPUT].value;
			bufferIndex++;
		}
	}

	// Are we waiting on the next trigger?
	if (bufferIndex >= BUFFER_SIZE) {
		// Trigger immediately if external but nothing plugged in, or in Lissajous mode
		if (lissajous) {
			bufferIndex = 0;
			frameIndex = 0;
			return;
		}

		// Reset the Schmitt trigger so we don't trigger immediately if the input is high
		if (frameIndex == 0) {
			resetTrigger.reset();
		}
		frameIndex++;

		// Must go below 0.1V to trigger
		// resetTrigger.setThresholds(params[TRIG_PARAM].value - 0.1, params[TRIG_PARAM].value);
		float gate = inputs[X_INPUT].value;

		// Reset if triggered
		float holdTime = 0.1;
		if (resetTrigger.process(gate) || (frameIndex >= engineGetSampleRate() * holdTime)) {
			bufferIndex = 0; frameIndex = 0; return;
		}

		// Reset if we've waited too long
		if (frameIndex >= engineGetSampleRate() * holdTime) {
			bufferIndex = 0; frameIndex = 0; return;
		}
	}
}


struct MinMaxDisplay : TransparentWidget {
	MinMax *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	struct Stats {
		float vrms, vpp, vmin, vmax;
		void calculate(float *values) {
			vrms = 0.0;
			vmax = -INFINITY;
			vmin = INFINITY;
			for (int i = 0; i < BUFFER_SIZE; i++) {
				float v = values[i];
				vrms += v*v;
				vmax = fmaxf(vmax, v);
				vmin = fminf(vmin, v);
			}
			vrms = sqrtf(vrms / BUFFER_SIZE);
			vpp = vmax - vmin;
		}
	};
	Stats statsX, statsY;

	MinMaxDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void drawStats(NVGcontext *vg, Vec pos, const char *title, Stats *stats) {
		nvgFontSize(vg, 24);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
		char text[128];
		snprintf(text, sizeof(text), "%5.2f", stats->vmin);
		nvgText(vg, pos.x + 10, pos.y + 28, text, NULL);
		snprintf(text, sizeof(text), "%5.2f", stats->vmax);
		nvgText(vg, pos.x + 10, pos.y + 78, text, NULL);
	}

	void draw(NVGcontext *vg) {
		float gainX = 1;
		float gainY = 1;
		float offsetX = 0;
		float offsetY = 0;

		float valuesX[BUFFER_SIZE];
		float valuesY[BUFFER_SIZE];
		for (int i = 0; i < BUFFER_SIZE; i++) {
			int j = i;
			// Lock display to buffer if buffer update deltaTime <= 2^-11
			if (module->lissajous)
				j = (i + module->bufferIndex) % BUFFER_SIZE;
			valuesX[i] = (module->bufferX[j] + offsetX) * gainX / 10.0;
			valuesY[i] = (module->bufferY[j] + offsetY) * gainY / 10.0;
		}

		// Calculate and draw stats
		if (++frame >= 4) {
			frame = 0;
			statsX.calculate(module->bufferX);
			statsY.calculate(module->bufferY);
		}
		drawStats(vg, Vec(0, 20), "X", &statsX);
	}
};


struct MinMaxWidget : ModuleWidget { 
	MinMaxWidget(MinMax *module); 
};

MinMaxWidget::MinMaxWidget(MinMax *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*6, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/MinMax.svg")));
		addChild(panel);
	}

	addChild(Widget::create<Screw_J>(Vec(16, 1)));
	addChild(Widget::create<Screw_J>(Vec(16, 365)));
	addChild(Widget::create<Screw_W>(Vec(box.size.x-29, 1)));
	addChild(Widget::create<Screw_W>(Vec(box.size.x-29, 365)));

	CenteredLabel* const titleLabel = new CenteredLabel(16);
	titleLabel->box.pos = Vec(22, 15);
	titleLabel->text = "MinMax";
	addChild(titleLabel);

	{
		MinMaxDisplay *display = new MinMaxDisplay();
		display->module = module;
		display->box.pos = Vec(0, 44);
		display->box.size = Vec(box.size.x, 140);
		addChild(display);
	}

	CenteredLabel* const minLabel = new CenteredLabel(12);
	minLabel->box.pos = Vec(22, 35);
	minLabel->text = "Min";
	addChild(minLabel);

	CenteredLabel* const maxLabel = new CenteredLabel(12);
	maxLabel->box.pos = Vec(22, 60);
	maxLabel->text = "Max";
	addChild(maxLabel);

	CenteredLabel* const timeLabel = new CenteredLabel(12);
	timeLabel->box.pos = Vec(22, 101);
	timeLabel->text = "Time";
	addChild(timeLabel);

	CenteredLabel* const inLabel = new CenteredLabel(12);
	inLabel->box.pos = Vec(23, 132);
	inLabel->text = "Input";
	addChild(inLabel);

	addParam(ParamWidget::create<SmallWhiteKnob>(Vec(32, 209), module, MinMax::TIME_PARAM, -6.0, -16.0, -14.0));
	addInput(Port::create<PJ301MPort>(Vec(33, 275), Port::INPUT, module, MinMax::X_INPUT));
}

} // namespace rack_plugin_JW_Modules

using namespace rack_plugin_JW_Modules;

RACK_PLUGIN_MODEL_INIT(JW_Modules, MinMax) {
   Model *modelMinMax = Model::create<MinMax, MinMaxWidget>("JW-Modules", "MinMax", "Min Max", UTILITY_TAG);
   return modelMinMax;
}
