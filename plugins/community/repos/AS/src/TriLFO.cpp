//**************************************************************************************
//TriLFO module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Code adapted from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//**************************************************************************************
#include "AS.hpp"
#include "dsp/digital.hpp"

struct LowFrequencyOscillator {
	float phase = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;
	bool offset = false;
	bool invert = false;
	SchmittTrigger resetTrigger;
	LowFrequencyOscillator() {
		//resetTrigger.setThresholds(0.0f, 0.01f);
	}
	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0f);
		freq = powf(2.0f, pitch);
	}
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01f;
		pw = clamp(pw_, pwMin, 1.0f - pwMin);
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset)) {
			phase = 0.0f;
		}
	}
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}
	float sin() {
		if (offset)
			return 1.0f - cosf(2*M_PI * phase) * (invert ? -1.0f : 1.0f);
		else
			return sinf(2.0f*M_PI * phase) * (invert ? -1.0f : 1.0f);
	}
	float tri(float x) {
		return 4.0f * fabsf(x - roundf(x));
	}
	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5f : phase);
		else
			return -1.0f + tri(invert ? phase - 0.25f : phase - 0.75f);
	}
	float saw(float x) {
		return 2.0f * (x - roundf(x));
	}
	float saw() {
		if (offset)
			return invert ? 2.0f * (1.0f - phase) : 2.0f * phase;
		else
			return saw(phase) * (invert ? -1.0f : 1.0f);
	}
	float sqr() {
		float sqr = (phase < pw) ^ invert ? 1.0f : -1.0f;
		return offset ? sqr + 1.0f : sqr;
	}
	float light() {
		return sinf(2.0f*M_PI * phase);
	}
};


struct TriLFO : Module {
	enum ParamIds {
		OFFSET1_PARAM,
		INVERT1_PARAM,
		FREQ1_PARAM,
		OFFSET2_PARAM,
		INVERT2_PARAM,
		FREQ2_PARAM,
		OFFSET3_PARAM,
		INVERT3_PARAM,
		FREQ3_PARAM,
		//
		FM1_PARAM,
		FM2_PARAM,
		PW_PARAM,
		PWM_PARAM,
		//
		NUM_PARAMS
	};
	enum InputIds {
		FM1_INPUT,
		FM2_INPUT,
		RESET1_INPUT,
		RESET2_INPUT,
		RESET3_INPUT,		
		PW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIN1_OUTPUT,
		TRI1_OUTPUT,
		SAW1_OUTPUT,
		SQR1_OUTPUT,
		SIN2_OUTPUT,
		TRI2_OUTPUT,
		SAW2_OUTPUT,
		SQR2_OUTPUT,
		SIN3_OUTPUT,
		TRI3_OUTPUT,
		SAW3_OUTPUT,
		SQR3_OUTPUT,		
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE1_POS_LIGHT,
		PHASE1_NEG_LIGHT,
		PHASE2_POS_LIGHT,
		PHASE2_NEG_LIGHT,
		PHASE3_POS_LIGHT,
		PHASE3_NEG_LIGHT,
		NUM_LIGHTS
	};

	LowFrequencyOscillator oscillator1;
	LowFrequencyOscillator oscillator2;
	LowFrequencyOscillator oscillator3;

	TriLFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	float pw_param = 0.5f;
};


void TriLFO::step() {
	//LFO1
	oscillator1.setPitch(params[FREQ1_PARAM].value + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value);
	//oscillator1.setPulseWidth(params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0);
	oscillator1.setPulseWidth(pw_param);
	oscillator1.offset = (params[OFFSET1_PARAM].value > 0.0f);
	oscillator1.invert = (params[INVERT1_PARAM].value <= 0.0f);
	oscillator1.step(1.0f / engineGetSampleRate());
	oscillator1.setReset(inputs[RESET1_INPUT].value);

	outputs[SIN1_OUTPUT].value = 5.0f * oscillator1.sin();
	outputs[TRI1_OUTPUT].value = 5.0f * oscillator1.tri();
	outputs[SAW1_OUTPUT].value = 5.0f * oscillator1.saw();
	outputs[SQR1_OUTPUT].value = 5.0f * oscillator1.sqr();

	lights[PHASE1_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0f, oscillator1.light()));
	lights[PHASE1_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -oscillator1.light()));
	//LFO2
	oscillator2.setPitch(params[FREQ2_PARAM].value + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value);
	//oscillator2.setPulseWidth(params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0);
	oscillator2.setPulseWidth(pw_param);
	oscillator2.offset = (params[OFFSET2_PARAM].value > 0.0f);
	oscillator2.invert = (params[INVERT2_PARAM].value <= 0.0f);
	oscillator2.step(1.0f / engineGetSampleRate());
	oscillator2.setReset(inputs[RESET2_INPUT].value);

	outputs[SIN2_OUTPUT].value = 5.0f * oscillator2.sin();
	outputs[TRI2_OUTPUT].value = 5.0f * oscillator2.tri();
	outputs[SAW2_OUTPUT].value = 5.0f * oscillator2.saw();
	outputs[SQR2_OUTPUT].value = 5.0f * oscillator2.sqr();

	lights[PHASE2_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0f, oscillator2.light()));
	lights[PHASE2_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -oscillator2.light()));
	//LFO3
	oscillator3.setPitch(params[FREQ3_PARAM].value + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value);
	//oscillator3.setPulseWidth(params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0);
	oscillator3.setPulseWidth(pw_param);
	oscillator3.offset = (params[OFFSET3_PARAM].value > 0.0f);
	oscillator3.invert = (params[INVERT3_PARAM].value <= 0.0f);
	oscillator3.step(1.0f / engineGetSampleRate());
	oscillator3.setReset(inputs[RESET3_INPUT].value);

	outputs[SIN3_OUTPUT].value = 5.0f * oscillator3.sin();
	outputs[TRI3_OUTPUT].value = 5.0f * oscillator3.tri();
	outputs[SAW3_OUTPUT].value = 5.0f * oscillator3.saw();
	outputs[SQR3_OUTPUT].value = 5.0f * oscillator3.sqr();

	lights[PHASE3_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0f, oscillator3.light()));
	lights[PHASE3_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -oscillator3.light()));

}


struct TriLFOWidget : ModuleWidget 
{ 
    TriLFOWidget(TriLFO *module);
};


TriLFOWidget::TriLFOWidget(TriLFO *module) : ModuleWidget(module) {

   setPanel(SVG::load(assetPlugin(plugin, "res/as_LFO.svg")));
   
 	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//LFO 1
	addInput(Port::create<as_PJ301MPort>(Vec(10, 60), Port::INPUT, module, TriLFO::RESET1_INPUT));
	addParam(ParamWidget::create<as_KnobBlack>(Vec(41, 55), module, TriLFO::FREQ1_PARAM, -8.0f, 6.0f, -1.0f));
	//
    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(37, 52), module, TriLFO::PHASE1_POS_LIGHT));
	//
    addParam(ParamWidget::create<as_CKSS>(Vec(90, 60), module, TriLFO::OFFSET1_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<as_CKSS>(Vec(120, 60), module, TriLFO::INVERT1_PARAM, 0.0f, 1.0f, 1.0f));
	//
	addOutput(Port::create<as_PJ301MPort>(Vec(11, 120), Port::OUTPUT, module, TriLFO::SIN1_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(45, 120), Port::OUTPUT, module, TriLFO::TRI1_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(80, 120), Port::OUTPUT, module, TriLFO::SAW1_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(114, 120), Port::OUTPUT, module, TriLFO::SQR1_OUTPUT));
	//LFO 2
	static const int lfo2_y_offset = 100;
	addInput(Port::create<as_PJ301MPort>(Vec(10, 60+lfo2_y_offset), Port::INPUT, module, TriLFO::RESET2_INPUT));
	addParam(ParamWidget::create<as_KnobBlack>(Vec(41, 55+lfo2_y_offset), module, TriLFO::FREQ2_PARAM, -8.0f, 6.0f, -1.0f));
	//
    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(37, 52+lfo2_y_offset), module, TriLFO::PHASE2_POS_LIGHT));
	//
    addParam(ParamWidget::create<as_CKSS>(Vec(90, 60+lfo2_y_offset), module, TriLFO::OFFSET2_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<as_CKSS>(Vec(120, 60+lfo2_y_offset), module, TriLFO::INVERT2_PARAM, 0.0f, 1.0f, 1.0f));
	//
	addOutput(Port::create<as_PJ301MPort>(Vec(11, 120+lfo2_y_offset), Port::OUTPUT, module, TriLFO::SIN2_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(45, 120+lfo2_y_offset), Port::OUTPUT, module, TriLFO::TRI2_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(80, 120+lfo2_y_offset), Port::OUTPUT, module, TriLFO::SAW2_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(114, 120+lfo2_y_offset), Port::OUTPUT, module, TriLFO::SQR2_OUTPUT));
	//LFO 3
	static const int lfo3_y_offset = 200;
	addInput(Port::create<as_PJ301MPort>(Vec(10, 60+lfo3_y_offset), Port::INPUT, module, TriLFO::RESET3_INPUT));
	addParam(ParamWidget::create<as_KnobBlack>(Vec(41, 55+lfo3_y_offset), module, TriLFO::FREQ3_PARAM, -8.0f, 6.0f, -1.0f));
	//
    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(37, 52+lfo3_y_offset), module, TriLFO::PHASE3_POS_LIGHT));
	//
    addParam(ParamWidget::create<as_CKSS>(Vec(90, 60+lfo3_y_offset), module, TriLFO::OFFSET3_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<as_CKSS>(Vec(120, 60+lfo3_y_offset), module, TriLFO::INVERT3_PARAM, 0.0f, 1.0f, 1.0f));
	//
	addOutput(Port::create<as_PJ301MPort>(Vec(11, 120+lfo3_y_offset), Port::OUTPUT, module, TriLFO::SIN3_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(45, 120+lfo3_y_offset), Port::OUTPUT, module, TriLFO::TRI3_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(80, 120+lfo3_y_offset), Port::OUTPUT, module, TriLFO::SAW3_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(114, 120+lfo3_y_offset), Port::OUTPUT, module, TriLFO::SQR3_OUTPUT));

}

RACK_PLUGIN_MODEL_INIT(AS, TriLFO) {
   Model *modelTriLFO = Model::create<TriLFO, TriLFOWidget>("AS", "TriLFO", "Tri LFO", LFO_TAG);
   return modelTriLFO;
}
