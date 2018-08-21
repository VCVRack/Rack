/*

TaHaSaHaN

TrackAndHoldAndSampleAndHoldandNoise

the mix knob blends SaH and TaH signals into one output.
left side: SaH
right side: TaH

TODO: cv handling

*/////////////////////////////////////////////////////////////////////////////


#include "pvc.hpp"
#include "dsp/digital.hpp" // SchmittTrigger // PulseGenerator

namespace rack_plugin_PvC {

struct TaHaSaHaN : Module {
	enum ParamIds {
		BLEND,
		
		NUM_PARAMS
	};

	enum InputIds {
		SAMPLE,
		TRIGGER,
		BLEND_CV,
		
		NUM_INPUTS
	};

	enum OutputIds {
		MIX,
		SNH,
		TNH,
		NOISE,
	
		NUM_OUTPUTS
	};

	enum LightIds {
	
		NUM_LIGHTS
	};

	SchmittTrigger sampleTrigger;

	float input = 0.0f;
	float snhOut = 0.0f;
	float tnhOut = 0.0f;
	float noise = 0.0f;

	TaHaSaHaN() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

	void reset() override {
		input = snhOut = tnhOut = noise = 0.0f;
	}

};

void TaHaSaHaN::step() {
	noise = randomUniform()*10.0f - 5.0f;

	if (inputs[TRIGGER].active){
		input = inputs[SAMPLE].normalize(noise);
		if (sampleTrigger.process(inputs[TRIGGER].value)){
			snhOut = input;
		}
		if (inputs[TRIGGER].value > 0.01f){
			tnhOut = input;
		}
	}
	else snhOut = tnhOut = 0.0f;

	// TODO: better cv/knob interaction.
	float blend = params[BLEND].value;
	if (inputs[BLEND_CV].active)
		blend *= clamp(inputs[BLEND_CV].value * 0.1f, 0.0f, 1.0f);

	outputs[MIX].value = crossfade(snhOut,tnhOut,blend);
	outputs[SNH].value = snhOut;
	outputs[TNH].value = tnhOut;
	outputs[NOISE].value = noise;

}

struct TaHaSaHaNWidget : ModuleWidget {
	TaHaSaHaNWidget(TaHaSaHaN *module);
};

TaHaSaHaNWidget::TaHaSaHaNWidget(TaHaSaHaN *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/panels/TaHaSaHaN.svg")));

	// screws
	addChild(Widget::create<ScrewHead4>(Vec(0, 0)));
	// addChild(Widget::create<ScrewHead1>(Vec(box.size.x - 15, 0)));
	// addChild(Widget::create<ScrewHead3>(Vec(0, 365)));
	addChild(Widget::create<ScrewHead2>(Vec(box.size.x - 15, 365)));

	addInput(Port::create<InPortAud>(Vec(4,22), Port::INPUT, module, TaHaSaHaN::SAMPLE));
	addInput(Port::create<InPortBin>(Vec(4,64), Port::INPUT, module, TaHaSaHaN::TRIGGER));
	

	addOutput(Port::create<OutPortVal>(Vec(4,132), Port::OUTPUT, module, TaHaSaHaN::TNH));
	addOutput(Port::create<OutPortVal>(Vec(4,176), Port::OUTPUT, module, TaHaSaHaN::SNH));
	addOutput(Port::create<OutPortVal>(Vec(4,220), Port::OUTPUT, module, TaHaSaHaN::NOISE));

	addParam(ParamWidget::create<PvCKnob>(Vec(4,258),module, TaHaSaHaN::BLEND, 0.0f, 1.0f, 0.5f));
	addInput(Port::create<InPortCtrl>(Vec(4,282), Port::INPUT, module, TaHaSaHaN::BLEND_CV));
	addOutput(Port::create<OutPortVal>(Vec(4,336), Port::OUTPUT, module, TaHaSaHaN::MIX));
	
}

} // namespace rack_plugin_PvC

using namespace rack_plugin_PvC;

RACK_PLUGIN_MODEL_INIT(PvC, TaHaSaHaN) {
   Model *modelTaHaSaHaN = Model::create<TaHaSaHaN, TaHaSaHaNWidget>(
      "PvC", "TaHaSaHaN", "TaHaSaHaN", SAMPLE_AND_HOLD_TAG, NOISE_TAG, RANDOM_TAG);
   return modelTaHaSaHaN;
}
