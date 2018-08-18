//**************************************************************************************
//Formant Filter  Module for VCV Rack by Autodafe http://www.autodafe.net
//
//And part of code  Public source code by alex@smartelectronix.com
//on musicdsp.org: http://www.musicdsp.org/showArchiveComment.php?ArchiveID=110
//**************************************************************************************

#include "Autodafe.hpp"

namespace rack_plugin_Autodafe {

double CosineInterpolate(
   double y1,double y2,
   double mu)
{
   double mu2;

   mu2 = (1-cos(mu*M_PI))/2;
   return(y1*(1-mu2)+y2*mu2);
}

/*
  Public source code by alex@smartelectronix.com
  Simple example of implementation of formant filter
  Vowelnum can be 0,1,2,3,4 <=> A,E,I,O,U
  Good for spectral rich input like saw or square
*/
//-------------------------------------------------------------VOWEL COEFFICIENTS
const double coeff[9][11] = {
   { 3.11044e-06,
     8.943665402, -36.83889529, 92.01697887, -154.337906, 181.6233289,
     -151.8651235, 89.09614114, -35.10298511, 8.388101016, -0.923313471  ///A
   },

   {
      CosineInterpolate(3.11044e-06, 4.36215e-06,0.5), 
      CosineInterpolate(8.943665402, 8.90438318,0.5),
      CosineInterpolate(-36.83889529, -36.55179099,0.5),
      CosineInterpolate(92.01697887, 91.05750846,0.5),
      CosineInterpolate(-154.337906, -152.422234,0.5),
      CosineInterpolate(181.6233289, 179.1170248,0.5),
      CosineInterpolate(-151.8651235, -149.6496211,0.5),
      CosineInterpolate(89.09614114, 87.78352223,0.5),
      CosineInterpolate(-35.10298511, -34.60687431,0.5),
      CosineInterpolate(8.388101016, 8.282228154,0.5),
      CosineInterpolate(-0.923313471, -0.914150747,0.5)
   },

   { 4.36215e-06,
     8.90438318, -36.55179099, 91.05750846, -152.422234, 179.1170248,  ///E
     -149.6496211, 87.78352223, -34.60687431, 8.282228154, -0.914150747
   },

   {
      CosineInterpolate(4.36215e-06, 3.33819e-06,0.5), 
      CosineInterpolate(8.90438318, 8.893102966,0.5), 
      CosineInterpolate(-36.55179099, -36.49532826,0.5), 
      CosineInterpolate(91.05750846, 90.96543286,0.5), 
      CosineInterpolate(-152.422234, -152.4545478,0.5), 
      CosineInterpolate(179.1170248, 179.4835618,0.5), 
      CosineInterpolate(-149.6496211, -150.315433,0.5), 
      CosineInterpolate(87.78352223, 88.43409371,0.5), 
      CosineInterpolate(-34.60687431, -34.98612086,0.5), 
      CosineInterpolate(8.282228154, 8.407803364,0.5), 
      CosineInterpolate(-0.914150747, -0.932568035,0.5)
   },

   { 3.33819e-06,
     8.893102966, -36.49532826, 90.96543286, -152.4545478, 179.4835618,
     -150.315433, 88.43409371, -34.98612086, 8.407803364, -0.932568035  ///I
   },


   {
      CosineInterpolate(3.33819e-06 , 1.13572e-06,0.5), 
      CosineInterpolate(8.893102966 , 8.994734087,0.5), 
      CosineInterpolate(-36.49532826 , -37.2084849,0.5), 
      CosineInterpolate(90.96543286 , 93.22900521,0.5), 
      CosineInterpolate(-152.4545478 , -156.6929844,0.5), 
      CosineInterpolate(179.4835618 , 184.596544,0.5), 
      CosineInterpolate(-150.315433 , -154.3755513,0.5), 
      CosineInterpolate(88.43409371 , 90.49663749,0.5), 
      CosineInterpolate(-34.98612086 , -35.58964535,0.5), 
      CosineInterpolate(8.407803364 , 8.478996281,0.5), 
      CosineInterpolate(-0.932568035 , -0.929252233,0.5), 

   },

   { 1.13572e-06,
     8.994734087, -37.2084849, 93.22900521, -156.6929844, 184.596544,   ///O
     -154.3755513, 90.49663749, -35.58964535, 8.478996281, -0.929252233
   },


   {
      CosineInterpolate(1.13572e-06 , 4.09431e-07,0.5), 
      CosineInterpolate(8.994734087 , 8.997322763,0.5), 
      CosineInterpolate(-37.2084849 , -37.20218544,0.5), 
      CosineInterpolate(93.22900521 , 93.11385476,0.5), 
      CosineInterpolate(-156.6929844 , -156.2530937,0.5), 
      CosineInterpolate(184.596544 , 183.7080141,0.5), 
      CosineInterpolate(-154.3755513 , -153.2631681,0.5), 
      CosineInterpolate(90.49663749 , 89.59539726,0.5), 
      CosineInterpolate(-35.58964535 , -35.12454591,0.5), 
      CosineInterpolate(8.478996281 , 8.338655623,0.5), 
      CosineInterpolate(-0.929252233 , -0.910251753,0.5), 


   },



   { 4.09431e-07,
     8.997322763, -37.20218544, 93.11385476, -156.2530937, 183.7080141,  ///U
     -153.2631681, 89.59539726, -35.12454591, 8.338655623, -0.910251753
   }
};
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------

 
struct FFilter {

   double memory[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

   
   float formant_filter(float in, int vowelnum, float tone)
      {
         float res = (float)(coeff[vowelnum][0] * in/2 +
                             coeff[vowelnum][1] * memory[0] +
                             coeff[vowelnum][2] * memory[1] +
                             coeff[vowelnum][3] * memory[2] +
                             coeff[vowelnum][4] * memory[3] +
                             coeff[vowelnum][5] * memory[4] +
                             coeff[vowelnum][6] * memory[5] +
                             coeff[vowelnum][7] * memory[6] +
                             coeff[vowelnum][8] * memory[7] +
                             coeff[vowelnum][9] * memory[8] +
                             coeff[vowelnum][10] * memory[9]);

         memory[9] = memory[8];
         memory[8] = memory[7];
         memory[7] = memory[6];
         memory[6] = memory[5];
         memory[5] = memory[4];
         memory[4] = memory[3];
         memory[3] = memory[2];
         memory[2] = memory[1];
         memory[1] = memory[0];
         memory[0] = (double)res;
         return res;
      }

};


struct FormantFilter : Module {
   enum ParamIds {
      VOWEL_PARAM,
      
      ATTEN_PARAM,
      NUM_PARAMS
   };
   enum InputIds {
      INPUT,
      CV_VOWEL,
      NUM_INPUTS
   };
   enum OutputIds {
      OUTPUT,
      NUM_OUTPUTS
   };

   FormantFilter();

   FFilter *ffilter = new FFilter();


   void step();
};
 
FormantFilter::FormantFilter() {
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
}


void FormantFilter::step() {
   float in = inputs[INPUT].value / 5.0f;
   
   int vowel = params[VOWEL_PARAM].value;
   
   float cv = clampf(inputs[CV_VOWEL].value * params[ATTEN_PARAM].value, 0, 8) ;
   
   float ffilterout = ffilter->formant_filter(in,clampf((vowel+cv), 0, 8), 0);

   outputs[OUTPUT].value= 5.0f*ffilterout; 
}

struct FormantFilterWidget : ModuleWidget{
	FormantFilterWidget(FormantFilter *module);
};

FormantFilterWidget::FormantFilterWidget(FormantFilter *module) : ModuleWidget(module) {
   box.size = Vec(15*6, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/FormantFilter.svg")));
      addChild(panel);
   }

   addChild(createScrew<ScrewSilver>(Vec(5, 0)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 0)));
   addChild(createScrew<ScrewSilver>(Vec(5, 365)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));

   addParam(createParam<AutodafeKnobBlueBig>(Vec(18,61), module, FormantFilter::VOWEL_PARAM, 0.0, 9.0, 0.0));
   
   addParam(createParam<AutodafeKnobBlue>(Vec(27, 190), module, FormantFilter::ATTEN_PARAM, -1.0, 1.0, 0.0));
   
   addInput(createInput<PJ301MPort>(Vec(32, 150), module, FormantFilter::CV_VOWEL));

   addInput(createInput<PJ301MPort>(Vec(10, 320), module, FormantFilter::INPUT));

   addOutput(createOutput<PJ301MPort>(Vec(48, 320), module, FormantFilter::OUTPUT));
 
}

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

RACK_PLUGIN_MODEL_INIT(Autodafe, FormantFilter) {
   return Model::create<FormantFilter, FormantFilterWidget>("Autodafe", "Formant Filter", "Formant Filter", FILTER_TAG);
}
