#include <string.h>
#include "FrozenWasteland.hpp"
#include "dsp/digital.hpp"

#define BUFFER_SIZE 512

namespace rack_plugin_FrozenWasteland {

struct LissajousLFO : Module {
	enum ParamIds {
		AMPLITUDE1_PARAM,
		AMPLITUDE2_PARAM,
		FREQX1_PARAM,
		FREQY1_PARAM,
		FREQX2_PARAM,
		FREQY2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AMPLITUDE1_INPUT,
		AMPLITUDE2_INPUT,
		FREQX1_INPUT,
		FREQY1_INPUT,
		FREQX2_INPUT,
		FREQY2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT_1,
		BLINK_LIGHT_2,
		BLINK_LIGHT_3,
		BLINK_LIGHT_4,
		NUM_LIGHTS
	};

struct LowFrequencyOscillator {
	float phase = 0.0;
	float freq = 1.0;
	bool offset = false;
	bool invert = false;
	//SchmittTrigger resetTrigger;
	//LowFrequencyOscillator() {
	//}
	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0);
		freq = powf(2.0, pitch);
	}
	//void setReset(float reset) {
	//	if (resetTrigger.process(reset)) {
	//		phase = 0.0;
	//	}
	//}

	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5);
		phase += deltaPhase;
		if (phase >= 1.0)
			phase -= 1.0;
	}
	float sin() {
		if (offset)
			return 1.0 - cosf(2*M_PI * phase) * (invert ? -1.0 : 1.0);
		else
			return sinf(2*M_PI * phase) * (invert ? -1.0 : 1.0);
	}

	float light() {
		return sinf(2*M_PI * phase);
	}
};


	float phase = 0.0;

	LowFrequencyOscillator oscillatorX1,oscillatorY1,oscillatorX2,oscillatorY2;

	float bufferX1[BUFFER_SIZE] = {};
	float bufferY1[BUFFER_SIZE] = {};
	float bufferX2[BUFFER_SIZE] = {};
	float bufferY2[BUFFER_SIZE] = {};
	int bufferIndex = 0;
	float frameIndex = 0;
	float deltaTime = powf(2.0, -8);

	//SchmittTrigger resetTrigger;


	float x1 = 0.0;
	float y1 = 0.0;
	float x2 = 0.0;
	float y2 = 0.0;

	LissajousLFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void LissajousLFO::step() {

	float amplitude1 = clamp(params[AMPLITUDE1_PARAM].value + (inputs[AMPLITUDE1_INPUT].value / 2.0f),0.0f,5.0f);
	float amplitude2 = clamp(params[AMPLITUDE2_PARAM].value + (inputs[AMPLITUDE2_INPUT].value / 2.0f),0.0f,5.0f);

	// Implement 4 simple sine oscillators
	oscillatorX1.setPitch(params[FREQX1_PARAM].value + inputs[FREQX1_INPUT].value);
	oscillatorY1.setPitch(params[FREQY1_PARAM].value + inputs[FREQY1_INPUT].value);
	oscillatorX2.setPitch(params[FREQX2_PARAM].value + inputs[FREQX2_INPUT].value);
	oscillatorY2.setPitch(params[FREQY2_PARAM].value + inputs[FREQY2_INPUT].value);
	oscillatorX1.step(1.0 / engineGetSampleRate());
	oscillatorY1.step(1.0 / engineGetSampleRate());
	oscillatorX2.step(1.0 / engineGetSampleRate());
	oscillatorY2.step(1.0 / engineGetSampleRate());

	//Amplitude isn't doing anything at the moment
	float x1 = amplitude1 * oscillatorX1.sin();
	float y1 = amplitude1 * oscillatorY1.sin();
	float x2 = amplitude2 * oscillatorX2.sin();
	float y2 = amplitude2 * oscillatorY2.sin();

	outputs[OUTPUT_1].value = (x1 + x2) / 2;
	outputs[OUTPUT_2].value = (y1 + y2) / 2;
	outputs[OUTPUT_3].value = (x1 + x2 + y1 + y2) / 4;
	float out4 = (x1/x2);
	outputs[OUTPUT_4].value = std::isfinite(out4) ? clamp(out4,-5.0f,5.0f) : 0.f;
	float out5 = (y1/y2);
	outputs[OUTPUT_5].value = std::isfinite(out5) ? clamp(out5,-5.0f,5.0f) : 0.f;

	//Update scope.
	int frameCount = (int)ceilf(deltaTime * engineGetSampleRate());

	// Add frame to buffers
	if (bufferIndex < BUFFER_SIZE) {
		if (++frameIndex > frameCount) {
			frameIndex = 0;
			bufferX1[bufferIndex] = x1;
			bufferY1[bufferIndex] = y1;
			bufferX2[bufferIndex] = x2;
			bufferY2[bufferIndex] = y2;
			bufferIndex++;
		}
	}

	// Are we waiting on the next trigger?
	if (bufferIndex >= BUFFER_SIZE) {
		bufferIndex = 0;
		frameIndex = 0;
	}
}





struct ScopeDisplay : TransparentWidget {
	LissajousLFO *module;
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

	ScopeDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/Sudo.ttf"));
	}

	void drawWaveform(NVGcontext *vg, float *valuesX, float *valuesY) {
		if (!valuesX)
			return;
		nvgSave(vg);
		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15*2)));
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
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

	void drawTrig(NVGcontext *vg, float value) {
		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15*2)));
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		value = value / 2.0 + 0.5;
		Vec p = Vec(box.size.x, b.pos.y + b.size.y * (1.0 - value));

		// Draw line
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, p.x - 13, p.y);
			nvgLineTo(vg, 0, p.y);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		// Draw indicator
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x60));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, p.x - 2, p.y - 4);
			nvgLineTo(vg, p.x - 9, p.y - 4);
			nvgLineTo(vg, p.x - 13, p.y);
			nvgLineTo(vg, p.x - 9, p.y + 4);
			nvgLineTo(vg, p.x - 2, p.y + 4);
			nvgClosePath(vg);
		}
		nvgFill(vg);

		nvgFontSize(vg, 9);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(0x1e, 0x28, 0x2b, 0xff));
		nvgText(vg, p.x - 8, p.y + 3, "T", NULL);
		nvgResetScissor(vg);
	}

	void drawStats(NVGcontext *vg, Vec pos, const char *title, Stats *stats) {
		nvgFontSize(vg, 13);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
		nvgText(vg, pos.x + 6, pos.y + 11, title, NULL);

		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
		char text[128];
		snprintf(text, sizeof(text), "pp % 06.2f  max % 06.2f  min % 06.2f", stats->vpp, stats->vmax, stats->vmin);
		nvgText(vg, pos.x + 22, pos.y + 11, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		float gainX = powf(2.0, 1);
		float gainY = powf(2.0, 1);
		//float offsetX = module->x1;
		//float offsetY = module->y1;

		float valuesX[BUFFER_SIZE];
		float valuesY[BUFFER_SIZE];
		for (int i = 0; i < BUFFER_SIZE; i++) {
			int j = i;
			// Lock display to buffer if buffer update deltaTime <= 2^-11
			j = (i + module->bufferIndex) % BUFFER_SIZE;
			valuesX[i] = (module->bufferX1[j]) * gainX / 10.0;
			valuesY[i] = (module->bufferY1[j]) * gainY / 10.0;
		}

		// Draw waveforms for LFO 1
		// X x Y
		nvgStrokeColor(vg, nvgRGBA(0x9f, 0xe4, 0x36, 0xc0));
		drawWaveform(vg, valuesX, valuesY);


		for (int i = 0; i < BUFFER_SIZE; i++) {
			int j = i;
			// Lock display to buffer if buffer update deltaTime <= 2^-11
			j = (i + module->bufferIndex) % BUFFER_SIZE;
			valuesX[i] = (module->bufferX2[j]) * gainX / 10.0;
			valuesY[i] = (module->bufferY2[j]) * gainY / 10.0;
		}

		// Draw waveforms for LFO 2
		// X x Y
		nvgStrokeColor(vg, nvgRGBA(0x3f, 0xe4, 0x96, 0xc0));
		drawWaveform(vg, valuesX, valuesY);

		// Calculate and draw stats
		//if (++frame >= 4) {
		//	frame = 0;
		//	statsX.calculate(module->bufferX);
		//	statsY.calculate(module->bufferY);
		//}
		//drawStats(vg, Vec(0, 0), "X", &statsX);
		//drawStats(vg, Vec(0, box.size.y - 15), "Y", &statsY);
	}
};

struct LissajousLFOWidget : ModuleWidget {
	LissajousLFOWidget(LissajousLFO *module);
};

LissajousLFOWidget::LissajousLFOWidget(LissajousLFO *module) : ModuleWidget(module) {
	box.size = Vec(15*13, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/LissajousLFO.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	{
		ScopeDisplay *display = new ScopeDisplay();
		display->module = module;
		display->box.pos = Vec(0, 35);
		display->box.size = Vec(box.size.x, 140);
		addChild(display);
	}

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(37, 186), module, LissajousLFO::AMPLITUDE1_PARAM, 0.0, 5.0, 2.5));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(87, 186), module, LissajousLFO::FREQX1_PARAM, -8.0, 3.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(137, 186), module, LissajousLFO::FREQY1_PARAM, -8.0, 3.0, 2.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(37, 265), module, LissajousLFO::AMPLITUDE2_PARAM, 0.0, 5.0, 2.5));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(87, 265), module, LissajousLFO::FREQX2_PARAM, -8.0, 3.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(137, 265), module, LissajousLFO::FREQY2_PARAM, -8.0, 3.0, 1.0));

	addInput(Port::create<PJ301MPort>(Vec(41, 219), Port::INPUT, module, LissajousLFO::AMPLITUDE1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(91, 219), Port::INPUT, module, LissajousLFO::FREQX1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(141, 219), Port::INPUT, module, LissajousLFO::FREQY1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(41, 298), Port::INPUT, module, LissajousLFO::AMPLITUDE2_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(91, 298), Port::INPUT, module, LissajousLFO::FREQX2_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(141, 298), Port::INPUT, module, LissajousLFO::FREQY2_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(22, 338), Port::OUTPUT, module, LissajousLFO::OUTPUT_1));
	addOutput(Port::create<PJ301MPort>(Vec(53, 338), Port::OUTPUT, module, LissajousLFO::OUTPUT_2));
	addOutput(Port::create<PJ301MPort>(Vec(86, 338), Port::OUTPUT, module, LissajousLFO::OUTPUT_3));
	addOutput(Port::create<PJ301MPort>(Vec(126, 338), Port::OUTPUT, module, LissajousLFO::OUTPUT_4));
	addOutput(Port::create<PJ301MPort>(Vec(158, 338), Port::OUTPUT, module, LissajousLFO::OUTPUT_5));

	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(21, 59), module, LissajousLFO::BLINK_LIGHT_1));
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(41, 59), module, LissajousLFO::BLINK_LIGHT_2));
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(61, 59), module, LissajousLFO::BLINK_LIGHT_3));
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(81, 59), module, LissajousLFO::BLINK_LIGHT_4));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, LissajousLFO) {
   Model *modelLissajousLFO = Model::create<LissajousLFO, LissajousLFOWidget>("Frozen Wasteland", "LissajousLFO", "Lissajous LFO", LFO_TAG);
   return modelLissajousLFO;
}
