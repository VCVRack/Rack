#include "FrozenWasteland.hpp"
#include "dsp/decimator.hpp"
#include "dsp/digital.hpp"

#include <complex>

using namespace std;

#define BUFFER_SIZE 32000
#define FRAME_COUNT 10
#define FREQUENCY_BANDS 6

#ifndef FCOMPLEX_H_
#define FCOMPLEX_H_
#define J fcomplex(0.0,1.0)
typedef std::complex<float> fcomplex;
#endif /* FCOMPLEX_H_ */

namespace rack_plugin_FrozenWasteland {

struct Sabonis : Module {
	enum ParamIds {
		SMOOTHENESS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_IN,		
		NUM_INPUTS
	};
	enum OutputIds {
		TRANSIENT_OUTPUT,
		TONAL_OUTPUT,
		NUM_OUTPUTS
	};
	
	int defaultWindowSize = 2048; //11ms window. NOTE add detection for sample rate change
	int halfWindowSize = defaultWindowSize / 2;
	int hopSize = 512; //30ms overlap. NOTE add detection for sample rate change

	float windowFunction[BUFFER_SIZE] = {};
	float signalBuffer[BUFFER_SIZE] = {};
	float X[BUFFER_SIZE][FREQUENCY_BANDS] = {};

	void CreateBlackmanHarrisWindow(int windowSize)
	{
		float a0 = 0.35875;
		float a1 = 0.48829;
		float a2 = 0.14128;
		float a3 = 0.01168;
		
		for(int n=0;n<halfWindowSize;n++)
		{
			windowFunction[n] = a0 - a1*cosf(2*M_PI*n/(halfWindowSize-1)) + a2*cosf(4*M_PI*n/(halfWindowSize-1)) - a3*cosf(6*M_PI*n/(halfWindowSize-1));
		}		
	}

	void stft(int n) {
		float total;

		for(int k=0;k<FREQUENCY_BANDS;k++) {
			total = 0.0f;
			for(int m=-halfWindowSize; m<halfWindowSize-1;m++) {
				total += signalBuffer[n*hopSize + m] * windowFunction[m] * std::real(exp((-J) * 2.0f * (float)M_PI * (float)m * (float)k / (float)defaultWindowSize));
			}
			X[n][k] = total;
		}
	}	

	void transientDetection(int n) {
		float energy;
		for(int k=0;k<FREQUENCY_BANDS;k++) {
		}
	}

	Sabonis() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {

		CreateBlackmanHarrisWindow(defaultWindowSize);
	}

	void step() override;
};

void Sabonis::step() {
	
	float signalIn = inputs[SIGNAL_IN].value/5;


	float out = 0.0;



	outputs[TONAL_OUTPUT].value = out / 4.0;	
}







struct SabonisWidget : ModuleWidget {
	SabonisWidget(Sabonis *module);
};

SabonisWidget::SabonisWidget(Sabonis *module) : ModuleWidget(module) {
	box.size = Vec(15*9, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Sabonis.svg")));
		addChild(panel);
	}
		
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(14, 84), module, Sabonis::SMOOTHENESS_PARAM, 0, 1.0, .25));

	addInput(Port::create<PJ301MPort>(Vec(10, 170), Port::INPUT, module, Sabonis::SIGNAL_IN));

	addOutput(Port::create<PJ301MPort>(Vec(10, 255), Port::OUTPUT, module, Sabonis::TRANSIENT_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10, 305), Port::OUTPUT, module, Sabonis::TONAL_OUTPUT));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, Sabonis) {
   Model *modelSabonis = Model::create<Sabonis, SabonisWidget>("Frozen Wasteland", "Sabonis", "Sabonis", FILTER_TAG);
   return modelSabonis;
}
