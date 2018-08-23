
// Obxd filter code by Filatov Vadim (see dep/Obxd/Filter.h)
// module interface code based on Fundamental.VCF by Andrew Belt
// Obxd module integration by bsp

// (todo) replace UI ! (any volunteers ?)

#include "bsp.hpp"

#include "../dep/Obxd/AudioUtils.h"
#include "../dep/Obxd/Filter.h"

namespace rack_plugin_bsp {

#define sMIN(a,b) (((a)>(b))?(b):(a))

struct Obxd_VCF : Module {
	enum ParamIds {
		FREQ_PARAM,
		MULTI_PARAM,
		RES_PARAM,
		FREQ_CV_PARAM,
		DRIVE_PARAM,
		FOURPOLE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_INPUT,
		RES_INPUT,
		DRIVE_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LPF_OUTPUT,
		HPF_OUTPUT,
		NUM_OUTPUTS
	};

	float sampleRate;
	float sampleRateInv;

	float d1,d2;
	float c1,c2;

	float brightCoef;
	float briHold;
	Filter flt;

   void handleSampleRateChanged(void) {
      sampleRate = engineGetSampleRate();
      sampleRateInv = 1.0f / sampleRate;
      flt.setSampleRate(sampleRate);
      setBrightness(sampleRate*0.5f);
   }

	Obxd_VCF() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		brightCoef = briHold = 1.0f;
		c1=c2=d1=d2=0;

      handleSampleRateChanged();

      //flt.setMultimode(0.0f);
   }

	void setBrightness(float val)
	{
		briHold = val;
		brightCoef = tanf(sMIN(val, flt.SampleRate*0.5f-10.f) * float(M_PI)*flt.sampleRateInv);
	}

	void step() override;

	void onReset() override {
      handleSampleRateChanged();
	}

   void onSampleRateChange() override {
      Module::onSampleRateChange();

      handleSampleRateChanged();
   }
};


void Obxd_VCF::step() {
	float input = inputs[IN_INPUT].value / 5.0f;
	float drive = params[DRIVE_PARAM].value + inputs[DRIVE_INPUT].value / 10.0f;
	float gain = powf(100.0f, drive);
	input *= gain;
	// // Add -60dB noise to bootstrap self-oscillation
	// input += 1e-6f * (2.0f*randomUniform() - 1.0f);

	// Set resonance
	float res = params[RES_PARAM].value + inputs[RES_INPUT].value / 5.0f;
	res = clamp(res, 0.0f, 1.0f);
	flt.setResonance(res);

   // Multimode
   float mm = params[MULTI_PARAM].value;
   mm = clamp(mm, 0.0f, 1.0f);
   flt.setMultimode(mm);

	// Set cutoff frequency
	float cutoffExp = params[FREQ_PARAM].value + params[FREQ_CV_PARAM].value*2.0f * inputs[FREQ_INPUT].value / 5.0f;
	cutoffExp = clamp(cutoffExp, 0.0f, 1.0f);
	const float minCutoff = 15.0f;
	const float maxCutoff = 16700.0f;
	float cutoff = minCutoff * powf(maxCutoff / minCutoff, cutoffExp);

   // float cutoffcalc = jmin(
   //    getPitch(
	// 		cutoff+
	// 		FltDetune*FltDetAmt+
	// 		fenvamt*fenvd.feedReturn(envm)+
	// 		-45 + (fltKF*(ptNote+40))
   //             )
   //    //noisy filter cutoff
   //    +(ng.nextFloat()-0.5f)*3.5f
   //    , (flt.SampleRate*0.5f-120.0f));//for numerical stability purposes
   float cutoffcalc = cutoff * 2.0f;

   if(cutoffcalc > (sampleRate*0.5f-120.0f))
      cutoffcalc = (sampleRate*0.5f-120.0f);

	// Filter
   float x1 = input - tptlpupw(c1, input, 12, sampleRateInv);
   x1 = tptpc(d2, x1, brightCoef);

	bool bFourPole;  // true=24db lowpass, false=12db multimode
   bFourPole = (params[FOURPOLE_PARAM].value >= 0.5f);

   if(bFourPole)
      x1 = flt.Apply4Pole(x1, cutoffcalc); 
   else
      x1 = flt.Apply(x1, cutoffcalc); 

#if 0
   static int xxx = 0;
   if(0 == (++xxx & 16383))
      printf("xxx cutoffcalc=%f brightCoef=%f mm=%f sampleRate=%f sampleRateInv=%f input=%f x1=%f\n", cutoffcalc, brightCoef, mm, sampleRate, sampleRateInv, input, x1);
#endif

	// Set outputs
   x1 *= 5.0f;
	outputs[LPF_OUTPUT].value = x1;
	outputs[HPF_OUTPUT].value = inputs[IN_INPUT].value - x1;
}


struct Obxd_VCFWidget : ModuleWidget {
	Obxd_VCFWidget(Obxd_VCF *module);
};

Obxd_VCFWidget::Obxd_VCFWidget(Obxd_VCF *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Obxd_VCF.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<CKSS>(Vec(15, 50), module, Obxd_VCF::FOURPOLE_PARAM, 0.0f, 1.0f, 0.0f));

	addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(33, 61), module, Obxd_VCF::FREQ_PARAM, -1.0f, 1.0f, 0.75f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(12, 143), module, Obxd_VCF::MULTI_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(71, 143), module, Obxd_VCF::RES_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(12, 208), module, Obxd_VCF::FREQ_CV_PARAM, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(71, 208), module, Obxd_VCF::DRIVE_PARAM, 0.0f, 1.0f, 0.5f));

	addInput(Port::create<PJ301MPort>(Vec(10, 276), Port::INPUT, module, Obxd_VCF::FREQ_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(48, 276), Port::INPUT, module, Obxd_VCF::RES_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(85, 276), Port::INPUT, module, Obxd_VCF::DRIVE_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10, 320), Port::INPUT, module, Obxd_VCF::IN_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(48, 320), Port::OUTPUT, module, Obxd_VCF::LPF_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(85, 320), Port::OUTPUT, module, Obxd_VCF::HPF_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, Obxd_VCF) {
   Model *modelVCF = Model::create<Obxd_VCF, Obxd_VCFWidget>("bsp", "Obxd_VCF", "Obxd-VCF", FILTER_TAG);
   return modelVCF;
}
