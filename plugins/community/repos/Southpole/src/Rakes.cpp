#include "Southpole.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_Southpole {

#define NBUF 6 

struct Rakes : Module {
   enum ParamIds {
      DECAY_PARAM,
      QUANT_PARAM,
      MIX_PARAM,
      TUNE1_PARAM,
      TUNE2_PARAM,
      TUNE3_PARAM,
      TUNE4_PARAM,
      TUNE5_PARAM,
      TUNE6_PARAM,
      FINE1_PARAM,
      FINE2_PARAM,
      FINE3_PARAM,
      FINE4_PARAM,
      FINE5_PARAM,
      FINE6_PARAM,
      GAIN1_PARAM,
      GAIN2_PARAM,
      GAIN3_PARAM,
      GAIN4_PARAM,
      GAIN5_PARAM,
      GAIN6_PARAM,
      NUM_PARAMS
   };
   enum InputIds {
      INL_INPUT,
      INR_INPUT,
      DECAY_INPUT,
      MIX_INPUT,
      TUNE1_INPUT,
      TUNE2_INPUT,
      TUNE3_INPUT,
      TUNE4_INPUT,
      TUNE5_INPUT,
      TUNE6_INPUT,
      //GAIN1_INPUT,GAIN2_INPUT,
      //GAIN3_INPUT,GAIN4_INPUT,
      //GAIN5_INPUT,GAIN6_INPUT,
      NUM_INPUTS
   };
   enum OutputIds {
      OUTL_OUTPUT,
      OUTR_OUTPUT,
      NUM_OUTPUTS
   };
   enum LightIds {
      NUM_LIGHTS
   };

   //SchmittTrigger clock;

   float *bufl[NBUF];
   float *bufr[NBUF];
   int maxsize;

   int headl[NBUF];
   int headr[NBUF];

   int sizel[NBUF];
   int sizer[NBUF];

   int lastsizel[NBUF];
   int lastsizer[NBUF];

   Rakes() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {  

      maxsize = engineGetSampleRate();

      for (int j=0; j < NBUF; j++) {
         bufl[j] = new float [maxsize];
         bufr[j] = new float [maxsize];
         for (int i=0; i < maxsize; i++) {
            bufl[j][i] = 0;      
            bufr[j][i] = 0;      
         }
         headl[j] = 0;
         headr[j] = 0;
      }
   }

   float xm1 = 0;
   float ym1 = 0;

   float dcblock( float x ) {
      float y = x - xm1 + 0.995 * ym1;
      xm1 = x;
      ym1 = y;
      return y;
   }

   void step() override;
};



void Rakes::step() {

   //float mix  = clamp(params[MIX_PARAM].value + inputs[MIX_INPUT].normalize(0.) / 10.0, 0.0, 1.0);
   float mix    = params[MIX_PARAM].value;
   float rate   = clamp(params[DECAY_PARAM].value + inputs[DECAY_INPUT].normalize(0.) / 10.0, 0.0f, .99f);

   const float f0 = 261.626;
   float inl  = inputs[INL_INPUT].normalize(0.);
   float inr  = inputs[INR_INPUT].normalize(inl);

   float sumoutl  = 0;
   float sumoutr  = 0;
   float sumgain = 1.;

   for (int j=0; j < NBUF; j++) {
      //float gain = clamp(params[GAIN1_PARAM + j].value + inputs[GAIN1_INPUT + j].normalize(0.) / 10.0, 0.0, 1.0);
      float gain = params[GAIN1_PARAM + j].value;
      if (gain < 1e-3) continue;
      sumgain += gain;

      float tune = clamp(params[TUNE1_PARAM + j].value + inputs[TUNE1_INPUT + j].normalize(0.), -5.0, 5.5);
      float fine = clamp(params[FINE1_PARAM + j].value, -1.0, 1.0);

      if ( params[QUANT_PARAM].value > 0.5 ) {
         tune = round(12.*tune)/12.;
      }

      float freql = f0 * powf(2., tune + fine/12.);
      float freqr = f0 * powf(2., tune - fine/12.);

      // key follow
      //float fb = crossfade(f0, freq, follow);

      // full follow decay rate is T60 time
      float fbl = pow(10, -3./freql/fabs(5.*rate));
      float fbr = pow(10, -3./freqr/fabs(5.*rate));

      //fb = fb * ((0. < rate) - (rate < 0.));

      //printf("%f %f %f\n",lfreq,rate,fb);

      sizel[j] = maxsize / freqr;
      sizer[j] = maxsize / freql;

      if (sizel[j] > lastsizel[j]) {
         for (int i=sizel[j]; i < lastsizel[j]; i++ ) bufl[i]=0;        
      }
      if (sizel[j] > lastsizer[j]) {
         for (int i=sizer[j]; i < lastsizer[j]; i++ ) bufr[i]=0;
      }

      lastsizel[j] = maxsize / freqr;
      lastsizer[j] = maxsize / freql;

      float outl = bufl[j][headl[j]];
      float outr = bufr[j][headr[j]];

      bufl[j][headl[j]] = inl + fbl * outl;
      bufr[j][headr[j]] = inr + fbr * outr;
      headl[j]++;
      headr[j]++;
      if (headl[j] > sizel[j]) { headl[j] = 0; }
      if (headr[j] > sizer[j]) { headr[j] = 0; }

      sumoutl  += gain*outl;
      sumoutr  += gain*outr;
   }

   sumoutl = clamp( dcblock(sumoutl) / sumgain, -10., 10.); //in + gain*out;
   sumoutr = clamp( dcblock(sumoutr) / sumgain, -10., 10.); //in + gain*out;

   outputs[OUTL_OUTPUT].value = crossfade(inl,sumoutl,mix);
   outputs[OUTR_OUTPUT].value = crossfade(inr,sumoutr,mix);
}

struct RakesWidget : ModuleWidget { 
   
   RakesWidget(Rakes *module) : ModuleWidget(module) {
   
      box.size = Vec(15*8, 380);

      {
         SVGPanel *panel = new SVGPanel();
         panel->box.size = box.size;
         panel->setBackground(SVG::load(assetPlugin(plugin, "res/Rakes.svg")));
         addChild(panel);
      }

      const float x1 = 5.;
      const float x2 = 35.;
      const float x3 = 65.;
      const float x4 = 95.;
      const float y1 = 40.;
      const float yh = 32.;
      
      addInput(Port::create<sp_Port>(Vec(x2, y1+0*yh), Port::INPUT, module, Rakes::DECAY_INPUT));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x3, y1+0*yh), module, Rakes::DECAY_PARAM, 0.0, 1.0, 0.0));
      //addParam(ParamWidget::create<sp_SmallBlackKnob>  (Vec(x3, y1+0*yh), module, Rakes::FOLLOW_PARAM, 0.0, 1.0, 0.0));

      for (int j=0; j < NBUF; j++) {
         addInput(Port::create<sp_Port>         (Vec(x1, y1+(j+1.5)*yh), Port::INPUT, module, Rakes::TUNE1_INPUT + j));
         addParam(ParamWidget::create<sp_SmallBlackKnob> (Vec(x2, y1+(j+1.5)*yh), module, Rakes::TUNE1_PARAM + j,  -5.0, 5.5, 0.0));
         addParam(ParamWidget::create<sp_SmallBlackKnob> (Vec(x3, y1+(j+1.5)*yh), module, Rakes::FINE1_PARAM + j,  -1.0, 1.0, 0.0));
         addParam(ParamWidget::create<sp_SmallBlackKnob> (Vec(x4, y1+(j+1.5)*yh), module, Rakes::GAIN1_PARAM + j,  0.0, 1.0, 0.0));
      }


      addInput(Port::create<sp_Port>   (Vec(x1, y1+8*yh), Port::INPUT, module, Rakes::INL_INPUT));
      addInput(Port::create<sp_Port>   (Vec(x1, y1+9*yh), Port::INPUT, module, Rakes::INR_INPUT));

      addParam(ParamWidget::create<CKSS>( Vec(x2, y1+7.5*yh), module, Rakes::QUANT_PARAM, 0.0, 1.0, 0.0));
      addParam(ParamWidget::create<sp_SmallBlackKnob> (Vec((x2+x3)/2., y1+8.5*yh), module, Rakes::MIX_PARAM, 0.0, 1.0, 0.5));

      addOutput(Port::create<sp_Port> (Vec(x4, y1+8*yh), Port::OUTPUT, module, Rakes::OUTL_OUTPUT));
      addOutput(Port::create<sp_Port> (Vec(x4, y1+9*yh), Port::OUTPUT, module, Rakes::OUTR_OUTPUT));
   }
};

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Rakes) {
   Model *modelRakes    = Model::create<Rakes,RakesWidget>(  "Southpole", "Rakes",      "Rakes - resonator bank", FILTER_TAG);
   return modelRakes;
}
