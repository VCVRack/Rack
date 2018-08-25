///////////////////////////////////////////////////////////////////
//
//  Patch Matrix Module for VCV
//
//  Strum 2017
//  strum@softhome.net
//
///////////////////////////////////////////////////////////////////

#include "mental.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mental {

struct MentalPatchMatrix : Module {
	enum ParamIds {
    SWITCHES,
		NUM_PARAMS = SWITCHES + 100
	};  
	enum InputIds {		
		INPUTS,
    NUM_INPUTS = INPUTS + 10
	};
	enum OutputIds {
		OUTPUTS,    
		NUM_OUTPUTS = OUTPUTS + 10
	};
  enum LightIds {
		SWITCH_LIGHTS,
    NUM_LIGHTS = SWITCH_LIGHTS + 100
	};

  SchmittTrigger switch_triggers[10][10];
  bool switch_states[10][10] = 
  {{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}};
   
  
  float input_values[10] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  float sums[10] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}; 
  
	MentalPatchMatrix() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
  
  json_t *toJson() override
  {
		json_t *rootJ = json_object();
    
    // button states
		json_t *button_statesJ = json_array();
		for (int i = 0; i < 10; i++)
    {
			for (int j = 0; j < 10; j++)
      {
        json_t *button_stateJ = json_integer((int) switch_states[i][j]);
			  json_array_append_new(button_statesJ, button_stateJ);
      }
		}
		json_object_set_new(rootJ, "buttons", button_statesJ);
    
    return rootJ;
  }
  
  void fromJson(json_t *rootJ) override
  {
    // button states
		json_t *button_statesJ = json_object_get(rootJ, "buttons");
		if (button_statesJ)
    {
			for (int i = 0; i < 10; i++)
      {
        for (int j = 0; j < 10; j++)
        {
				  json_t *button_stateJ = json_array_get(button_statesJ, i*10 + j);
				  if (button_stateJ)
					  switch_states[i][j] = !!json_integer_value(button_stateJ);
        }
      }
		}  
  }  
};

////// Step function
void MentalPatchMatrix::step() {
	
  for ( int i = 0 ; i < 10 ; i++)
  {
   sums[i] = 0.0;
  }
  // deal with buttons
  for (int i = 0 ; i < 10 ; i++)
  {
   for (int j = 0 ; j < 10 ; j++)
   {
     if (switch_triggers[i][j].process(params[SWITCHES+j*10 + i].value))
     {
		   switch_states[i][j] = !switch_states[i][j];
	   }
     lights[SWITCH_LIGHTS + i + j * 10].value  = (switch_states[i][j]) ? 1.0 : 0.0;
   }
  }
  // get inputs
  for (int i = 0 ; i < 10 ; i++)
  {
    input_values[i] = inputs[INPUTS + i].value;
  }
  
  // add inputs 
  
  for (int i = 0 ; i < 10 ; i++)
  {
   for (int j = 0 ; j < 10 ; j++)
   {
     if (switch_states[j][i]) sums[i] += input_values[j];
   }
  }
  /// outputs  
  for (int i = 0 ; i < 10 ; i++)
  {
    outputs[OUTPUTS + i].value = sums[i];    
  }  
}

///// Gui
struct MentalPatchMatrixWidget : ModuleWidget {
  MentalPatchMatrixWidget(MentalPatchMatrix *module);
};

MentalPatchMatrixWidget::MentalPatchMatrixWidget(MentalPatchMatrix *module) : ModuleWidget(module)
{
  
  setPanel(SVG::load(assetPlugin(plugin, "res/MentalPatchMatrix.svg")));

  int top_row = 75;
  int row_spacing = 25; 
  int column_spacing = 25;

	for (int i = 0 ; i < 10 ; i++)
  {
	 addInput(Port::create<InPort>(Vec(3, i * row_spacing + top_row), Port::INPUT, module, MentalPatchMatrix::INPUTS + i));  
   addOutput(Port::create<OutPort>(Vec(33 + i * column_spacing , top_row + 10 * row_spacing), Port::OUTPUT, module, MentalPatchMatrix::OUTPUTS + i));
   for(int j = 0 ; j < 10 ; j++ )
   {
     addParam(ParamWidget::create<LEDButton>(Vec(35 + column_spacing * j, top_row + row_spacing * i), module, MentalPatchMatrix::SWITCHES + i + j * 10, 0.0, 1.0, 0.0));
     addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(35 + column_spacing * j + 5, top_row + row_spacing * i + 5), module, MentalPatchMatrix::SWITCH_LIGHTS  + i + j * 10));
   }
	}  
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalPatchMatrix) {
   Model *modelMentalPatchMatrix = Model::create<MentalPatchMatrix, MentalPatchMatrixWidget>("mental", "MentalPatchMatrix", "Patch Matrix", UTILITY_TAG);
   return modelMentalPatchMatrix;
}
