#include "Stochasm.hpp"

namespace rack_plugin_com_soundchasing_stochasm {

#define LFSR_TAPS_MAX_32 0xA3000000u;

#define sMIN(a,b) (((a)>(b))?(b):(a))
#define sMAX(a,b) (((a)>(b))?(a):(b))

struct Resonator : Module {
   enum ParamIds {
      UPPER_CHAMBER,
      UPPER_FILTER1,
      UPPER_FILTER2,
      UPPER_MANUAL,
      LOWER_CHAMBER,
      LOWER_FILTER1,
      LOWER_FILTER2,
      LOWER_MANUAL,
      NUM_PARAMS
   };
   enum InputIds {
      UPPER_VOCT,
      UPPER_GATE,
      LOWER_VOCT,
      LOWER_GATE,
      NUM_INPUTS
   };
   enum OutputIds {
      UPPER_OUT,
      UPPER_BITS,
      LOWER_OUT,
      LOWER_BITS,
      NUM_OUTPUTS
   };
   enum LightIds {
      BLINK_LIGHT,
      NUM_LIGHTS
   };

   const int FILTER_MASK[11] = {15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383};
   const float IIR_FILTER_DECAY = 0.09350953781417137f;

   unsigned char uDelay[65536] = {0};
   int uDelayIndex = 0;
   int uDelaySize = 0;

   unsigned int uFilter1LFSR = 0xa3005670;
   int uFilter1Size = 5;
   int uFilter1Count = 0;

   unsigned int uFilter2LFSR = 0x6a10d2be;
   int uFilter2Size = 5;
   int uFilter2Count = 0;

   float uCurrent = 0.f;
   float uPre = 0.f;

   unsigned char lDelay[65536] = {0};
   int lDelayIndex = 0;
   int lDelaySize = 0;

   unsigned int lFilter1LFSR = 0x2ea88906;
   int lFilter1Size = 5;
   int lFilter1Count = 0;

   unsigned int lFilter2LFSR = 0xc1d18e4c;
   int lFilter2Size = 5;
   int lFilter2Count = 0;

   float lCurrent = 0.f;
   float lPre = 0.f;


   Resonator() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
   void step() override;

   // For more advanced Module features, read Rack's engine.hpp header file
   // - toJson, fromJson: serialization of internal data
   // - onSampleRateChange: event triggered by a change of sample rate
   // - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void Resonator::step() {
   int i;
        
   uDelaySize = pow(2, clamp(params[UPPER_CHAMBER].value + inputs[UPPER_VOCT].value, 0.f, 10.f) +6);
   lDelaySize = pow(2, clamp(params[LOWER_CHAMBER].value + inputs[LOWER_VOCT].value, 0.f, 10.f) +6);

   uDelayIndex %= uDelaySize;
   lDelayIndex %= lDelaySize;

   int size;
   size = (int)params[UPPER_FILTER1].value;
   if (size - uFilter1Size > 0)
      uFilter1Size << (size - uFilter1Size);
   else if (uFilter1Size - size > 0)
      uFilter1Size >> (uFilter1Size - size);
   uFilter1Size = size;

   size = (int)params[UPPER_FILTER2].value;
   if (size - uFilter2Size > 0)
      uFilter2Size << (size - uFilter2Size);
   else if (uFilter2Size - size > 0)
      uFilter2Size >> (uFilter2Size - size);
   uFilter2Size = size;

   size = (int)params[LOWER_FILTER1].value;
   if (size - lFilter1Size > 0)
      lFilter1Size << (size - lFilter1Size);
   else if (lFilter1Size - size > 0)
      lFilter1Size >> (lFilter1Size - size);
   lFilter1Size = size;

   size = (int)params[LOWER_FILTER2].value;
   if (size - lFilter2Size > 0)
      lFilter2Size << (size - lFilter2Size);
   else if (lFilter2Size - size > 0)
      lFilter2Size >> (lFilter2Size - size);
   lFilter2Size = size;


   for (i = 0; i < 32; i++)
   {
      bool uOut1, uOut2, lOut1, lOut2, uDelayOut, lDelayOut;
      //Update LFSR Values
      unsigned lsb;
      lsb = uFilter1LFSR & 1;
      uFilter1LFSR >>= 1;
      uFilter1LFSR ^= (-lsb) & LFSR_TAPS_MAX_32;
      lsb = uFilter2LFSR & 1;
      uFilter2LFSR >>= 1;
      uFilter2LFSR ^= (-lsb) & LFSR_TAPS_MAX_32;

      lsb = lFilter1LFSR & 1;
      lFilter1LFSR >>= 1;
      lFilter1LFSR ^= (-lsb) & LFSR_TAPS_MAX_32;
      lsb = lFilter2LFSR & 1;
      lFilter2LFSR >>= 1;
      lFilter2LFSR ^= (-lsb) & LFSR_TAPS_MAX_32;

      if (params[UPPER_MANUAL].value > 0.f || inputs[UPPER_GATE].value > 1.7f)
      {
         //Determine out values
         uDelayOut = (bool) uDelay[uDelayIndex];
         uOut1 = (uFilter1LFSR & FILTER_MASK[uFilter1Size]) < uFilter1Count;
         uOut2 = (uFilter2LFSR & FILTER_MASK[uFilter2Size]) < uFilter2Count;

         uFilter1Count = (uDelayOut && !uOut2) ? sMIN(uFilter1Count+1, FILTER_MASK[uFilter1Size]) : 
            (!uDelayOut && uOut2) ? sMAX(uFilter1Count-1, 0) : uFilter1Count;
         uFilter2Count = (uOut1 && !uOut2) ? sMIN(uFilter2Count+1, FILTER_MASK[uFilter2Size]) : 
            (!uOut1 && uOut2) ? sMAX(uFilter2Count-1, 0) : uFilter2Count;

         uDelay[uDelayIndex] = uOut2;
         uPre = uPre + IIR_FILTER_DECAY * (((uOut2) ? 5.f : -5.f) - uPre);
         uCurrent = uCurrent + IIR_FILTER_DECAY * (uPre - uCurrent);
      }
      else
      {    
         uDelay[uDelayIndex] = (char)(uFilter2LFSR & 1);
         uPre = uPre + IIR_FILTER_DECAY * (-uPre);
         uCurrent = uCurrent + IIR_FILTER_DECAY * (-uCurrent);
      }

      if (params[LOWER_MANUAL].value > 0.f || inputs[LOWER_GATE].value > 1.7f)
      {
         //Determine out values
         lDelayOut = (bool) lDelay[lDelayIndex];
         lOut1 = (lFilter1LFSR & FILTER_MASK[lFilter1Size]) < lFilter1Count;
         lOut2 = (lFilter2LFSR & FILTER_MASK[lFilter2Size]) < lFilter2Count;

         lFilter1Count = (lDelayOut && !lOut2) ? sMIN(lFilter1Count+1, FILTER_MASK[lFilter1Size]) : 
            (!lDelayOut && lOut2) ? sMAX(lFilter1Count-1, 0) : lFilter1Count;
         lFilter2Count = (lOut1 && !lOut2) ? sMIN(lFilter2Count+1, FILTER_MASK[lFilter2Size]) : 
            (!lOut1 && lOut2) ? sMAX(lFilter2Count-1, 0) : lFilter2Count;

         lDelay[lDelayIndex] = lOut2;
         lPre = lPre + IIR_FILTER_DECAY * (((lOut2) ? 5.f : -5.f) - lPre);
         lCurrent = lCurrent + IIR_FILTER_DECAY * (lPre - lCurrent);
      }
      else
      {
         lDelay[lDelayIndex] = (char)(lFilter2LFSR & 1);
         lPre = lPre + IIR_FILTER_DECAY * (-lPre);
         lCurrent = lCurrent + IIR_FILTER_DECAY * (-lCurrent);
      }

      uDelayIndex = (uDelayIndex+1) % uDelaySize;
      lDelayIndex = (lDelayIndex+1) % lDelaySize;

   }

   outputs[UPPER_OUT].value = uCurrent;
   outputs[LOWER_OUT].value = lCurrent;
}


struct ResonatorWidget : ModuleWidget {
   ResonatorWidget(Resonator *module);
};


ResonatorWidget::ResonatorWidget(Resonator *module) : ModuleWidget(module) {
   
   //Resonator *module = new Resonator();
   //setModule(module);
   
   box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Resonator.svg")));
      addChild(panel);
   }

   addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
   addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
   addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

   //Upper
   addParam(ParamWidget::create<StochasmMintLargeKnob>(Vec(52, 30), module, Resonator::UPPER_CHAMBER, 0.0, 10.0, 5.0));
   addParam(ParamWidget::create<StochasmMintKnob>(Vec(48, 104), module, Resonator::UPPER_FILTER1, 0.0, 10.0, 5.0));
   addParam(ParamWidget::create<StochasmMintKnob>(Vec(85, 104), module, Resonator::UPPER_FILTER2, 0.0, 10.0, 5.0));
   addParam(ParamWidget::create<MintMomentarySwitch>(Vec(11, 81), module, Resonator::UPPER_MANUAL, 0.0, 1.0, 0.0));

   addInput(Port::create<PJ301MPort>(Vec(12, 43), Port::INPUT, module, Resonator::UPPER_VOCT));
   addInput(Port::create<PJ301MPort>(Vec(12, 120), Port::INPUT, module, Resonator::UPPER_GATE));

   addOutput(Port::create<PJ301MPort>(Vec(12, 159), Port::OUTPUT, module, Resonator::UPPER_OUT));
   addOutput(Port::create<PJ301MPort>(Vec(86, 159), Port::OUTPUT, module, Resonator::UPPER_BITS));

   //Lower
   addParam(ParamWidget::create<StochasmMintLargeKnob>(Vec(52, 218), module, Resonator::LOWER_CHAMBER, 0.0, 10.0, 5.0));
   addParam(ParamWidget::create<StochasmMintKnob>(Vec(48, 292), module, Resonator::LOWER_FILTER1, 0.0, 10.0, 5.0));
   addParam(ParamWidget::create<StochasmMintKnob>(Vec(85, 292), module, Resonator::LOWER_FILTER2, 0.0, 10.0, 5.0));
   addParam(ParamWidget::create<MintMomentarySwitch>(Vec(11, 269), module, Resonator::LOWER_MANUAL, 0.0, 1.0, 0.0));

   addInput(Port::create<PJ301MPort>(Vec(12, 230), Port::INPUT, module, Resonator::LOWER_VOCT));
   addInput(Port::create<PJ301MPort>(Vec(12, 308), Port::INPUT, module, Resonator::LOWER_GATE));

   addOutput(Port::create<PJ301MPort>(Vec(12, 346), Port::OUTPUT, module, Resonator::LOWER_OUT));
   addOutput(Port::create<PJ301MPort>(Vec(86, 346), Port::OUTPUT, module, Resonator::LOWER_BITS));

   //addInput(createInput<PJ301MPort>(Vec(33, 186), module, Resonator::PITCH_INPUT));

   //addOutput(createOutput<PJ301MPort>(Vec(33, 275), module, Resonator::SINE_OUTPUT));

   //addChild(createLight<MediumLight<RedLight>>(Vec(41, 59), module, Resonator::BLINK_LIGHT));
}

/*
  Upper:
  Chamber, LargeMintKnob -> (52, 30)
  Filter1, MintKnob -> (48, 104)
  Filter2, MintKnob -> (85, 104)
  1V/OCT, PJ301M -> (12, 43)
  Manual, MintMomentary -> (11, 81)
  Gate, PJ301M -> (12, 120)
  Out, PJ301M -> (12, 159)
  Bits, PJ301M -> (86, 159)

  Lower:
  Chamber, LargeMintKnob -> (52, 218)
  Filter1, MintKnob -> (48, 292)
  Filter2, MintKnob -> (85, 292)
  1V/OCT, PJ301M -> (12, 230)
  Manual, MintMomentary -> (11, 269)
  Gate, PJ301M -> (12, 308)
  Out, PJ301M -> (12, 346)
  Bits, PJ301M -> (86, 346)
*/

} // namespace rack_plugin_com_soundchasing_stochasm

using namespace rack_plugin_com_soundchasing_stochasm;

RACK_PLUGIN_MODEL_INIT(com_soundchasing_stochasm, Resonator) {
   Model *modelResonator = Model::create<Resonator, ResonatorWidget>("Stochasm", "Resonator", "Bitstream Resonator", OSCILLATOR_TAG, NOISE_TAG);
   return modelResonator;
}
