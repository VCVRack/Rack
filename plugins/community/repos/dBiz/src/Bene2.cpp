///////////////////////////
//  Bene2 - Big thx to Strum Mental and JW jeremywen for sharing their magic code !!
//  still some fix to do as usuall ;)
//
///////////////////////

#include "dBiz.hpp"
#include "dsp/digital.hpp"
 
using namespace std;

namespace rack_plugin_dBiz {
 
struct Bene2 : Module {
  enum ParamIds
  {
    RESET_LINE,
    RESET_COL,
    RUNL_PARAM,
    RUNC_PARAM,
    GATE_PARAM,
    KNOB_PARAM=GATE_PARAM+16,
    NUM_PARAMS = KNOB_PARAM + 16
  };
  enum InputIds
  {
    RESETL_INPUT,
    RESETC_INPUT,
    RUNC_INPUT,
    RUNL_INPUT,
    RESETL,
    RESETC = RESETL + 4,
    UP = RESETC + 4,
    DOWN = UP + 4,
    LEFT = DOWN + 4,
    RIGHT = LEFT + 4,
    NUM_INPUTS = RIGHT + 4
  };
  enum OutputIds
  {
    GATES_ROW_OUT,
    GATES_COL_OUT = GATES_ROW_OUT + 4,
    ROW_OUT = GATES_COL_OUT + 4,
    COLUMN_OUT = ROW_OUT + 4,
    NUM_OUTPUTS = COLUMN_OUT + 4
  };

  enum LightIds
  {
    RESETL_LIGHT,
    RESETC_LIGHT,
    RUNL_LIGHT,
    RUNC_LIGHT,
    GATE_LIGHT,
    STEPS_LIGHT = GATE_LIGHT + 16,
    NUM_LIGHTS = STEPS_LIGHT + 16
  };

  SchmittTrigger leftTrigger[4]={};
  SchmittTrigger rightTrigger[4]={};
  SchmittTrigger upTrigger[4]={};
  SchmittTrigger downTrigger[4]={};
  SchmittTrigger resetCoTrigger[4]={};
  SchmittTrigger resetLiTrigger[4]={};

  SchmittTrigger resetALTrigger;
  SchmittTrigger resetACTrigger;

  SchmittTrigger runningLTrigger;
  SchmittTrigger runningCTrigger;

  SchmittTrigger gateTriggers[16]={};
  SchmittTrigger button_triggers[4][4];                
    
  float row_outs[4] = {0.0,0.0,0.0,0.0};
  float column_outs[4] = {0.0,0.0,0.0,0.0};
  
  int posX[4] = {};
  int posY[4] = {};
  int index[16]={};
  int indexl[4] = {};

  bool gateState[16] = {};
  bool runningL = true;
  bool runningC = true;
  bool ignoreGateOnPitchOut = false;

  enum GateMode
  {
    TRIGGER,
    RETRIGGER,
    CONTINUOUS
  };
  GateMode gateMode = TRIGGER;
  PulseGenerator gatePulse;

  Bene2() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  
  void step() override;

  json_t *toJson() override
  {
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "running Line", json_boolean(runningL));
    json_object_set_new(rootJ, "running Column", json_boolean(runningC));
    json_object_set_new(rootJ, "ignoreGateOnPitchOut", json_boolean(ignoreGateOnPitchOut));

    // gates
    json_t *gatesJ = json_array();
    for (int i = 0; i < 16; i++)
    {
      json_t *gateJ = json_integer((int)gateState[i]);
      json_array_append_new(gatesJ, gateJ);
    }
    json_object_set_new(rootJ, "gates", gatesJ);

    // gateMode
    json_t *gateModeJ = json_integer((int)gateMode);
    json_object_set_new(rootJ, "gateMode", gateModeJ);

    return rootJ;
  }

  void fromJson(json_t *rootJ) override
  {
    json_t *runningLJ = json_object_get(rootJ, "running Line");
    if (runningLJ)
      runningL = json_is_true(runningLJ);

    json_t *runningCJ = json_object_get(rootJ, "running Column");
    if (runningCJ)
      runningC = json_is_true(runningCJ);

    json_t *ignoreGateOnPitchOutJ = json_object_get(rootJ, "ignoreGateOnPitchOut");
    if (ignoreGateOnPitchOutJ)
      ignoreGateOnPitchOut = json_is_true(ignoreGateOnPitchOutJ);

    // gates
    json_t *gatesJ = json_object_get(rootJ, "gates");
    if (gatesJ)
    {
      for (int i = 0; i < 16; i++)
      {
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

  void reset() override
  {
    for (int i = 0; i < 16; i++)
    {
      gateState[i] = false;
    }
  }

  void randomize() override
  {
    randomizeGateStates();
  }

  void randomizeGateStates()
  {
    for (int i = 0; i < 16; i++)
    {
      gateState[i] = (randomUniform() > 0.50);
    }
  }

  void handleMoveRight() {
    for (int i=0; i<4; i++)
     { 
       posX[i] = posX[i] == 3 ? 0 : posX[i] + 1;
      }
    }
  void handleMoveLeft() {
    for (int i=0; i<4; i++)
     { 
       posX[i] = posX[i] == 0 ? 3 : posX[i] - 1;
      }
    }
  void handleMoveDown() {
    for (int i=0; i<4; i++)
     { 
       posY[i] = posY[i] == 3 ? 0 : posY[i] + 1;
      }
    }
  void handleMoveUp() {
    for (int i=0; i<4; i++)
     { 
       posY[i] = posY[i] == 0 ? 3 : posY[i] - 1;
      }
    }

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// STEP
///////////////////////////////////////////////////////////////////////////////////////////////////

  void Bene2::step()
  {

  	 bool step_right[4] = {false};
     bool step_left[4] = {false};
     bool step_up[4] = {false};
     bool step_down[4] = {false};
   


    const float lightLambda = 0.1;


///////////////////////////////////////////////////////////////////////////////
// Running Buttons
/////////////////////////////////////////////////////////////////////////////

    if (runningLTrigger.process(params[RUNL_PARAM].value+inputs[RUNL_INPUT].value))
    {
      runningL = !runningL;
    }
    lights[RUNL_LIGHT].value = runningL ? 1.0 : 0.0;

    if (runningCTrigger.process(params[RUNC_PARAM].value))
    {
      runningC = !runningC;
    }
    lights[RUNC_LIGHT].value = runningC ? 1.0 : 0.0;

///////////////////////////////////////////////////////////////////////////////
// RESET
/////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 4; i++)
    {
      if (resetLiTrigger[i].process(inputs[RESETL + i].value))
      {
        posX[i] = 0;
        step_right[i] = false;
        step_left[i] = false;
        lights[STEPS_LIGHT + posX[i] + i * 4].value = 0.8;
     }
      if (resetCoTrigger[i].process(inputs[RESETC + i].value))
      {
        posY[i] = 0;
        step_up[i] = false;
        step_down[i] = false;
        lights[STEPS_LIGHT + i + posY[i] * 4].value = 0.8;
      }
    }

   if(resetALTrigger.process(params[RESET_LINE].value+inputs[RESETL_INPUT].value))
    {
      lights[RESETL_LIGHT].value = 1.0;
      for (int i=0;i<4;i++)
      {
        posX[i] = 0;
        step_right[i] = false;
        step_left[i] = false;
        lights[STEPS_LIGHT + posX[i] + i * 4].value = 0.8;
      }
    }
    if(resetACTrigger.process(params[RESET_COL].value+inputs[RESETC_INPUT].value))
    {
      lights[RESETC_LIGHT].value = 1.0;
      for (int i = 0; i < 4; i++)
      {
        posY[i] = 0;
        step_up[i] = false;
        step_down[i] = false;
        lights[STEPS_LIGHT + i + posY[i] * 4].value = 0.8;
      }
    }

    if (lights[RESETL_LIGHT].value > 0)
    {
      lights[RESETL_LIGHT].value -= lights[RESETL_LIGHT].value / lightLambda / engineGetSampleRate();
    }

    if (lights[RESETC_LIGHT].value > 0)
    {
      lights[RESETC_LIGHT].value -= lights[RESETC_LIGHT].value / lightLambda / engineGetSampleRate();
    }

      ///////////////////////////////////////////////////////////////////////////////
      // RUN
      /////////////////////////////////////////////////////////////////////////////

      if (runningL)
      {
        for (int i = 0; i < 4; i++)
        {
          // handle clock inputs
          if (inputs[RIGHT + i].active)
          {
            if (rightTrigger[i].process(inputs[RIGHT + i].value))
            {
              step_right[i] = true;
            }
          }
          if (inputs[LEFT + i].active)
          {
            if (leftTrigger[i].process(inputs[LEFT + i].value))
            {
              step_left[i] = true;
            }
          }
        }
      }
      if (runningC)
       {
         for (int i = 0; i < 4; i++)
         {
           if (inputs[DOWN + i].active)
           {
             if (downTrigger[i].process(inputs[DOWN + i].value))
             {
               step_down[i] = true;
             }
           }
           if (inputs[UP + i].active)
           {
             if (upTrigger[i].process(inputs[UP + i].value))
             {
               step_up[i] = true;
             }
           }
        }
       }

       

     for (int i = 0; i < 4; i++)
     {
       index[i]=posX[i]+(posY[i]*4);
     }

     // change x and y
       for (int i = 0; i < 4; i++)
       {
         if (step_right[i])
         {
           posX[i] += 1;
           if (posX[i] > 3)
             posX[i] = 0;
           lights[STEPS_LIGHT + posX[i] + i * 4].value = 0.8;
           gatePulse.trigger(1e-3);
         }

         if (step_left[i])
         {
           posX[i] -= 1;
           if (posX[i] < 0)
             posX[i] = 3;
           lights[STEPS_LIGHT + posX[i] + i * 4].value = 0.8;
           gatePulse.trigger(1e-3);
         }

         if (step_down[i])
         {
           posY[i] += 1;
           if (posY[i] > 3)
             posY[i] = 0;
           lights[STEPS_LIGHT + i + posY[i] * 4].value = 0.8;
           gatePulse.trigger(1e-3);
         }
         if (step_up[i])
         {
           posY[i] -= 1;
           if (posY[i] < 0)
             posY[i] = 3;
           lights[STEPS_LIGHT + i + posY[i] * 4].value = 0.8;
           gatePulse.trigger(1e-3);
         }
       }

       bool pulse = gatePulse.process(1.0 / engineGetSampleRate());

       for (int i = 0; i < 16; i++)
       {
         if (gateTriggers[i].process(params[GATE_PARAM + i].value))
         {
           gateState[i] = !gateState[i];
         }
         
         if (lights[STEPS_LIGHT + i].value > 0)
         {
           lights[STEPS_LIGHT + i].value -= lights[STEPS_LIGHT + i].value / lightLambda / engineGetSampleRate();
         }
         lights[GATE_LIGHT + i].value = gateState[i] ? 1.0 - lights[STEPS_LIGHT + i].value : lights[STEPS_LIGHT + i].value;
       }


       // Outputs
    
       for (int i=0;i<4;i++)
       {
         bool gatesOnL = (runningL && gateState[i + posY[i] * 4]);
         if (gateMode == TRIGGER)
           gatesOnL = gatesOnL && pulse;
         else if (gateMode == RETRIGGER)
           gatesOnL = gatesOnL && !pulse;

         bool gatesOnC = (runningC && gateState[posX[i] + i * 4]);
         if (gateMode == TRIGGER)
           gatesOnC = gatesOnC && pulse;
         else if (gateMode == RETRIGGER)
           gatesOnC = gatesOnC && !pulse;

         row_outs[i] = (params[KNOB_PARAM + i + posY[i] * 4].value);
         column_outs[i] = (params[KNOB_PARAM + posX[i] + i * 4].value);

         if (gatesOnL || ignoreGateOnPitchOut)
        {
          outputs[ROW_OUT + i].value = row_outs[i];
        }

        if (gatesOnC || ignoreGateOnPitchOut)
        {
         outputs[COLUMN_OUT + i].value = column_outs[i];
        }

         outputs[GATES_COL_OUT + i].value = gatesOnC ? 10.0 : 0.0;
         outputs[GATES_ROW_OUT + i].value = gatesOnL ? 10.0 : 0.0;
       }
      }
     
  
  
    


template <typename BASE>
struct RunLight : BASE
{
  RunLight()
  {
    this->box.size = mm2px(Vec(5.5, 5.5));
  }
};


struct Bene2Widget : ModuleWidget 
{
  void appendContextMenu(Menu *menu) override;
Bene2Widget(Bene2 *module) : ModuleWidget(module)

{
	box.size = Vec(15*20, 380);

  int top = 90;
  int top2 = 35;
  int left = 118;
  int column_spacing = 37;
  int jacks=30;
  int row_spacing = 37;
  int lb=18;

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin,"res/Bene2.svg")));
    addChild(panel);
  }

for (int i=0;i<4;i++)
{
  addInput(Port::create<PJ301MCPort>(Vec(lb, top+jacks*i), Port::INPUT, module, Bene2::LEFT+i));
  addInput(Port::create<PJ301MCPort>(Vec(lb+27, top+jacks*i), Port::INPUT, module, Bene2::RIGHT+i));
  addInput(Port::create<PJ301MCPort>(Vec(lb + 27 + 27, top + jacks * i), Port::INPUT, module, Bene2::RESETL + i));

  addInput(Port::create<PJ301MCPort>(Vec(lb, top+jacks*i + 140), Port::INPUT, module, Bene2::UP+i));
  addInput(Port::create<PJ301MCPort>(Vec(lb + 27, top + jacks * i + 140), Port::INPUT, module, Bene2::DOWN + i));
  addInput(Port::create<PJ301MCPort>(Vec(lb + 27 + 27, top + jacks * i + 140), Port::INPUT, module, Bene2::RESETC + i));
}

addParam(ParamWidget::create<LEDBezel>(Vec(lb,5+ 10 ), module, Bene2::RUNL_PARAM, 0.0, 1.0, 0.0));
addParam(ParamWidget::create<LEDBezel>(Vec(lb,5+ 10+30), module, Bene2::RUNC_PARAM, 0.0, 1.0, 0.0));

addChild(GrayModuleLightWidget::create<RunLight<OrangeLight>>(Vec(lb+3,5+ 10+3), module, Bene2::RUNL_LIGHT));
addChild(GrayModuleLightWidget::create<RunLight<OrangeLight>>(Vec(lb+3,5+ 10+3+30), module, Bene2::RUNC_LIGHT));

addInput(Port::create<PJ301MCPort>(Vec(lb+30,5+ 9), Port::INPUT, module, Bene2::RUNL_INPUT));
addInput(Port::create<PJ301MCPort>(Vec(lb+ 30,5+ 9 + 30), Port::INPUT, module, Bene2::RUNC_INPUT));

addParam(ParamWidget::create<LEDBezel>(Vec(lb+ 120, 5 + 10), module, Bene2::RESET_LINE, 0.0, 1.0, 0.0));
addParam(ParamWidget::create<LEDBezel>(Vec(lb+ 120, 5 + 10 + 30), module, Bene2::RESET_COL, 0.0, 1.0, 0.0));

addChild(GrayModuleLightWidget::create<RunLight<OrangeLight>>(Vec(lb +  120+3, 5 + 10 + 3), module, Bene2::RESETL_LIGHT));
addChild(GrayModuleLightWidget::create<RunLight<OrangeLight>>(Vec(lb +  120+3, 5 + 10 + 3 + 30), module, Bene2::RESETC_LIGHT));

addInput(Port::create<PJ301MCPort>(Vec(lb + 150, 5 + 9), Port::INPUT, module, Bene2::RESETL_INPUT));
addInput(Port::create<PJ301MCPort>(Vec(lb + 150, 5 + 9 + 30), Port::INPUT, module, Bene2::RESETC_INPUT));

//addInput(Port::create<PJ301MCPort>(Vec(left + column_spacing * 3, top ), Port::INPUT, module, Bene2::RESET));
 
 
  for ( int i = 0 ; i < 4 ; i++)
  {
    for ( int j = 0 ; j < 4 ; j++)
    {
      addParam(ParamWidget::create<Rogan2PWhite>(Vec(left + column_spacing * i, top2 + row_spacing * j + 70), module, Bene2::KNOB_PARAM + i + j * 4, 0.0, 2.0, 1.0));
      addParam(ParamWidget::create<LEDBezel>(Vec(left + column_spacing * i + 7.0, top2 + row_spacing * j + 70 + 7.0), module, Bene2::GATE_PARAM + i + j * 4, 0.0, 1.0, 0.0));
      addChild(GrayModuleLightWidget::create<RunLight<OrangeLight>>(Vec(left + column_spacing * i + 10, top2 + row_spacing * j + 70 + 10), module, Bene2::STEPS_LIGHT + i + j * 4));
      addChild(GrayModuleLightWidget::create<RunLight<OrangeLight>>(Vec(left + column_spacing * i + 10, top2 + row_spacing * j + 70 + 10), module, Bene2::GATE_LIGHT + i + j * 4));
    }
    addOutput(Port::create<PJ301MOPort>(Vec(left+column_spacing * i+5, top2 + row_spacing * 4 + 75 ), Port::OUTPUT, module, Bene2::ROW_OUT + i));
    addOutput(Port::create<PJ301MOPort>(Vec(left+column_spacing * 4+5, top2 + row_spacing * i + 75 ), Port::OUTPUT, module, Bene2::COLUMN_OUT + i));

    addOutput(Port::create<PJ301MOPort>(Vec(left + column_spacing*i,300), Port::OUTPUT, module, Bene2::GATES_COL_OUT + i));
    addOutput(Port::create<PJ301MOPort>(Vec(left + column_spacing * i,330), Port::OUTPUT, module, Bene2::GATES_ROW_OUT + i));
  }

  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

}

};

struct Bene2PitchMenuItem : MenuItem
{
  Bene2 *bene2;
  void onAction(EventAction &e) override
  {
    bene2->ignoreGateOnPitchOut = !bene2->ignoreGateOnPitchOut;
  }
  void step() override
  {
    rightText = (bene2->ignoreGateOnPitchOut) ? "✔" : "";
  }
};

struct Bene2GateModeItem : MenuItem
{
  Bene2 *bene2;
  Bene2::GateMode gateMode;
  void onAction(EventAction &e) override
  {
    bene2->gateMode = gateMode;
  }
  void step() override
  {
    rightText = (bene2->gateMode == gateMode) ? "✔" : "";
  }
};

void Bene2Widget::appendContextMenu(Menu *menu) {

  MenuLabel *spacerLabel = new MenuLabel();
  menu->addChild(spacerLabel);

  Bene2 *bene2 = dynamic_cast<Bene2 *>(module);
  assert(bene2);

  menu->addChild(MenuLabel::create("Gate Mode"));

  Bene2GateModeItem *triggerItem = MenuItem::create<Bene2GateModeItem>("Trigger");
  triggerItem->bene2 = bene2;
  triggerItem->gateMode = Bene2::TRIGGER;
  menu->addChild(triggerItem);

  Bene2GateModeItem *retriggerItem = MenuItem::create<Bene2GateModeItem>("Retrigger");
  retriggerItem->bene2 = bene2;
  retriggerItem->gateMode = Bene2::RETRIGGER;
  menu->addChild(retriggerItem);

  Bene2GateModeItem *continuousItem = MenuItem::create<Bene2GateModeItem>("Continuous");
  continuousItem->bene2 = bene2;
  continuousItem->gateMode = Bene2::CONTINUOUS;
  menu->addChild(continuousItem);

  MenuLabel *spacerLabel2 = new MenuLabel();
  menu->addChild(spacerLabel2);

  Bene2PitchMenuItem *pitchMenuItem = MenuItem::create<Bene2PitchMenuItem>("Ignore Gate for V/OCT Out");
  pitchMenuItem->bene2 = bene2;
  menu->addChild(pitchMenuItem);
}

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, Bene2) {
   Model *modelBene2 = Model::create<Bene2, Bene2Widget>("dBiz", "Bene2", "Bene2", SEQUENCER_TAG);
   return modelBene2;
}
