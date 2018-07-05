#include "FrozenWasteland.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_FrozenWasteland {

struct LowFrequencyOscillator {
	float phase = 0.0;
	float pw = 0.5;
	float freq = 1.0;
	bool offset = false;
	bool invert = false;
	SchmittTrigger resetTrigger;
	LowFrequencyOscillator() {}
	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0);
		freq = powf(2.0, pitch);
	}
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01;
		pw = clamp(pw_, pwMin, 1.0f - pwMin);
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset)) {
			phase = 0.0;
		}
	}
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
	float tri(float x) {
		return 4.0 * fabsf(x - roundf(x));
	}
	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5 : phase);
		else
			return -1.0 + tri(invert ? phase - 0.25 : phase - 0.75);
	}
	float saw(float x) {
		return 2.0 * (x - roundf(x));
	}
	float saw() {
		if (offset)
			return invert ? 2.0 * (1.0 - phase) : 2.0 * phase;
		else
			return saw(phase) * (invert ? -1.0 : 1.0);
	}
	float sqr() {
		float sqr = (phase < pw) ^ invert ? 1.0 : -1.0;
		return offset ? sqr + 1.0 : sqr;
	}
	float light() {
		return sinf(2*M_PI * phase);
	}
};



struct QuantussyCell : Module {
	enum ParamIds {
		FREQ_PARAM,
		CV_ATTENUVERTER_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CASTLE_INPUT,
		CV_INPUT,
		CV_AMOUNT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CASTLE_OUTPUT,
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};


	LowFrequencyOscillator oscillator;


	//Stuff for S&Hs
	SchmittTrigger _castleTrigger, _cvTrigger;
	float _value1, _value2;
	//Castle S&H is #1, CV #2

	QuantussyCell() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void QuantussyCell::step() {
	oscillator.setPitch(params[FREQ_PARAM].value  + _value2);
	oscillator.step(1.0 / engineGetSampleRate());

	outputs[SIN_OUTPUT].value = 5.0 * oscillator.sin();
	outputs[TRI_OUTPUT].value = 5.0 * oscillator.tri();
	outputs[SAW_OUTPUT].value = 5.0 * oscillator.saw();

	float squareOutput = 5.0 * oscillator.sqr(); //Used a lot :)
	outputs[SQR_OUTPUT].value = squareOutput;


	//Process Castle
	if (_castleTrigger.process(squareOutput)) {
		if (inputs[CASTLE_INPUT].active) {
			_value1 = inputs[CASTLE_INPUT].value;
		}
		else {
			_value1 = 0; //Maybe at some point add a default noise source, but not for now
		}
	}
	outputs[CASTLE_OUTPUT].value = _value1;

	//Process CV
	if (_cvTrigger.process(squareOutput)) {
		if (inputs[CV_INPUT].active) {
			float attenuverting = params[CV_ATTENUVERTER_PARAM].value + (inputs[CV_AMOUNT_INPUT].value / 10.0f);
			_value2 = inputs[CV_INPUT].value * attenuverting;
		}
		else {
			_value2 = 0; //Maybe at some point add a default noise source, but not for now
		}
	}


	lights[BLINK_LIGHT].setBrightnessSmooth(fmaxf(0.0, oscillator.light()));

}

struct QuantussyCellWidget : ModuleWidget {
	QuantussyCellWidget(QuantussyCell *module);
};

QuantussyCellWidget::QuantussyCellWidget(QuantussyCell *module) : ModuleWidget(module) {
	box.size = Vec(15*6, RACK_GRID_HEIGHT);


	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/QuantussyCell.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, 0)));	
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(28, 64), module, QuantussyCell::FREQ_PARAM, -3.0, 3.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(13, 182), module, QuantussyCell::CV_ATTENUVERTER_PARAM, -1.0, 1.0, 1.0));

	addInput(Port::create<PJ301MPort>(Vec(35, 113), Port::INPUT, module, QuantussyCell::CASTLE_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(50, 205), Port::INPUT, module, QuantussyCell::CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(15, 213), Port::INPUT, module, QuantussyCell::CV_AMOUNT_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(35, 160), Port::OUTPUT, module, QuantussyCell::CASTLE_OUTPUT));

	addOutput(Port::create<PJ301MPort>(Vec(15, 255), Port::OUTPUT, module, QuantussyCell::SIN_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(50, 255), Port::OUTPUT, module, QuantussyCell::TRI_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(15, 301), Port::OUTPUT, module, QuantussyCell::SQR_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(50, 301), Port::OUTPUT, module, QuantussyCell::SAW_OUTPUT));



	addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(68, 70), module, QuantussyCell::BLINK_LIGHT));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, QuantussyCell) {
   Model *modelQuantussyCell = Model::create<QuantussyCell, QuantussyCellWidget>("Frozen Wasteland", "QuantussyCell", "Quantussy Cell", LOGIC_TAG);
   return modelQuantussyCell;
}
