#include <string.h>
#include "FrozenWasteland.hpp"
#include "dsp/digital.hpp"

#define BUFFER_SIZE 512

namespace rack_plugin_FrozenWasteland {

struct RouletteLFO : Module {
	enum ParamIds {
		FIXED_RADIUS_PARAM,
		ROTATING_RADIUS_PARAM,
		DISTANCE_PARAM,
		FREQUENCY_PARAM,
		EPI_HYPO_PARAM,
		FIXED_D_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FIXED_RADIUS_INPUT,
		ROATATING_RADIUS_INPUT,
		DISTANCE_INPUT,
		FREQUENCY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_X,
		OUTPUT_Y,
		NUM_OUTPUTS
	};
	enum RouletteTypes {
		HYPOTROCHOID_ROULETTE,
		EPITROCHIID_ROULETTE
	};
	

	float bufferX1[BUFFER_SIZE] = {};
	float bufferY1[BUFFER_SIZE] = {};
	int bufferIndex = 0;
	float frameIndex = 0;	
	float scopeDeltaTime = powf(2.0, -8);

	//SchmittTrigger resetTrigger;


	float x1 = 0.0;
	float y1 = 0.0;
	float phase = 0.0;

	RouletteLFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void RouletteLFO::step() {

	float pitch = fminf(params[FREQUENCY_PARAM].value + inputs[FREQUENCY_INPUT].value, 8.0);
	float freq = powf(2.0, pitch);
	float deltaTime = 1.0 / engineGetSampleRate();
	float deltaPhase = fminf(freq * deltaTime, 0.5);
	phase += deltaPhase;
	if (phase >= 1.0)
		phase -= 1.0;

	if(params[EPI_HYPO_PARAM].value == HYPOTROCHOID_ROULETTE) {
		float r = clamp(params[ROTATING_RADIUS_PARAM].value + inputs[ROATATING_RADIUS_INPUT].value,1.0,10.0);
		float R = clamp(params[FIXED_RADIUS_PARAM].value +inputs[FIXED_RADIUS_INPUT].value,r,20.0);
		float d = clamp(params[DISTANCE_PARAM].value + inputs[DISTANCE_INPUT].value,1.0,10.0);
		if(params[FIXED_D_PARAM].value) {
			d=r;
		}

		float amplitudeScaling = 5.0 / (R-r+d);

		float theta = phase * 2 * M_PI;
		x1 = amplitudeScaling * (((R-r) * cosf(theta)) + (d * cosf((R-r)/r * theta)));
		y1 = amplitudeScaling * (((R-r) * sinf(theta)) - (d * sinf((R-r)/r * theta)));
	} else {
		float R = clamp(params[FIXED_RADIUS_PARAM].value +inputs[FIXED_RADIUS_INPUT].value,1.0,20.0);
		float r = clamp(params[ROTATING_RADIUS_PARAM].value + inputs[ROATATING_RADIUS_INPUT].value,1.0,10.0);
		float d = clamp(params[DISTANCE_PARAM].value + inputs[DISTANCE_INPUT].value,1.0,20.0);
		if(params[FIXED_D_PARAM].value) {
			d=r;
		}

		float amplitudeScaling = 5.0 / (R+r+d);

		float theta = phase * 2 * M_PI;
		x1 = amplitudeScaling * (((R+r) * cosf(theta)) - (d * cosf((R+r)/r * theta)));
		y1 = amplitudeScaling * (((R+r) * sinf(theta)) - (d * sinf((R+r)/r * theta)));

	}
	outputs[OUTPUT_X].value = x1;
	outputs[OUTPUT_Y].value = y1;
	

	//Update scope.
	int frameCount = (int)ceilf(scopeDeltaTime * engineGetSampleRate());

	// Add frame to buffers
	if (bufferIndex < BUFFER_SIZE) {
		if (++frameIndex > frameCount) {
			frameIndex = 0;
			bufferX1[bufferIndex] = x1;
			bufferY1[bufferIndex] = y1;
			bufferIndex++;
		}
	}

	// Are we waiting on the next trigger?
	if (bufferIndex >= BUFFER_SIZE) {
		bufferIndex = 0;
		frameIndex = 0;
	}
}





struct RouletteScopeDisplay : TransparentWidget {
	RouletteLFO *module;
	int frame = 0;
	std::shared_ptr<Font> font;


	RouletteScopeDisplay() {
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

	


	void draw(NVGcontext *vg) override {
		//float gainX = powf(2.0, 1);
		//float gainY = powf(2.0, 1);
		//float offsetX = module->x1;
		//float offsetY = module->y1;

		float valuesX[BUFFER_SIZE];
		float valuesY[BUFFER_SIZE];
		for (int i = 0; i < BUFFER_SIZE; i++) {
			int j = i;
			// Lock display to buffer if buffer update deltaTime <= 2^-11
			j = (i + module->bufferIndex) % BUFFER_SIZE;
			valuesX[i] = (module->bufferX1[j]) / 5.0;
			valuesY[i] = (module->bufferY1[j]) / 5.0;
		}

		// Draw waveforms for LFO 1
		// X x Y
		nvgStrokeColor(vg, nvgRGBA(0x9f, 0xe4, 0x36, 0xc0));
		drawWaveform(vg, valuesX, valuesY);


	}
};

struct RouletteLFOWidget : ModuleWidget {
	RouletteLFOWidget(RouletteLFO *module);
};

RouletteLFOWidget::RouletteLFOWidget(RouletteLFO *module) : ModuleWidget(module) {
	box.size = Vec(15*13, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/RouletteLFO.svg"))); 
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	{
		RouletteScopeDisplay *display = new RouletteScopeDisplay();
		display->module = module;
		display->box.pos = Vec(0, 35);
		display->box.size = Vec(box.size.x, 140);
		addChild(display);
	}

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 186), module, RouletteLFO::FIXED_RADIUS_PARAM, 1, 20.0, 5));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(60, 186), module, RouletteLFO::ROTATING_RADIUS_PARAM, 1, 10.0, 3));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(113, 186), module, RouletteLFO::DISTANCE_PARAM, 1, 10.0, 5.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(160, 186), module, RouletteLFO::FREQUENCY_PARAM, -8.0, 4.0, 0.0));
	addParam(ParamWidget::create<CKSS>(Vec(55, 265), module, RouletteLFO::EPI_HYPO_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<CKSS>(Vec(130, 265), module, RouletteLFO::FIXED_D_PARAM, 0.0, 1.0, 0.0));
	//addParam(ParamWidget::create<RoundBlackKnob>(Vec(87, 265), module, RouletteLFO::FREQX2_PARAM, -8.0, 3.0, 0.0));
	//addParam(ParamWidget::create<RoundBlackKnob>(Vec(137, 265), module, RouletteLFO::FREQY2_PARAM, -8.0, 3.0, 1.0));

	addInput(Port::create<PJ301MPort>(Vec(13, 219), Port::INPUT, module, RouletteLFO::FIXED_RADIUS_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(63, 219), Port::INPUT, module, RouletteLFO::ROATATING_RADIUS_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(116, 219), Port::INPUT, module, RouletteLFO::DISTANCE_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(163, 219), Port::INPUT, module, RouletteLFO::FREQUENCY_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(57, 335), Port::OUTPUT, module, RouletteLFO::OUTPUT_X));
	addOutput(Port::create<PJ301MPort>(Vec(113, 335), Port::OUTPUT, module, RouletteLFO::OUTPUT_Y));

	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(21, 59), module, LissajousLFO::BLINK_LIGHT_1));
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(41, 59), module, LissajousLFO::BLINK_LIGHT_2));
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(61, 59), module, LissajousLFO::BLINK_LIGHT_3));
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(81, 59), module, LissajousLFO::BLINK_LIGHT_4));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, RouletteLFO) {
   Model *modelRouletteLFO = Model::create<RouletteLFO, RouletteLFOWidget>("Frozen Wasteland", "RouletteLFO", "Roulette LFO", LFO_TAG);
   return modelRouletteLFO;
}
