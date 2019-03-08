#include <string.h>
#include "JWModules.hpp"
#include "JWResizableHandle.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_JW_Modules {

#define BUFFER_SIZE 512

struct FullScope : Module {
	enum ParamIds {
		X_SCALE_PARAM,
		X_POS_PARAM,
		Y_SCALE_PARAM,
		Y_POS_PARAM,
		TIME_PARAM,
		LISSAJOUS_PARAM,
		TRIG_PARAM,
		EXTERNAL_PARAM,
		ROTATION_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,
		Y_INPUT,
		TRIG_INPUT,
		COLOR_INPUT,
		TIME_INPUT,
		ROTATION_INPUT,
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
	bool lissajous = true;
	bool external = false;
	float lights[4] = {};
	SchmittTrigger resetTrigger;

	FullScope() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "lissajous", json_integer((int) lissajous));
		json_object_set_new(rootJ, "external", json_integer((int) external));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "lissajous");
		if (sumJ)
			lissajous = json_integer_value(sumJ);

		json_t *extJ = json_object_get(rootJ, "external");
		if (extJ)
			external = json_integer_value(extJ);
	}

	void reset() override {
		lissajous = true;
		external = false;
	}
};

void FullScope::step() {
	lights[0] = lissajous ? 0.0 : 1.0;
	lights[1] = lissajous ? 1.0 : 0.0;

	if (extTrigger.process(params[EXTERNAL_PARAM].value)) {
		external = !external;
	}
	lights[2] = external ? 0.0 : 1.0;
	lights[3] = external ? 1.0 : 0.0;

	// Compute time
	float deltaTime = powf(2.0, params[TIME_PARAM].value + inputs[TIME_INPUT].value);
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
		if (lissajous || (external && !inputs[TRIG_INPUT].active)) {
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
		float gate = external ? inputs[TRIG_INPUT].value : inputs[X_INPUT].value;

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

struct FullScopeDisplay : TransparentWidget {
	FullScope *module;
	int frame = 0;
	float rot = 0;
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

	FullScopeDisplay() {
	}

	void drawWaveform(NVGcontext *vg, float *valuesX, float *valuesY) {
		if (!valuesX)
			return;
		nvgSave(vg);
		Rect b = Rect(Vec(0, 0), box.size);
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		
		float rotRate = rescalefjw(module->params[FullScope::ROTATION_PARAM].value + module->inputs[FullScope::ROTATION_INPUT].value, 0, 10, 0, 0.5);
		if(rotRate != 0){
			nvgTranslate(vg, box.size.x/2.0, box.size.y/2.0);
			nvgRotate(vg, rot+=rotRate);
			nvgTranslate(vg, -box.size.x/2.0, -box.size.y/2.0);
		} else {
			nvgRotate(vg, 0);
		}

		nvgBeginPath(vg);
		// Draw maximum display left to right
		for (int i = 0; i < BUFFER_SIZE; i++) {
			float x, y;
			if (valuesY) {
				x = valuesX[i] / 2.0 + 0.5;
				y = valuesY[i] / 2.0 + 0.5;
			}
			else {
				x = (float)i / (BUFFER_SIZE - 1);
				y = valuesX[i] / 2.0 + 0.5;
			}
			Vec p;
			p.x = b.pos.x + b.size.x * x;
			p.y = b.pos.y + b.size.y * (1.0 - y);
			if (i == 0)
				nvgMoveTo(vg, p.x, p.y);
			else
				nvgLineTo(vg, p.x, p.y);
		}
		nvgLineCap(vg, NVG_ROUND);
		nvgMiterLimit(vg, 2.0);
		nvgStrokeWidth(vg, 1.5);
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgStroke(vg);
		nvgResetScissor(vg);
		nvgRestore(vg);
	}

	void draw(NVGcontext *vg) {
		float gainX = powf(2.0, roundf(module->params[FullScope::X_SCALE_PARAM].value));
		float gainY = powf(2.0, roundf(module->params[FullScope::Y_SCALE_PARAM].value));
		float offsetX = module->params[FullScope::X_POS_PARAM].value;
		float offsetY = module->params[FullScope::Y_POS_PARAM].value;

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

		//color
		if(module->inputs[FullScope::COLOR_INPUT].active){
			float hue = rescalefjw(module->inputs[FullScope::COLOR_INPUT].value, 0.0, 6.0, 0, 1.0);
			nvgStrokeColor(vg, nvgHSLA(hue, 0.5, 0.5, 0xc0));
		} else {
			nvgStrokeColor(vg, nvgRGBA(25, 150, 252, 0xc0));
		}

		// Draw waveforms
		if (module->lissajous) {
			// X x Y
			if (module->inputs[FullScope::X_INPUT].active || module->inputs[FullScope::Y_INPUT].active) {
				drawWaveform(vg, valuesX, valuesY);
			}
		}
		else {
			// Y
			if (module->inputs[FullScope::Y_INPUT].active) {
				drawWaveform(vg, valuesY, NULL);
			}

			// X
			if (module->inputs[FullScope::X_INPUT].active) {
				nvgStrokeColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xc0));
				drawWaveform(vg, valuesX, NULL);
			}
		}

		// Calculate stats
		if (++frame >= 4) {
			frame = 0;
			statsX.calculate(module->bufferX);
			statsY.calculate(module->bufferY);
		}
	}
};

struct FullScopeWidget : ModuleWidget {
	Panel *panel;
	JWModuleResizeHandle *leftHandle;
	JWModuleResizeHandle *rightHandle;
	TransparentWidget *display;
	FullScopeWidget(FullScope *module);
	void step() override;
	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
	Menu *createContextMenu() override;
};

FullScopeWidget::FullScopeWidget(FullScope *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*17, RACK_GRID_HEIGHT);

	{
		panel = new Panel();
		panel->backgroundColor = nvgRGB(20, 30, 33);
		panel->box.size = box.size;
		addChild(panel);
	}

	leftHandle = new JWModuleResizeHandle(box.size.x);
	rightHandle = new JWModuleResizeHandle(box.size.x);
	rightHandle->right = true;
	addChild(leftHandle);
	addChild(rightHandle);

	{
		FullScopeDisplay *display = new FullScopeDisplay();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, box.size.y);
		addChild(display);
		this->display = display;
	}

	int compX = 5, compY = -15, adder = 20;
	addInput(Port::create<TinyPJ301MPort>(Vec(compX, compY+=adder), Port::INPUT, module, FullScope::X_INPUT));
	addInput(Port::create<TinyPJ301MPort>(Vec(compX, compY+=adder), Port::INPUT, module, FullScope::Y_INPUT));
	addInput(Port::create<TinyPJ301MPort>(Vec(compX, compY+=adder), Port::INPUT, module, FullScope::COLOR_INPUT));
	addInput(Port::create<TinyPJ301MPort>(Vec(compX, compY+=adder), Port::INPUT, module, FullScope::ROTATION_INPUT));
	addInput(Port::create<TinyPJ301MPort>(Vec(compX, compY+=adder), Port::INPUT, module, FullScope::TIME_INPUT));

	addParam(ParamWidget::create<JwTinyKnob>(Vec(compX, compY+=adder), module, FullScope::X_POS_PARAM, -10.0, 10.0, 0.0));
	addParam(ParamWidget::create<JwTinyKnob>(Vec(compX, compY+=adder), module, FullScope::Y_POS_PARAM, -10.0, 10.0, 0.0));
	addParam(ParamWidget::create<JwTinyKnob>(Vec(compX, compY+=adder), module, FullScope::X_SCALE_PARAM, -2.0, 8.0, 1.0));
	addParam(ParamWidget::create<JwTinyKnob>(Vec(compX, compY+=adder), module, FullScope::Y_SCALE_PARAM, -2.0, 8.0, 1.0));
	addParam(ParamWidget::create<JwTinyKnob>(Vec(compX, compY+=adder), module, FullScope::ROTATION_PARAM, -10.0, 10.0, 0));
	addParam(ParamWidget::create<JwTinyKnob>(Vec(compX, compY+=adder), module, FullScope::TIME_PARAM, -6.0, -16.0, -14.0));

	addChild(Widget::create<Screw_J>(Vec(compX+2, compY+=adder)));
	addChild(Widget::create<Screw_W>(Vec(compX+2, compY+=adder-5)));
}

void FullScopeWidget::step() {
	panel->box.size = box.size;
	display->box.size = Vec(box.size.x, box.size.y);
	rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
	rightHandle->box.pos.y = box.size.y - rightHandle->box.size.y;
	leftHandle->box.pos.y = box.size.y - leftHandle->box.size.y;
	ModuleWidget::step();
}

json_t *FullScopeWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();
	json_object_set_new(rootJ, "width", json_real(box.size.x));
	json_object_set_new(rootJ, "height", json_real(box.size.y));
	return rootJ;
}

void FullScopeWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);
	json_t *widthJ = json_object_get(rootJ, "width");
	if (widthJ)
		box.size.x = json_number_value(widthJ);
	json_t *heightJ = json_object_get(rootJ, "height");
	if (heightJ)
		box.size.y = json_number_value(heightJ);
}

struct FullScopeLissajousModeMenuItem : MenuItem {
	FullScope *fullScope;
	void onAction(EventAction &e) override {
		fullScope->lissajous = !fullScope->lissajous;
	}
	void step() override {
		rightText = (fullScope->lissajous) ? "✔" : "";
	}
};

Menu *FullScopeWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	FullScope *fullScope = dynamic_cast<FullScope*>(module);
	assert(fullScope);

	FullScopeLissajousModeMenuItem *lissMenuItem = new FullScopeLissajousModeMenuItem();
	lissMenuItem->text = "Lissajous Mode";
	lissMenuItem->fullScope = fullScope;
	menu->addChild(lissMenuItem);

	return menu;
}

} // namespace rack_plugin_JW_Modules

using namespace rack_plugin_JW_Modules;

RACK_PLUGIN_MODEL_INIT(JW_Modules, FullScope) {
   Model *modelFullScope = Model::create<FullScope, FullScopeWidget>("JW-Modules", "FullScope", "Full Scope", VISUAL_TAG);
   return modelFullScope;
}
