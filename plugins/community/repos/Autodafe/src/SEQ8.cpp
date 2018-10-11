#include "Autodafe.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_Autodafe {

struct SEQ8 : Module {
   enum ParamIds {
      CLOCK_PARAM,
      RUN_PARAM,
      RESET_PARAM,
      STEPS_PARAM,
      ROW1_PARAM,
      ROW2_PARAM = ROW1_PARAM + 8,
      ROW3_PARAM = ROW2_PARAM + 8,
      GATE_PARAM = ROW3_PARAM + 8,
      NUM_PARAMS = GATE_PARAM + 8
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
      CLOCK_GATE_OUT,
      GATES_OUTPUT,
      ROW1_OUTPUT,
      ROW2_OUTPUT,
      ROW3_OUTPUT,
      GATE_OUTPUT,
      NUM_OUTPUTS = GATE_OUTPUT + 8
   };
   enum LightIds {
      RUNNING_LIGHT,
      RESET_LIGHT,
      GATES_LIGHT,
      ROW_LIGHTS,
      GATE_LIGHTS = ROW_LIGHTS + 3,
      NUM_LIGHTS = GATE_LIGHTS + 8
   };

   bool running = true;
   SchmittTrigger clockTrigger; // for external clock
   // For buttons
   SchmittTrigger runningTrigger;
   SchmittTrigger resetTrigger;
   SchmittTrigger gateTriggers[8];
   float phase = 0.0;
   int index = 0;
   bool gateState[8] = {};
   float resetLight = 0.0;
   float stepLights[8] = {};

   enum GateMode {
      TRIGGER,
      RETRIGGER,
      CONTINUOUS,
   };
   GateMode gateMode = TRIGGER;
   PulseGenerator gatePulse;

   SEQ8() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
   void step() override;

   json_t *toJson() override {
      json_t *rootJ = json_object();

      // running
      json_object_set_new(rootJ, "running", json_boolean(running));

      // gates
      json_t *gatesJ = json_array();
      for (int i = 0; i < 8; i++) {
         json_t *gateJ = json_integer((int) gateState[i]);
         json_array_append_new(gatesJ, gateJ);
      }
      json_object_set_new(rootJ, "gates", gatesJ);

      // gateMode
      json_t *gateModeJ = json_integer((int) gateMode);
      json_object_set_new(rootJ, "gateMode", gateModeJ);

      return rootJ;
   }

   void fromJson(json_t *rootJ) override {
      // running
      json_t *runningJ = json_object_get(rootJ, "running");
      if (runningJ)
         running = json_is_true(runningJ);

      // gates
      json_t *gatesJ = json_object_get(rootJ, "gates");
      if (gatesJ) {
         for (int i = 0; i < 8; i++) {
            json_t *gateJ = json_array_get(gatesJ, i);
            if (gateJ)
               gateState[i] = !!json_integer_value(gateJ);
         }
      }

      // gateMode
      json_t *gateModeJ = json_object_get(rootJ, "gateMode");
      if (gateModeJ)
         gateMode = (GateMode)json_integer_value(gateModeJ);
   }

   void reset() override {
      for (int i = 0; i < 8; i++) {
         gateState[i] = false;
      }
   }

   void randomize() override {
      for (int i = 0; i < 8; i++) {
         gateState[i] = (randomf() > 0.5);
      }
   }
};


void SEQ8::step() {
   const float lightLambda = 0.075;


   outputs[CLOCK_OUT].value=0;

   outputs[CLOCK_GATE_OUT].value=0;

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
         float clockTime = powf(2.0, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
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
      index = 8;
      nextStep = true;
      resetLight = 1.0;
   }

   if (nextStep) {
      // Advance step
      int numSteps = clampi(roundf(params[STEPS_PARAM].value + inputs[STEPS_INPUT].value), 1, 8);
      index += 1;
      if (index >= numSteps) {
         index = 0;
      }
      stepLights[index] = 1.0;
      gatePulse.trigger(1e-3);

   }

   resetLight -= resetLight / lightLambda / engineGetSampleRate();

   bool pulse = gatePulse.process(1.0 / engineGetSampleRate());

   // Gate buttons
   for (int i = 0; i < 8; i++) {
      if (gateTriggers[i].process(params[GATE_PARAM + i].value)) {
         gateState[i] = !gateState[i];
      }
      bool gateOn = (running && i == index && gateState[i]);
      if (gateMode == TRIGGER)
         gateOn = gateOn && pulse;
      else if (gateMode == RETRIGGER)
         gateOn = gateOn && !pulse;

      outputs[GATE_OUTPUT + i].value = gateOn ? 10.0 : 0.0;

      if (outputs[GATE_OUTPUT + i].value!=0)
      {
         outputs[CLOCK_GATE_OUT].value=gateOn ? 1.0 : 0.0;
      }

      stepLights[i] -= stepLights[i] / lightLambda / engineGetSampleRate();
      lights[GATE_LIGHTS + i].value = gateState[i] ? 1.0 - stepLights[i] : stepLights[i];
   }

   // Rows
   float row1 = params[ROW1_PARAM + index].value;
   float row2 = params[ROW2_PARAM + index].value;
   float row3 = params[ROW3_PARAM + index].value;
   bool gatesOn = (running && gateState[index]);
   if (gateMode == TRIGGER)
      gatesOn = gatesOn && pulse;
   else if (gateMode == RETRIGGER)
      gatesOn = gatesOn && !pulse;

   // Outputs
   outputs[ROW1_OUTPUT].value = row1;
   outputs[ROW2_OUTPUT].value = row2;
   outputs[ROW3_OUTPUT].value = row3;
   outputs[GATES_OUTPUT].value = gatesOn ? 10.0 : 0.0;
   lights[RESET_LIGHT].value = resetLight;
   lights[GATES_LIGHT].value = gatesOn ? 1.0 : 0.0;
   lights[ROW_LIGHTS].value = row1 / 10.0;
   lights[ROW_LIGHTS + 1].value = row2 / 10.0;
   lights[ROW_LIGHTS + 2].value = row3 / 10.0;
}

struct AutodafePurpleLight : ModuleLightWidget {
   AutodafePurpleLight() {
      addBaseColor(nvgRGB(0x89, 0x13, 0xC4));
   }
};

struct SEQ8Widget : ModuleWidget{
	SEQ8Widget(SEQ8 *module);
	Menu *createContextMenu() override;
};

SEQ8Widget::SEQ8Widget(SEQ8 *module) : ModuleWidget(module) {
   box.size = Vec(15*27, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/SEQ8.svg")));
      addChild(panel);
   }

   addChild(createScrew<ScrewSilver>(Vec(15, 0)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
   addChild(createScrew<ScrewSilver>(Vec(15, 365)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

   addParam(createParam<AutodafeKnobPurpleSmall>(Vec(18, 56), module, SEQ8::CLOCK_PARAM, -2.0, 6.0, 2.0));
   addParam(createParam<LEDButton>(Vec(60, 61-1), module, SEQ8::RUN_PARAM, 0.0, 1.0, 0.0));
   addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(64.4, 64.4), module, SEQ8::RUNNING_LIGHT));
   addParam(createParam<LEDButton>(Vec(99, 61-1), module, SEQ8::RESET_PARAM, 0.0, 1.0, 0.0));
   addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(103.4, 64.4), module, SEQ8::RESET_LIGHT));


   addParam(createParam<AutodafeKnobPurple>(Vec(128, 52), module, SEQ8::STEPS_PARAM, 1.0, 8.0, 8.0));


   addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(179.4, 64.4), module, SEQ8::GATES_LIGHT));
   addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(218.4, 64.4), module, SEQ8::ROW_LIGHTS));
   addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(256.4, 64.4), module, SEQ8::ROW_LIGHTS + 1));
   addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(295.4, 64.4), module, SEQ8::ROW_LIGHTS + 2));

   static const float portX[8] = {20, 58, 96, 135, 173, 212, 250, 289};
   addInput(createInput<PJ301MPort>(Vec(portX[0]-1, 98), module, SEQ8::CLOCK_INPUT));
   addInput(createInput<PJ301MPort>(Vec(portX[1]-1, 98), module, SEQ8::EXT_CLOCK_INPUT));
   addInput(createInput<PJ301MPort>(Vec(portX[2]-1, 98), module, SEQ8::RESET_INPUT));
   addInput(createInput<PJ301MPort>(Vec(portX[3]-1, 98), module, SEQ8::STEPS_INPUT));
   addOutput(createOutput<PJ301MPort>(Vec(portX[4]-1, 98), module, SEQ8::GATES_OUTPUT));
   addOutput(createOutput<PJ301MPort>(Vec(portX[5]-1, 98), module, SEQ8::ROW1_OUTPUT));
   addOutput(createOutput<PJ301MPort>(Vec(portX[6]-1, 98), module, SEQ8::ROW2_OUTPUT));
   addOutput(createOutput<PJ301MPort>(Vec(portX[7]-1, 98), module, SEQ8::ROW3_OUTPUT));

   addOutput(createOutput<PJ301MPort>(Vec(327, 307), module, SEQ8::CLOCK_OUT));
   addOutput(createOutput<PJ301MPort>(Vec(365, 307), module, SEQ8::CLOCK_GATE_OUT));


   addInput(createInput<PJ301MPort>(Vec(327, 98), module, SEQ8::START_INPUT));
   addInput(createInput<PJ301MPort>(Vec(365, 98), module, SEQ8::STOP_INPUT));

   for (int i = 0; i < 8; i++) {
      addParam(createParam<AutodafeKnobPurpleSmall>(Vec(portX[i]-2, 157), module, SEQ8::ROW1_PARAM + i, 0.0, 10.0, 0.0));
      addParam(createParam<AutodafeKnobPurpleSmall>(Vec(portX[i]-2, 198), module, SEQ8::ROW2_PARAM + i, 0.0, 10.0, 0.0));
      addParam(createParam<AutodafeKnobPurpleSmall>(Vec(portX[i]-2, 240), module, SEQ8::ROW3_PARAM + i, 0.0, 10.0, 0.0));
      addParam(createParam<LEDButton>(Vec(portX[i]+2, 278-1), module, SEQ8::GATE_PARAM + i, 0.0, 1.0, 0.0));
      addChild(createLight<MediumLight<AutodafePurpleLight>>(Vec(portX[i]+6.4, 281.4), module, SEQ8::GATE_LIGHTS + i));
      addOutput(createOutput<PJ301MPort>(Vec(portX[i]-1, 307), module, SEQ8::GATE_OUTPUT + i));
   }
}

struct SEQ8GateModeItem : MenuItem {
   struct SEQ8 *SEQ8;
   SEQ8::GateMode gateMode;
   void onAction(EventAction &e) override {
      SEQ8->gateMode = gateMode;
   }
   void step() override {
      rightText = (SEQ8->gateMode == gateMode) ? "✔" : "";
   }
};

Menu *SEQ8Widget::createContextMenu() {
   Menu *menu = ModuleWidget::createContextMenu();

   MenuLabel *spacerLabel = new MenuLabel();
   menu->pushChild(spacerLabel);

   SEQ8 *SEQ8 = dynamic_cast<struct SEQ8*>(module);
   assert(SEQ8);

   MenuLabel *modeLabel = new MenuLabel();
   modeLabel->text = "Gate Mode";
   menu->pushChild(modeLabel);

   SEQ8GateModeItem *triggerItem = new SEQ8GateModeItem();
   triggerItem->text = "Trigger";
   triggerItem->SEQ8 = SEQ8;
   triggerItem->gateMode = SEQ8::TRIGGER;
   menu->pushChild(triggerItem);

   SEQ8GateModeItem *retriggerItem = new SEQ8GateModeItem();
   retriggerItem->text = "Retrigger";
   retriggerItem->SEQ8 = SEQ8;
   retriggerItem->gateMode = SEQ8::RETRIGGER;
   menu->pushChild(retriggerItem);

   SEQ8GateModeItem *continuousItem = new SEQ8GateModeItem();
   continuousItem->text = "Continuous";
   continuousItem->SEQ8 = SEQ8;
   continuousItem->gateMode = SEQ8::CONTINUOUS;
   menu->pushChild(continuousItem);

   return menu;
}

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

RACK_PLUGIN_MODEL_INIT(Autodafe, SEQ8) {	
   return Model::create<SEQ8, SEQ8Widget>("Autodafe",  "8-Step Sequencer", "8-Step Sequencer", SEQUENCER_TAG);
}
