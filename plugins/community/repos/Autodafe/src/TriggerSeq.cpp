//**************************************************************************************
//Trigger Sequencer Module for VCV Rack by Autodafe http://www.autodafe.net
//
//Based on code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//
//**************************************************************************************

#include "Autodafe.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_Autodafe {

struct TriggerSeq : Module {
   enum ParamIds {

      CLOCK_PARAM,

      RUN_PARAM,
      RESET_PARAM,
      STEPS_PARAM,

      ROW_PARAM,
      COLUMN_PARAM = ROW_PARAM+8,

      GATE_PARAM=COLUMN_PARAM+16,
      NUM_PARAMS = GATE_PARAM+128
   };

   enum InputIds {
      CLOCK_INPUT,
      EXT_CLOCK_INPUT,
      START_INPUT, 
      STOP_INPUT,
      RESET_INPUT, 
      STEPS_INPUT,
      NUM_INPUTS
   };

   enum OutputIds {
      CLOCK_OUT,
      GATES_OUTPUT,

      NUM_OUTPUTS = GATES_OUTPUT + 8
   };

   enum LightIds {
      RUNNING_LIGHT,
      RESET_LIGHT,
      STEP_LIGHTS,
      
      GATE_LIGHTS=STEP_LIGHTS+16,

      GATES_LIGHTS=GATE_LIGHTS+8,
      NUM_LIGHTS=GATES_LIGHTS +128
   };


   bool running = true;
   SchmittTrigger clockTrigger; // for external clock
   SchmittTrigger runningTrigger;
   SchmittTrigger resetTrigger;


   SchmittTrigger rowTriggers[8];
   SchmittTrigger columnTriggers[16];
   bool FullRow[8];
   bool FullColumn[16];


   float phase = 0.0;
   int index = 0;
   SchmittTrigger gateTriggers[8][16];
   bool gateState[8][16]={};



   float stepLights[8][16] ;

   float stepLightsTop[16] ;
   // Lights
   float runningLight = 0.0;
   float resetLight = 0.0;
   float gatesLight[8];
   float rowLights[8][16] ;
   float gateLights[8][16];

   TriggerSeq()  : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
   void step();

   json_t *toJson() {

      json_t *rootJtrigseq = json_object();

      json_t *gatesJtrigSeq = json_array();
      for (int z = 0; z < 8; z++) {
         
         for (int i = 0; i < 16; i++) {
            json_t *gateJtrigseq = json_integer((int)gateState[z][i]);
            json_array_append_new(gatesJtrigSeq, gateJtrigseq);
         }
      }
      json_object_set_new(rootJtrigseq, "gatesTrigSeq", gatesJtrigSeq);

      return rootJtrigseq;
   }

   void fromJson(json_t *rootJtrigseq) {

//EMPTY EVERYTHING
      for (int z = 0; z < 8; z++) {
         
         for (int i = 0; i < 16; i++) {
            gateState[z][i] = false;
            
         }
      }


      //LOAD FROM FILE
      json_t *gatesJtrigSeq = json_object_get(rootJtrigseq, "gatesTrigSeq");
      
      for (int z = 0; z < 8; z++) {
         
         for (int i = 0; i < 16; i++) {
         

            json_t *gateJtrigseq = json_array_get(gatesJtrigSeq, z*16+i);
            gateState[z][i] = !!json_integer_value(gateJtrigseq);

         }
      }
   }

   void reset() {
      
      for (int z = 0; z < 8; z++) {
         for (int i = 0; i < 16; i++) {
         
            gateState[z][i] = false;
         }
      }
   }

   void randomize() {
      for (int z = 0; z < 8; z++) {
         for (int i = 0; i < 16; i++) {
         
            gateState[z][i] = rand()%2;
         }
      }
   }
};

void TriggerSeq::step() {
   
   float gate[8] = { 0 };
   
   const float lightLambda = 0.05f;

   outputs[CLOCK_OUT].value=0;
   
   // Run
   if (runningTrigger.process(params[RUN_PARAM].value)) {
      running = !running;
   }

   if(inputs[START_INPUT].value>0){running=true;}

   if(inputs[STOP_INPUT].value>0){running=false;}


   lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;

   bool nextStep = false;

   if (running) {
      if (inputs[EXT_CLOCK_INPUT].active) {
         // External clock
         if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) {
            phase = 0.0;
            nextStep = true;
            outputs[CLOCK_OUT].value=1;
         }
      }
      else {
         // Internal clock
         float clockTime = powf(2.0, params[CLOCK_PARAM].value+ inputs[CLOCK_INPUT].value);
         phase += clockTime / engineGetSampleRate();
         if (phase >= 1.0) {
            phase -= 1.0;
            nextStep = true;
            outputs[CLOCK_OUT].value=1;
         }
      }
   }

   // Reset
   if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) {
      phase = 0.0;
      index = 999;
      nextStep = true;
      resetLight = 1.0;
   }

   if (nextStep)  {

      // Advance step
      int numSteps = clampi(roundf(params[STEPS_PARAM].value + inputs[STEPS_INPUT].value), 1, 16);
      index += 1;
      if (index >= numSteps) {
         index = 0;
      }
         
      //for (int z = 0; z < 8; z++) {
      // stepLights[z][index] = 1.0;
      //}
   }
         
   resetLight -= resetLight / lightLambda / engineGetSampleRate();

   // Gate buttons

   for (int z = 0; z < 8; z++) {

      //ROW BUTTONS     
      if (rowTriggers[z].process(params[ROW_PARAM + z].value)) {
         FullRow[z]=!FullRow[z];
                  

         if (FullRow[z]){

            gateState[z][0] = 1;
            gateState[z][1] = 1;
            gateState[z][2] = 1;
            gateState[z][3] = 1;
            gateState[z][4] = 1;
            gateState[z][5] = 1;
            gateState[z][6] = 1;
            gateState[z][7] = 1;
            gateState[z][8] = 1;
            gateState[z][9] = 1;
            gateState[z][10] = 1;
            gateState[z][11] = 1;
            gateState[z][12] = 1;
            gateState[z][13] = 1;
            gateState[z][14] = 1;
            gateState[z][15] = 1;
         }
         else 
         {

            gateState[z][0] = 0;
            gateState[z][1] = 0;
            gateState[z][2] = 0;
            gateState[z][3] = 0;
            gateState[z][4] = 0;
            gateState[z][5] = 0;
            gateState[z][6] = 0;
            gateState[z][7] = 0;
            gateState[z][8] = 0;
            gateState[z][9] = 0;
            gateState[z][10] = 0;
            gateState[z][11] = 0;
            gateState[z][12] = 0;
            gateState[z][13] = 0;
            gateState[z][14] = 0;
            gateState[z][15] = 0;
         }
      }

      for (int i = 0; i < 16; i++) {

         //COLUMN BUTTONS     
         if (columnTriggers[i].process(params[COLUMN_PARAM + i].value)) {
            FullColumn[i]=!FullColumn[i];

            if (FullColumn[i]) {

               gateState[0][i] = 1;
               gateState[1][i] = 1;
               gateState[2][i] = 1;
               gateState[3][i] = 1;
               gateState[4][i] = 1;
               gateState[5][i] = 1;
               gateState[6][i] = 1;
               gateState[7][i] = 1;
            }
            else
            {
               gateState[0][i] = 0;
               gateState[1][i] = 0;
               gateState[2][i] = 0;
               gateState[3][i] = 0;
               gateState[4][i] = 0;
               gateState[5][i] = 0;
               gateState[6][i] = 0;
               gateState[7][i] = 0;
            }
                 
         }  

         gate[z] = (gateState[z][index] >= 1.0) && !nextStep ? 10.0 : 0.0;
         outputs[GATES_OUTPUT + z].value= gate[z];

         lights[GATES_LIGHTS +z*16+i].value = (gateState[z][i] >= 1.0) ? 1.0 : 0.0;

         if (gateTriggers[z][i].process(params[GATE_PARAM + z*16+i].value)) {
            gateState[z][i] = !gateState[z][i];
      
         }
      } 

      lights[RESET_LIGHT].value = resetLight;
      lights[GATE_LIGHTS + z].value  = (gateState[z][index] >= 1.0) ? 1.0 : 0.0;
      
      for (int y=0; y<16; y++){lights[STEP_LIGHTS + y].value=0;}

      lights[STEP_LIGHTS + index].value  = 1.0;
   }
   
}

struct AutodafePurpleLight : ModuleLightWidget {
   AutodafePurpleLight() {
      addBaseColor(nvgRGB(0x89, 0x13, 0xC4));
   }
};
     
struct TriggerSeqWidget : ModuleWidget {
	TriggerSeqWidget(TriggerSeq *module);
};
    
TriggerSeqWidget::TriggerSeqWidget(TriggerSeq *module) : ModuleWidget(module) { 
   box.size = Vec(15*37, 380);
 
   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      

      panel->setBackground(SVG::load(assetPlugin(plugin, "res/TriggerSeq.svg")));
      addChild(panel);
   }

   addChild(createScrew<ScrewSilver>(Vec(5, 0)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x-20, 0)));
   addChild(createScrew<ScrewSilver>(Vec(5, 365)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x-20, 365)));

   addParam(createParam<AutodafeKnobPurpleSmall>(Vec(17, 56), module, TriggerSeq::CLOCK_PARAM, -2.0, 6.0, 2.0));
   addParam(createParam<LEDButton>(Vec(60, 61-1), module, TriggerSeq::RUN_PARAM, 0.0, 1.0, 0.0));


   //addChild(createValueLight<SmallLight<GreenValueLight>>(Vec(60+5, 61+4), &module->runningLight));
   addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(64.4, 64.4), module, TriggerSeq::RUNNING_LIGHT));

   addInput(createInput<PJ301MPort>(Vec(173, 98), module, TriggerSeq::START_INPUT));
   addInput(createInput<PJ301MPort>(Vec(211, 98), module, TriggerSeq::STOP_INPUT));

   addOutput(createOutput<PJ301MPort>(Vec(250, 98), module, TriggerSeq::CLOCK_OUT));

   addParam(createParam<LEDButton>(Vec(98, 61-1), module, TriggerSeq::RESET_PARAM, 0.0, 1.0, 0.0));

   //addChild(createValueLight<SmallLight<GreenValueLight>>(Vec(98+5, 61+4), &module->resetLight));
   addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(103.4, 64.4), module, TriggerSeq::RESET_LIGHT));

   addParam(createParam<AutodafeKnobPurple>(Vec(128, 52), module, TriggerSeq::STEPS_PARAM, 1.0, 16.0, 16.0));

   static const float portX[16] = { 19, 57, 96, 134, 173, 211, 250, 288, 326, 364, 402, 440,478,516,554,592};
   static const float portX2[16] = { 19, 49, 79, 109, 139, 169, 199, 229, 259, 289, 319, 349,379,409,439,469};

   //static const float portX[16] = { 20, 50, 80, 110, 140, 170, 200, 230, 260, 290, 320, 350,380,410,440,470};
   addInput(createInput<PJ301MPort>(Vec(portX[0]-1, 99-1), module, TriggerSeq::CLOCK_INPUT));
   addInput(createInput<PJ301MPort>(Vec(portX[1]-1, 99-1), module, TriggerSeq::EXT_CLOCK_INPUT));
   addInput(createInput<PJ301MPort>(Vec(portX[2]-1, 99-1), module, TriggerSeq::RESET_INPUT));
   addInput(createInput<PJ301MPort>(Vec(portX[3]-1, 99-1), module, TriggerSeq::STEPS_INPUT));
   
   for (int k=0;k<16;k++)
   {
      addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(portX2[k] + 6.4, 125), module, TriggerSeq::STEP_LIGHTS + k));
   }

   for (int z = 0; z < 8; z++) 
   {
      //Gates Oututs
      addOutput(createOutput<PJ301MPort>(Vec(510, 140 + 25 * z - 5), module, TriggerSeq::GATES_OUTPUT+z));
      addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(495, 143 + 25 * z), module, TriggerSeq::GATE_LIGHTS+z));

      addParam(createParam<BtnTrigSequencerSmall>(Vec(4, 140 + 25 * z +2), module, TriggerSeq::ROW_PARAM + z, 0.0, 1.0, 0.0));
      
      for (int i = 0; i < 16; i++) {
         //Lighst and Button Matrix
         addParam(createParam<BtnTrigSequencer>(Vec(portX2[i] + 2, 140  + 25  * z - 1), module, TriggerSeq::GATE_PARAM + z*16+i, 0.0, 1.0, 0.0));
        
         addParam(createParam<BtnTrigSequencerSmall>(Vec(portX2[i] + 5, 140  + 25  * 8-4.0), module, TriggerSeq::COLUMN_PARAM + i, 0.0, 1.0, 0.0));

         //addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(portX[i] + 2, 140  + 25  * z - 1), module, SEQ16::GATE_LIGHTS + z*16+i));

         //addParam(createParam<AutodafeButton>(Vec(portX[i] + 2, 140  + 25  * z - 1), module, TriggerSeq::GATE_PARAM + z*16+i, 0.0, 1.0, 0.0));
        
         //addChild(createValueLight<SmallLight<GreenValueLight>>(Vec(portX[i] + 7, 140 + 25 * z + 4), &module->gateLights[z][i]));
         addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(portX2[i] + 6.4, 140  + 25  * z+3.0), module, TriggerSeq::GATES_LIGHTS + z*16+i));

      }
   }

}

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

RACK_PLUGIN_MODEL_INIT(Autodafe, TriggerSeq) {
   return Model::create<TriggerSeq, TriggerSeqWidget>("Autodafe","8x16 Trigger Sequencer", "8x16 Trigger Sequencer", SEQUENCER_TAG);
}
