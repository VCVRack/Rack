//#include <string.h>
#include "FrozenWasteland.hpp"
#include "dsp/digital.hpp"

#define DIVISIONS 27

namespace rack_plugin_FrozenWasteland {

struct BPMLFO2 : Module {
	enum ParamIds {
		DIVISION_PARAM,
		DIVISION_CV_ATTENUVERTER_PARAM,
		SKEW_PARAM,
		SKEW_CV_ATTENUVERTER_PARAM,
		OFFSET_PARAM,	
		WAVESHAPE_PARAM,
		HOLD_CLOCK_BEHAVIOR_PARAM,
		HOLD_MODE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		DIVISION_INPUT,
		SKEW_INPUT,
		RESET_INPUT,
		HOLD_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LFO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLOCK_LIGHT,
		HOLD_LIGHT,
		NUM_LIGHTS
	};	
	enum Waveshapes {
		SKEWSAW_WAV,
		SQUARE_WAV
	};

struct LowFrequencyOscillator {
	float phase = 0.0;
	float freq = 1.0;
	float pw = 0.5;
	float skew = 0.5; // Triangle
	bool offset = false;


	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0);
		freq = powf(2.0, pitch);
	}
	void setFrequency(float frequency) {
		freq = frequency;
	}
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01;
		pw = clamp(pw_, pwMin, 1.0f - pwMin);
	}

	void hardReset()
	{
		phase = 0.0;
	}

	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5);
		phase += deltaPhase;
		if (phase >= 1.0)
			phase -= 1.0;
	}

	float skewsaw(float x) {
		x = eucmod(x,1.0);
		float inverseSkew = 1 - skew;
		float result;
        if (skew == 0 && x == 0) //Avoid /0 error
            return 2;
        if (x <= skew)
            result = 2.0 * (1- (-1 / skew * x + 1));
        else
            result = 2.0 * (1-(1 / inverseSkew * (x - skew)));
		return result;
	}

	float skewsaw() {
		if (offset)
			return skewsaw(phase);
		else
			return skewsaw(phase) - 1; //Going to keep same phase for now
			//return skewsaw(phase - .5) - 1;
	}

	float sqr() {
		float sqr = (phase < pw) ? 1.0 : -1.0;
		return offset ? sqr + 1.0 : sqr;
	}
	
	float progress() {
		return phase;
	}
};


	LowFrequencyOscillator oscillator;
	SchmittTrigger clockTrigger,resetTrigger,holdTrigger;
	float divisions[DIVISIONS] = {1/64.0f,1/32.0f,1/16.0f,1/13.0f,1/11.0f,1/8.0f,1/7.0f,1/6.0f,1/5.0f,1/4.0f,1/3.0f,1/2.0f,1/1.5f,1,1.5f,2,3,4,5,6,7,8,11,13,16,32,64};
	const char* divisionNames[DIVISIONS] = {"/64","/32","/16","/13","/11","/8","/7","/6","/5","/4","/3","/2","/1.5","x 1","x 1.5","x 2","x 3","x 4","x 5","x 6","x 7","x 8","x 11","x 13","x 16","x 32","x 64"};
	int division;
	float time = 0.0;
	float duration = 0;
	float waveshape = 0;
	float skew = 0.5;
	bool holding = false;
	bool secondClockReceived = false;

	float lfoOutputValue = 0.0;



	BPMLFO2() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	void reset() override {
		division = 0;
	}

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void BPMLFO2::step() {

    time += 1.0 / engineGetSampleRate();
	if(inputs[CLOCK_INPUT].active) {
		if(clockTrigger.process(inputs[CLOCK_INPUT].value)) {
			if(secondClockReceived) {
				duration = time;
			}
			time = 0;
			secondClockReceived = true;			
		}
		lights[CLOCK_LIGHT].value = time > (duration/2.0);
	}
	
	

	float divisionf = params[DIVISION_PARAM].value;
	if(inputs[DIVISION_INPUT].active) {
		divisionf +=(inputs[DIVISION_INPUT].value * params[DIVISION_CV_ATTENUVERTER_PARAM].value * (DIVISIONS / 10.0));
	}
	divisionf = clamp(divisionf,0.0f,26.0f);
	division = int(divisionf);

	waveshape = params[WAVESHAPE_PARAM].value;

	skew = params[SKEW_PARAM].value;
	if(inputs[SKEW_INPUT].active) {
		skew +=inputs[SKEW_INPUT].value / 10 * params[SKEW_CV_ATTENUVERTER_PARAM].value;
	}
	skew = clamp(skew,0.0f,1.0f);
	
	oscillator.offset = (params[OFFSET_PARAM].value > 0.0);
	oscillator.skew = skew;
	oscillator.setPulseWidth(skew);

	if(duration != 0) {
		oscillator.setFrequency(1.0 / (duration / divisions[division]));
	}
	else {
		oscillator.setFrequency(0);
	}

	if(inputs[RESET_INPUT].active) {
		if(resetTrigger.process(inputs[RESET_INPUT].value)) {
			oscillator.hardReset();
		}		
	} 

	if(inputs[HOLD_INPUT].active) {
		if(params[HOLD_MODE_PARAM].value == 1.0) { //Latched is default		
			if(holdTrigger.process(inputs[HOLD_INPUT].value)) {
				holding = !holding;
			}		
		} else {
			holding = inputs[HOLD_INPUT].value >= 1; 			
		}
		lights[HOLD_LIGHT].value = holding;
	} 

    if(!holding || (holding && params[HOLD_CLOCK_BEHAVIOR_PARAM].value == 0.0)) {
    	oscillator.step(1.0 / engineGetSampleRate());
    }

	if(!holding) {
		if(waveshape == SKEWSAW_WAV)
			lfoOutputValue = 5.0 * oscillator.skewsaw();
		else
			lfoOutputValue = 5.0 * oscillator.sqr();
	}


	outputs[LFO_OUTPUT].value = lfoOutputValue;		
}

struct BPMLFO2ProgressDisplay : TransparentWidget {
	BPMLFO2 *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	

	BPMLFO2ProgressDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/01 Digit.ttf"));
	}

	void drawProgress(NVGcontext *vg, int waveshape, float skew, float phase) 
	{
		float inverseSkew = 1 - skew;
		float y;
        if (skew == 0 && phase == 0) //Avoid /0 error
            y = 72.0;
        if (phase <= skew)
            y = 72.0 * (1- (-1 / skew * phase + 1));
        else
            y = 72.0 * (1-(1 / inverseSkew * (phase - skew)));

		// Draw indicator
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0x20, 0xff));
		if(waveshape == BPMLFO2::SKEWSAW_WAV)
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg,68,213);
			if(phase > skew) {
				nvgLineTo(vg,68 + (skew * 68),141);
			}
			nvgLineTo(vg,68 + (phase * 68),213-y);
			nvgLineTo(vg,68 + (phase * 68),213);
			nvgClosePath(vg);
			nvgFill(vg);
		} else 
		{
			float endpoint = min(phase,skew);
			nvgBeginPath(vg);
			nvgMoveTo(vg,68,177);
			nvgRect(vg,68,177,endpoint*68,36.0);
			//nvgLineTo(vg,68 + (endpoint * 68),213);
			//nvgLineTo(vg,68 + (endpoint * 68),177);
			//nvgLineTo(vg,68,177);
			nvgClosePath(vg);
			nvgFill(vg);
			if(phase > skew) {
				nvgBeginPath(vg);
				nvgMoveTo(vg,68 + (skew * 68),141);
				nvgRect(vg,68 + (skew * 68),141,(phase-skew)*68,36.0);
				//nvgLineTo(vg,68 + (phase * 68),177);
				//nvgLineTo(vg,68 + (phase * 68),141);
				//nvgMoveTo(vg,68 + (skew * 68),141);
				nvgClosePath(vg);
				nvgFill(vg);
			}

		}
	}

	void drawWaveShape(NVGcontext *vg, int waveshape, float skew) 
	{
		// Draw wave shape
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0x20, 0xff));	
		nvgStrokeWidth(vg, 2.0);
		if(waveshape == BPMLFO2::SKEWSAW_WAV) {						
			nvgBeginPath(vg);
			nvgMoveTo(vg,68,213);
			nvgLineTo(vg,68 + (skew * 68),141);
			nvgLineTo(vg,136,213);
			nvgClosePath(vg);			
		} else 
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg,68,213);
			nvgLineTo(vg,68 + (skew * 68),213);
			nvgLineTo(vg,68 + (skew * 68),141);
			nvgLineTo(vg,136,141);
			//nvgClosePath(vg);			
		}
		nvgStroke(vg);		
	}

	void drawDivision(NVGcontext *vg, Vec pos, int division) {
		nvgFontSize(vg, 28);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), "%s", module->divisionNames[division]);
		nvgText(vg, pos.x + 52, pos.y, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		
		drawWaveShape(vg,module->waveshape, module->skew);
		drawProgress(vg,module->waveshape, module->skew, module->oscillator.progress());
		drawDivision(vg, Vec(0, box.size.y - 153), module->division);
	}
};

struct BPMLFO2Widget : ModuleWidget {
	BPMLFO2Widget(BPMLFO2 *module);
};

BPMLFO2Widget::BPMLFO2Widget(BPMLFO2 *module) : ModuleWidget(module) {
	box.size = Vec(15*10, RACK_GRID_HEIGHT);
	
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/BPMLFO2.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	{
		BPMLFO2ProgressDisplay *display = new BPMLFO2ProgressDisplay();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, 220);
		addChild(display);
	}

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(75, 78), module, BPMLFO2::DIVISION_PARAM, 0.0, 26.5, 13.0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(40, 109), module, BPMLFO2::DIVISION_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(14, 140), module, BPMLFO2::SKEW_PARAM, 0.0, 1.0, 0.5));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(17, 200), module, BPMLFO2::SKEW_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<CKSS>(Vec(12, 240), module, BPMLFO2::OFFSET_PARAM, 0.0, 1.0, 1.0));
	addParam(ParamWidget::create<CKSS>(Vec(42, 240), module, BPMLFO2::WAVESHAPE_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<CKSS>(Vec(82, 240), module, BPMLFO2::HOLD_CLOCK_BEHAVIOR_PARAM, 0.0, 1.0, 1.0));
	addParam(ParamWidget::create<CKSS>(Vec(125, 240), module, BPMLFO2::HOLD_MODE_PARAM, 0.0, 1.0, 1.0));

	addInput(Port::create<PJ301MPort>(Vec(40, 81), Port::INPUT, module, BPMLFO2::DIVISION_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(16, 172), Port::INPUT, module, BPMLFO2::SKEW_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(31, 290), Port::INPUT, module, BPMLFO2::CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(62, 290), Port::INPUT, module, BPMLFO2::RESET_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(94, 290), Port::INPUT, module, BPMLFO2::HOLD_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(63, 336), Port::OUTPUT, module, BPMLFO2::LFO_OUTPUT));

	addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(12, 294), module, BPMLFO2::CLOCK_LIGHT));
	addChild(ModuleLightWidget::create<LargeLight<RedLight>>(Vec(122, 294), module, BPMLFO2::HOLD_LIGHT));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, BPMLFO2) {
   Model *modelBPMLFO2 = Model::create<BPMLFO2, BPMLFO2Widget>("Frozen Wasteland", "BPMLFO2", "BPM LFO 2", LFO_TAG);
   return modelBPMLFO2;
}
