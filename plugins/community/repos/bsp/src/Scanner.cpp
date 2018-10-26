/*
Copyright (c) 2018 bsp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <math.h>
#include <stdlib.h> // memset

#include "bsp.hpp"

namespace rack_plugin_bsp {

typedef union fi_u {
   float f;
   unsigned int u;
   int s;
} fi_t;

// struct TrigButton : CKD6 {
// struct TrigButton : TL1105 {
struct TrigButton : LEDButton {
};

struct NullButton : SVGSwitch, ToggleSwitch {
	NullButton() {
		addFrame(SVG::load(assetPlugin("res/null.svg")));
		addFrame(SVG::load(assetPlugin("res/null.svg")));
   }
};

struct Scanner : Module {

   static const uint32_t MAX_INPUTS = 16u;

   enum ParamIds {
		POSITION_PARAM,
      MOD_POSITION_AMOUNT_PARAM,
      SHAPE_PARAM,  // sin..tri..square
      WIDTH_PARAM,
      TABLE_TYPE_PARAM,
      OUT_WINDOW_SHAPE_PARAM,
      OUT_WINDOW_OFFSET_SWITCH_PARAM,
      RANDOM_TRIG_PARAM,
      RANDOM_ENABLE_PARAM,
      RANDOM_SEED_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
      MIX_1_INPUT,
      MIX_2_INPUT,
      MIX_3_INPUT,
      MIX_4_INPUT,
      MIX_5_INPUT,
      MIX_6_INPUT,
      MIX_7_INPUT,
      MIX_8_INPUT,
      MIX_9_INPUT,
      MIX_10_INPUT,
      MIX_11_INPUT,
      MIX_12_INPUT,
      MIX_13_INPUT,
      MIX_14_INPUT,
      MIX_15_INPUT,
      MIX_16_INPUT,
      MOD_POSITION_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		MIX_OUTPUT,
		WIN_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
      MIX_1_LIGHT,
      MIX_2_LIGHT,
      MIX_3_LIGHT,
      MIX_4_LIGHT,
      MIX_5_LIGHT,
      MIX_6_LIGHT,
      MIX_7_LIGHT,
      MIX_8_LIGHT,
      MIX_9_LIGHT,
      MIX_10_LIGHT,
      MIX_11_LIGHT,
      MIX_12_LIGHT,
      MIX_13_LIGHT,
      MIX_14_LIGHT,
      MIX_15_LIGHT,
      MIX_16_LIGHT,
		NUM_LIGHTS
	};

#define MIX_LUT_SIZE (4096)
   // (note) the table is actually symmetric (center = LUT_SIZE/2)
   float mix_lut[MIX_LUT_SIZE + 1];

   float last_mix_shape;
   float last_out_shape;

   static const uint32_t OUT_BUFFER_SIZE = 32u;
   static const uint32_t OUT_BUFFER_MASK = (OUT_BUFFER_SIZE - 1u);
   float out_lut[OUT_BUFFER_SIZE + 1];
   float out_buffer[OUT_BUFFER_SIZE];
   // (note) the table is actually symmetric (center = LUT_SIZE/2)
   uint32_t out_buffer_idx;

   uint32_t input_shuffle_lut[MAX_INPUTS];
   fi_t     last_input_shuffle_seed;
   fi_t     tmp_seed;
   float    last_rand_enable;
   uint32_t last_num_active_inputs;


	Scanner() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
      last_mix_shape = -999.f;
      last_out_shape = -999.f;
      tmp_seed.u = 0u;
      last_num_active_inputs  = 0u;
      last_rand_enable = -999.f;
      memset((void*)out_buffer, 0, sizeof(out_buffer));
      out_buffer_idx = 0u;
   }

   void calcLUT (float *_lut, const uint32_t _lutSize, const float _shape);
   void calcMixLUT (void);
   void calcOutLUT (void);

   uint32_t fastRand (void);
   void calcInputShuffleLUT (uint32_t _numActiveInputs);

	void step() override;
};

void Scanner::calcLUT(float *_lut, const uint32_t _lutSize, const float _shape) {

   // printf("xxx Scanner::calcMixLUT: shape=%f\n", _shape);

   float x = 0.0f;
   float stepX = (1.0f / _lutSize);

   for(uint32_t i = 0u ; i < _lutSize; i++)
   {
      float cy = 0.0f;
      
      float triY = x*2;
      if(triY > 1.0f)
         triY = (2.0f - triY);

      float expY = sinf(x*3.14159265359f/*PI*/);
      expY = powf(expY, powf(1.0f - _shape, 9.0f)*64 + 1.0f);

      float rectY = triY;
      // rectY = (triY < 0.5) ? 0.0f : 1.0f;
      rectY = powf(triY * 2.0f, powf((_shape - 0.75f)*4, 20) * 400.0f + 1.0f);
      if(rectY > 1.0f)
         rectY = 1.0f;

      if(_shape < 0.5f)
      {
         cy = expY;
      }
      else if( (_shape >= 0.5f) && (_shape < 0.75f))
      {
         float t = (_shape -0.5f) * 4.0f;
         cy = expY + (triY - expY) * t;
      }
      else if(_shape >= 0.75f)
      {
         float t = (_shape - 0.75f) * 4.0f;
         cy = triY + (rectY - triY) * t;
      }

      // printf("xxx mix_lut[%d] = %f triY=%f\n", i, cy, triY);

      _lut[i] = cy;

      x += stepX;
   }

   _lut[_lutSize] = _lut[0];
}

void Scanner::calcMixLUT(void) {
   calcLUT(mix_lut, MIX_LUT_SIZE, last_mix_shape);
}

void Scanner::calcOutLUT(void) {
   calcLUT(out_lut, OUT_BUFFER_SIZE, last_out_shape);

   float sum = 0.0f;
   for(uint32_t i = 0u; i < OUT_BUFFER_SIZE; i++)
      sum += out_lut[i];
   float scl = 1.0f / sum;
   for(uint32_t i = 0u; i < OUT_BUFFER_SIZE; i++)
      out_lut[i] *= scl;
}

uint32_t Scanner::fastRand(void) {
   tmp_seed.u *= 16807u;
   printf("xxx fastRand()=%u\n", tmp_seed.u);
   return tmp_seed.u >> 10;
}

void Scanner::calcInputShuffleLUT(uint32_t _numActiveInputs) {

   printf("xxx Scanner::calcInputShuffleLUT(numActiveInputs=%u)\n", _numActiveInputs);

   tmp_seed.f = params[RANDOM_SEED_PARAM].value;
   tmp_seed.u &= 0xFFffFFu;
   tmp_seed.u += (~tmp_seed.u) & 1u;

   if(params[RANDOM_ENABLE_PARAM].value >= 0.5f)
   {
      for(uint32_t i = 0u; i < _numActiveInputs; i++)
      {
         // (note) there are other "random" functions that produce non-repeating number sequences
         //         but this one is good enough (usually <8 iterations to generate 4 unique random values)
         bool bDuplicate;

         do
         {
            input_shuffle_lut[i] = fastRand() % _numActiveInputs;

            bDuplicate = false;

            for(uint32_t j = 0u; j < i; j++)
            {
               if(input_shuffle_lut[j] == input_shuffle_lut[i])
               {
                  bDuplicate = true;
                  break;
               }
            }
         }
         while(bDuplicate);

      }
   }
   else
   {
      for(uint32_t i = 0u; i < _numActiveInputs; i++)
      {
         input_shuffle_lut[i] = i;
      }
   }

}


void Scanner::step() {

   if(params[SHAPE_PARAM].value != last_mix_shape)
   {
      last_mix_shape = params[SHAPE_PARAM].value;
      calcMixLUT();
   }

   uint32_t numInputs = 0u;
   int inputIdx[16];
   float outWeights[16];
   for(int i = 0; i < 16; i++)
   {
      if(inputs[MIX_1_INPUT + i].active)
      {
         inputIdx[numInputs] = i;
         outWeights[numInputs] = 0.0f;
         numInputs++;
      }
      else
      {
         lights[MIX_1_LIGHT + i].setBrightnessSmooth(0.0f);
      }
   }

   if(params[RANDOM_TRIG_PARAM].value >= 0.5f)
   {
      // (todo) don't handle UI button in the audio thread
      params[RANDOM_TRIG_PARAM].value = 0.0f;
      fi_t r; r.s = rand();
      params[RANDOM_SEED_PARAM].value = r.f;
   }

   if((last_num_active_inputs != numInputs) || 
      (last_input_shuffle_seed.f != params[RANDOM_SEED_PARAM].value) ||
      (last_rand_enable != params[RANDOM_ENABLE_PARAM].value)
      )
   {
      last_num_active_inputs = numInputs;
      last_input_shuffle_seed.f = params[RANDOM_SEED_PARAM].value;
      last_rand_enable = params[RANDOM_ENABLE_PARAM].value;
      calcInputShuffleLUT(numInputs);
   }

   float mixOut = 0.0f;

   static int xxx = 0;

   float width = 1.0f + (1.0f - params[WIDTH_PARAM].value) * 3.0f;
   
   if(numInputs > 0)
   {
      float pos = -params[POSITION_PARAM].value + 0.5f - (inputs[MOD_POSITION_INPUT].value * (1.0f/5.0f))*params[MOD_POSITION_AMOUNT_PARAM].value;
      float posStep = 1.0f / numInputs;
      // float xStep = float(MIX_LUT_SIZE) / numInputs;
      float outWSum = 0.0f;

      for(uint32_t i = 0u; i < numInputs; i++)
      {
         if(pos < 0.0f)
            pos += 1.0f;
         else if(pos >= 1.0f)
            pos -= 1.0f;

         float w;

         float posF = ((pos - 0.5f) * width);

         if((posF > -0.5f) && (posF < 0.5f))
         {
            posF += 0.5f;
            posF *= MIX_LUT_SIZE;
            int posI = int(posF);
            float posFrac = posF - posI;
            w = mix_lut[posI] + (mix_lut[posI + 1] - mix_lut[posI]) * posFrac;
         }
         else
         {
            w = 0.0f;
         }

#if 0
         if(0 == (xxx & 32767))
         {
            printf("xxx i=%d pos=%f w=%f posStep=%f\n", i, pos, w, posStep);
         }
#endif

         outWeights[i] = w;
         outWSum += w;

         pos += posStep;
      }

      float outWScale = (outWSum > 0.0f) ? (1.0f / outWSum) : 0.0f;

      for(uint32_t i = 0u; i < numInputs; i++)
      {
         int portIdx = inputIdx[input_shuffle_lut[i]];

         lights[MIX_1_LIGHT + portIdx].setBrightnessSmooth(outWeights[i]);

         mixOut += inputs[MIX_1_INPUT + portIdx].value * outWeights[i] * outWScale;
      }
      
   }

   float winOut = 0.0f;

   if(outputs[WIN_OUTPUT].active)
   {
      if(params[OUT_WINDOW_SHAPE_PARAM].value != last_out_shape)
      {
         last_out_shape = params[OUT_WINDOW_SHAPE_PARAM].value;
         calcOutLUT();
      }

      out_buffer[out_buffer_idx] = mixOut;

      uint32_t j;

      bool bOffsetSw = (params[Scanner::OUT_WINDOW_OFFSET_SWITCH_PARAM].value >= 0.5f);

      if(bOffsetSw)
      {
         // (note) this used to be a bug in the first implementation but it turned out
         //         that this produces some nice hihat / cymbal sounds so I left it
         //         in as a switchable parameter
         j = (out_buffer_idx - (OUT_BUFFER_SIZE>>1)) & OUT_BUFFER_MASK;

         for(uint32_t i = 0u; i < OUT_BUFFER_SIZE; i++)
         {
            winOut += out_buffer[j] * out_lut[i];

            j = (j + 1u) & OUT_BUFFER_MASK;
         }
      }
      else
      {
         j = out_buffer_idx;

         for(uint32_t i = 0u; i < OUT_BUFFER_SIZE; i++)
         {
            j = (j - 1u) & OUT_BUFFER_MASK;

            winOut += out_buffer[j] * out_lut[i];
         }
      }

      out_buffer_idx = (out_buffer_idx + 1u) & OUT_BUFFER_MASK;
   }
  

#if 1
   if(0 == (++xxx & 32767))
   {
      // printf("xxx numInputs=%d\n", numInputs);
      // printf("xxx mix_lut[2048]=%f\n", mix_lut[MIX_LUT_SIZE/2]);
      // printf("xxx outWeights=[%f; %f; %f; %f]\n", outWeights[0], outWeights[1], outWeights[2], outWeights[3]);
      
   }
#endif

	outputs[MIX_OUTPUT].value = mixOut;
	outputs[WIN_OUTPUT].value = winOut;
}


struct ScannerWidget : ModuleWidget {
	ScannerWidget(Scanner *module);
};

ScannerWidget::ScannerWidget(Scanner *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Scanner.svg")));

   addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
   addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
   addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

   // Audio input ports
   {
      float cy = 20.0f;
      int k = 0;
      for(int y = 0; y < 4; y++)
      {
         float cx = 3.6f;
         for(int x = 0; x < 4; x++)
         {
            addInput(Port::create<PJ301MPort>(mm2px(Vec(cx, cy)), Port::INPUT, module, Scanner::MIX_1_INPUT + k));
            addChild(ModuleLightWidget::create<SmallLight<RedLight>>(mm2px(Vec(cx+3.1f, cy-3.5f)), module, Scanner::MIX_1_LIGHT + k));
            cx += 12.0f;
            k++;
         }
         cy += 15.0f;
      }
   }

#define STX 35
#define STY 40
   float cx = 9.0f;
   float cy = 248.0f;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Scanner::POSITION_PARAM, -1.0f, 1.0f, 0.0f));
   cx += STX;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Scanner::MOD_POSITION_AMOUNT_PARAM, -1.0f, 1.0f, 0.0f));
   addInput(Port::create<PJ301MPort>(Vec(cx+2.3f, cy + 37.0f), Port::INPUT, module, Scanner::MOD_POSITION_INPUT));
   cx += STX;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Scanner::SHAPE_PARAM, 0.0f, 1.0f, 0.45f));
   cx += STX;
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(cx, cy), module, Scanner::WIDTH_PARAM, 0.0f, 1.0f, 1.0f));

	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(27, box.size.y - 60), module, Scanner::OUT_WINDOW_SHAPE_PARAM, 0.0f, 1.0f, 0.34f));

	addParam(ParamWidget::create<CKSS>(Vec(9, box.size.y-58), module, Scanner::OUT_WINDOW_OFFSET_SWITCH_PARAM, 0.0f, 1.0f, 0.0f));

   cy = 286.0f;
   addParam(ParamWidget::create<TrigButton>(Vec(box.size.x - 45, cy+2.0f), module, Scanner::RANDOM_TRIG_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<CKSS>(Vec(box.size.x - 25, cy), module, Scanner::RANDOM_ENABLE_PARAM, 0.0f, 1.0f, 0.0f));
   addParam(ParamWidget::create<NullButton>(Vec(box.size.x - 70, cy-30), module, Scanner::RANDOM_SEED_PARAM, -INFINITY, INFINITY, 0.0f));

	addOutput(Port::create<PJ301MPort>(Vec(box.size.x - 40, 320), Port::OUTPUT, module, Scanner::MIX_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(box.size.x - 90, 320), Port::OUTPUT, module, Scanner::WIN_OUTPUT));
}

} // namespace rack_plugin_bsp

using namespace rack_plugin_bsp;

RACK_PLUGIN_MODEL_INIT(bsp, Scanner) {
   Model *modelScanner = Model::create<Scanner, ScannerWidget>("bsp", "Scanner", "Scanner", MIXER_TAG);
   return modelScanner;
}
