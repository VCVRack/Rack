///////////////////////////////////////////////////////////////////
//
//  Cartesian Sequencer Module for VCV similar to Make Noise Rene
//
//  Strum 2017
//  strum@softhome.net
//
///////////////////////////////////////////////////////////////////

#include "mental.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mental {

struct MentalCartesian : Module {
	enum ParamIds {
    BUTTON_PARAM,
    KNOB_PARAM = BUTTON_PARAM + 16,
    NUM_PARAMS = KNOB_PARAM + 16		
	};  
	enum InputIds {		
		UP,
    DOWN,
    LEFT,
    RIGHT,
    X_CV,
    Y_CV,
    RESET,
    X_RESET,
    Y_RESET,
		NUM_INPUTS
	};
	enum OutputIds {
		UNQUANT_OUT,
    QUANT_OUT,
    ROW_OUT,
    COLUMN_OUT = ROW_OUT + 4,    
		NUM_OUTPUTS = COLUMN_OUT + 4
	};
  enum LightIds {
		BUTTON_LIGHTS,
    GRID_LIGHTS = BUTTON_LIGHTS + 16,
		NUM_LIGHTS = GRID_LIGHTS + 16
	};
  
  SchmittTrigger leftTrigger;
  SchmittTrigger rightTrigger;
  SchmittTrigger upTrigger;
  SchmittTrigger downTrigger;
  SchmittTrigger resetTrigger;
  SchmittTrigger x_resetTrigger;
  SchmittTrigger y_resetTrigger;
    
  SchmittTrigger button_triggers[4][4];
    
  float row_outs[4] = {0.0,0.0,0.0,0.0};
  float column_outs[4] = {0.0,0.0,0.0,0.0};
  
  int x_position = 0;
  int y_position = 0;
    
	MentalCartesian() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
   
};

void MentalCartesian::step() {

  	bool step_right = false;
    bool step_left = false;
    bool step_up = false;
    bool step_down = false;
    
    lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
    
    // handle clock inputs
    if (inputs[RIGHT].active)
    {
			if (rightTrigger.process(inputs[RIGHT].value))
      {
				step_right = true;
			}
		}
    if (inputs[LEFT].active)
    {
			if (leftTrigger.process(inputs[LEFT].value))
      {
				step_left = true;
			}
		}
    if (inputs[DOWN].active)
    {
			if (downTrigger.process(inputs[DOWN].value))
      {
				step_down = true;
			}
		}
    if (inputs[UP].active)
    {
			if (upTrigger.process(inputs[UP].value))
      {
				step_up = true;
			}
		}
    // resets
    if (resetTrigger.process(inputs[RESET].value))
    {
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
		  x_position = 0;
      y_position = 0;
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
      step_right = false;
      step_left = false;
      step_up = false;	
      step_down = false;	
	  }
    if (x_resetTrigger.process(inputs[X_RESET].value))
    {
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
		  x_position = 0;
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
      step_right = false;
      step_left = false;
      step_up = false;	
      step_down = false;	
	  }
    if (y_resetTrigger.process(inputs[Y_RESET].value))
    {
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
		  y_position = 0;
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
      step_right = false;
      step_left = false;
      step_up = false;	
      step_down = false;	
	  }
    // handle CV inputs
    int x_cv = round(inputs[X_CV].value);
    if (x_cv < 0 ) x_cv = 0;
    if (x_cv > 3 ) x_cv = 3;
    std::div_t division_x;
    division_x = div(x_position + x_cv,4);
    lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
    x_position = division_x.rem;
    lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
    int y_cv = round(inputs[Y_CV].value);
    if (y_cv < 0 ) y_cv = 0;
    if (y_cv > 3 ) y_cv = 3;    
    std::div_t division_y;
    division_y = div(y_position + y_cv,4);
    lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
    y_position = division_y.rem;
    lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
    // handle button triggers
    for (int i = 0 ; i < 4 ; i++)
    {
      for (int j = 0 ; j < 4 ; j++)
      {
        if ((params[BUTTON_PARAM + i + j * 4].value))
        {
          lights[BUTTON_LIGHTS + i + j * 4].value = 1.0;
          lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
          x_position = i;
          y_position = j;
          lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
        } else lights[BUTTON_LIGHTS + i + j * 4].value = 0.0;
      }		  
	  }
    
    // change x and y    
    if (step_right)
    {
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
      x_position += 1;
      if (x_position > 3) x_position = 0;
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
    }
    if (step_left)
    {
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
      x_position -= 1;
      if (x_position < 0) x_position = 3;      
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
    }
    if (step_down)
    {
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
      y_position += 1;
      if (y_position > 3) y_position = 0;
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
    }
    if (step_up)
    {
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 0.0;
      y_position -= 1;
      if (y_position < 0) y_position = 3;      
      lights[GRID_LIGHTS + x_position + y_position * 4].value = 1.0;
    }
    
    /// set outputs
    int which_knob = y_position * 4 + x_position;
    float main_out = params[KNOB_PARAM + which_knob].value;
    
    int oct = round(main_out);
    float left = main_out - oct;
    int semi = round(left * 12);
    float quant_out = oct + semi/12.0;
    for (int i = 0 ; i < 4 ; i++)
    {
      row_outs[i] = params[KNOB_PARAM + y_position * 4 + i ].value;
      column_outs[i] = params[KNOB_PARAM + x_position + i * 4].value;
      outputs[ROW_OUT + i ].value = row_outs[i];
      outputs[COLUMN_OUT + i ].value = column_outs[i];            
    }
    
    outputs[UNQUANT_OUT].value = main_out;
    outputs[QUANT_OUT].value = quant_out;     
}

////////////////////////////////
struct MentalCartesianWidget : ModuleWidget {
  MentalCartesianWidget(MentalCartesian *module);
};


MentalCartesianWidget::MentalCartesianWidget(MentalCartesian *module) : ModuleWidget(module)
{


  setPanel(SVG::load(assetPlugin(plugin, "res/MentalCartesian.svg")));
 
  int top = 25;
  int left = 3;
  int column_spacing = 25; 
  int row_spacing = 25;
  int button_offset = 140;
	addInput(Port::create<GateInPort>(Vec(left, top), Port::INPUT, module, MentalCartesian::LEFT));
  addInput(Port::create<GateInPort>(Vec(left+column_spacing, top), Port::INPUT, module, MentalCartesian::RIGHT));
  addInput(Port::create<CVInPort>(Vec(left+column_spacing * 2, top), Port::INPUT, module, MentalCartesian::X_CV));
  addInput(Port::create<GateInPort>(Vec(left+column_spacing * 3, top), Port::INPUT, module, MentalCartesian::X_RESET));
  
  addInput(Port::create<GateInPort>(Vec(left, top + 30), Port::INPUT, module, MentalCartesian::UP));
  addInput(Port::create<GateInPort>(Vec(left + column_spacing, top + 30), Port::INPUT, module, MentalCartesian::DOWN));  
  addInput(Port::create<CVInPort>(Vec(left + column_spacing * 2, top + 30), Port::INPUT, module, MentalCartesian::Y_CV));
  addInput(Port::create<GateInPort>(Vec(left + column_spacing * 3, top + 30), Port::INPUT, module, MentalCartesian::Y_RESET));
    
  addInput(Port::create<GateInPort>(Vec(left + column_spacing * 3, top + row_spacing * 2 + 10), Port::INPUT, module, MentalCartesian::RESET));
  
  addOutput(Port::create<CVOutPort>(Vec(213, 20), Port::OUTPUT, module, MentalCartesian::UNQUANT_OUT));  
  addOutput(Port::create<CVOutPort>(Vec(213, 50), Port::OUTPUT, module, MentalCartesian::QUANT_OUT));	
  
  for ( int i = 0 ; i < 4 ; i++)
  {
    for ( int j = 0 ; j < 4 ; j++)
    {
      addParam(ParamWidget::create<SmlKnob>(Vec(left+column_spacing * i, top + row_spacing * j + 150 ), module, MentalCartesian::KNOB_PARAM + i + j * 4, -2.0, 2.0, 0.0));
      addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(left+column_spacing * i + 4, top + row_spacing * j + 150 + 4), module, MentalCartesian::GRID_LIGHTS + i + j * 4));
      addParam(ParamWidget::create<LEDButton>(Vec(button_offset+left+column_spacing * i, top + row_spacing * j + 150 ), module, MentalCartesian::BUTTON_PARAM + i + j * 4, 0.0, 1.0, 0.0));
      addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(button_offset+left+column_spacing * i + 5, top + row_spacing * j + 150 + 5), module, MentalCartesian::BUTTON_LIGHTS + i + j * 4));
    }
    addOutput(Port::create<CVOutPort>(Vec(left+column_spacing * i, top + row_spacing * 4 + 150 ), Port::OUTPUT, module, MentalCartesian::ROW_OUT + i));
    addOutput(Port::create<CVOutPort>(Vec(left+column_spacing * 4, top + row_spacing * i + 150 ), Port::OUTPUT, module, MentalCartesian::COLUMN_OUT + i));
	}  
	
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalCartesian) {
   Model *modelMentalCartesian = Model::create<MentalCartesian, MentalCartesianWidget>("mental", "MentalCartesian", "Cartesian Sequencer", SEQUENCER_TAG);
   return modelMentalCartesian;
}
