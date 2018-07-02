#include "Fundamental.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/filter.hpp"
#include "samplerate.h"


#define HISTORY_SIZE (1<<21)

struct Delay : Module {
	enum ParamIds {
		TIME_PARAM,
		FEEDBACK_PARAM,
		COLOR_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TIME_INPUT,
		FEEDBACK_INPUT,
		COLOR_INPUT,
		MIX_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer;
	DoubleRingBuffer<float, 16> outBuffer;
	SRC_STATE *src;
	float lastWet = 0.0f;
	RCFilter lowpassFilter;
	RCFilter highpassFilter;

	Delay() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		src = src_new(SRC_SINC_FASTEST, 1, NULL);
		assert(src);
	}

	~Delay() {
		src_delete(src);
	}

	void step() override;
};


void Delay::step() {
	// Get input to delay block
	float in = inputs[IN_INPUT].value;
	float feedback = clamp(params[FEEDBACK_PARAM].value + inputs[FEEDBACK_INPUT].value / 10.0f, 0.0f, 1.0f);
	float dry = in + lastWet * feedback;

	// Compute delay time in seconds
	float delay = 1e-3 * powf(10.0f / 1e-3, clamp(params[TIME_PARAM].value + inputs[TIME_INPUT].value / 10.0f, 0.0f, 1.0f));
	// Number of delay samples
	float index = delay * engineGetSampleRate();

	// Push dry sample into history buffer
	if (!historyBuffer.full()) {
		historyBuffer.push(dry);
	}

	// How many samples do we need consume to catch up?
	float consume = index - historyBuffer.size();

	if (outBuffer.empty()) {
		double ratio = 1.f;
		if (fabsf(consume) >= 16.f) {
			ratio = powf(10.f, clamp(consume / 10000.f, -1.f, 1.f));
		}

		SRC_DATA srcData;
		srcData.data_in = (const float*) historyBuffer.startData();
		srcData.data_out = (float*) outBuffer.endData();
		srcData.input_frames = min(historyBuffer.size(), 16);
		srcData.output_frames = outBuffer.capacity();
		srcData.end_of_input = false;
		srcData.src_ratio = ratio;
		src_process(src, &srcData);
		historyBuffer.startIncr(srcData.input_frames_used);
		outBuffer.endIncr(srcData.output_frames_gen);
	}

	float wet = 0.0f;
	if (!outBuffer.empty()) {
		wet = outBuffer.shift();
	}

	// Apply color to delay wet output
	// TODO Make it sound better
	float color = clamp(params[COLOR_PARAM].value + inputs[COLOR_INPUT].value / 10.0f, 0.0f, 1.0f);
	float lowpassFreq = 10000.0f * powf(10.0f, clamp(2.0f*color, 0.0f, 1.0f));
	lowpassFilter.setCutoff(lowpassFreq / engineGetSampleRate());
	lowpassFilter.process(wet);
	wet = lowpassFilter.lowpass();
	float highpassFreq = 10.0f * powf(100.0f, clamp(2.0f*color - 1.0f, 0.0f, 1.0f));
	highpassFilter.setCutoff(highpassFreq / engineGetSampleRate());
	highpassFilter.process(wet);
	wet = highpassFilter.highpass();

	lastWet = wet;

	float mix = clamp(params[MIX_PARAM].value + inputs[MIX_INPUT].value / 10.0f, 0.0f, 1.0f);
	float out = crossfade(in, wet, mix);
	outputs[OUT_OUTPUT].value = out;
}


struct DelayWidget : ModuleWidget {
	DelayWidget(Delay *module);
};

DelayWidget::DelayWidget(Delay *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Delay.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(67, 57), module, Delay::TIME_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(67, 123), module, Delay::FEEDBACK_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(67, 190), module, Delay::COLOR_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(67, 257), module, Delay::MIX_PARAM, 0.0f, 1.0f, 0.5f));

	addInput(Port::create<PJ301MPort>(Vec(14, 63), Port::INPUT, module, Delay::TIME_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(14, 129), Port::INPUT, module, Delay::FEEDBACK_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(14, 196), Port::INPUT, module, Delay::COLOR_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(14, 263), Port::INPUT, module, Delay::MIX_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(14, 320), Port::INPUT, module, Delay::IN_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(73, 320), Port::OUTPUT, module, Delay::OUT_OUTPUT));
}


RACK_PLUGIN_MODEL_INIT(Fundamental, Delay) {
   Model *modelDelay = Model::create<Delay, DelayWidget>("Fundamental", "Delay", "Delay", DELAY_TAG);
   return modelDelay;
}

