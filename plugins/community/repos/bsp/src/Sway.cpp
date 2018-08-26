
#include <math.h>

#include "bsp.hpp"

namespace rack_plugin_bsp {

struct Sway : Module {
	enum ParamIds {
		MIN_T_PARAM,
		MAX_T_PARAM,
		MIN_A_PARAM,
		MAX_A_PARAM,
      SCALE_A_PARAM,
      OFFSET_A_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CTL_OUTPUT,
		NUM_OUTPUTS
	};

   static const int32_t SCALE_T_SEC = 60;
   static const int32_t SCALE_MAX = 5;

	float sampleRate;

   float cur_rand_val_step;
   float cur_rand_val;
   int   cur_rand_val_countdown;

   float last_min_t;
   float last_max_t;

#if 0
	void onRandomize() override {
      cur_rand_val_countdown = -1;
   }
#endif

   static float randf(float _max) {
      return ((rand()*(0.999999999999f / float(RAND_MAX))) * _max);
   }

   void handleSampleRateChanged(void) {
      sampleRate = engineGetSampleRate();
   }

	Sway() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
      handleSampleRateChanged();
      cur_rand_val_countdown = -1;
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


void Sway::step() {

   if(params[MIN_T_PARAM].value != last_min_t)
   {
      last_min_t = params[MIN_T_PARAM].value;
      cur_rand_val_countdown = -1;
   }
   else if(params[MAX_T_PARAM].value != last_max_t)
   {
      last_max_t = params[MAX_T_PARAM].value;
      cur_rand_val_countdown = -1;
   }

   if(cur_rand_val_countdown < 0)
   {
      // First sample after reset / init
      float minA = params[MIN_A_PARAM].value;
      float maxA = params[MAX_A_PARAM].value;

      if(minA > maxA)
      {
         float t = minA;
         minA = maxA;
         maxA = t;
      }

      cur_rand_val = randf(maxA - minA) + minA;
   }

   if(--cur_rand_val_countdown <= 0)
   {
      // Next target val
      float minT = params[MIN_T_PARAM].value;
      float maxT = params[MAX_T_PARAM].value;
      float minA = params[MIN_A_PARAM].value;
      float maxA = params[MAX_A_PARAM].value;

      // Sort min / max
      if(minT > maxT)
      {
         float t = minT;
         minT = maxT;
         maxT = t;
      }

      if(minA > maxA)
      {
         float t = minA;
         minA = maxA;
         maxA = t;
      }

      // Bias towards slow modulation
      minT = minT * minT;
      minT = minT * minT;

      maxT = maxT * maxT;
      maxT = maxT * maxT;

      cur_rand_val_countdown = int((randf(maxT - minT) + minT) * (sampleRate * float(SCALE_T_SEC)));
      if(cur_rand_val_countdown < 1)
         cur_rand_val_countdown = 1;

      float nextVal = randf(maxA - minA) + minA;
      cur_rand_val_step = (nextVal - cur_rand_val) / cur_rand_val_countdown;
   }

   cur_rand_val += cur_rand_val_step;

	// Set output
	      float scaleA = params[SCALE_A_PARAM].value;
	const float offA   = params[OFFSET_A_PARAM].value;

   // Bias towards subtle modulation
   scaleA = scaleA * scaleA;
   scaleA = scaleA * scaleA;
   scaleA *= float(SCALE_MAX);

	outputs[CTL_OUTPUT].value = cur_rand_val * scaleA + offA;
}


struct SwayWidget : ModuleWidget {
	SwayWidget(Sway *module);

#if 0
   void randomize() override {
      module->onRandomize();
   }
#endif
};

SwayWidget::SwayWidget(Sway *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/sway.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

	// addParam(ParamWidget::create<CKSS>(Vec(15, 50), module, Obxd_VCF::FOURPOLE_PARAM, 0.0f, 1.0f, 0.0f));

#define HL 55
#define HS 40
   float cx = 9.0f;
   float cy = 50.0f;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Sway::MIN_T_PARAM, 0.0f, 1.0f, 0.2f));
   cy += HS;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Sway::MAX_T_PARAM, 0.0f, 1.0f, 0.3f));
   cy += HL;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Sway::MIN_A_PARAM, -1.0f, 1.0f, -1.0f));
   cy += HS;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Sway::MAX_A_PARAM, -1.0f, 1.0f, 1.0f));
   cy += HL;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Sway::SCALE_A_PARAM, 0.0f, 1.0f, 0.1f));
   cy += HS;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Sway::OFFSET_A_PARAM,-5.0f, 5.0f, 0.0f));

	addOutput(Port::create<PJ301MPort>(Vec(11, 325), Port::OUTPUT, module, Sway::CTL_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, Sway) {
   Model *modelSway = Model::create<Sway, SwayWidget>("bsp", "Sway", "Sway", NOISE_TAG, OSCILLATOR_TAG, LFO_TAG);
   return modelSway;
}
