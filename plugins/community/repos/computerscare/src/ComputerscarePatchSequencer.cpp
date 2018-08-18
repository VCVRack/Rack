#include "Computerscare.hpp"
#include "dsp/digital.hpp"
#include "dsp/filter.hpp"

#include <string>
#include <sstream>
#include <iomanip>

namespace rack_plugin_computerscare {

struct ComputerscarePatchSequencer : Module {
	enum ParamIds {
    STEPS_PARAM,
    MANUAL_CLOCK_PARAM,
    EDIT_PARAM,
    EDIT_PREV_PARAM,
    ENUMS(SWITCHES,100),
	NUM_PARAMS
	};  
	enum InputIds {
		TRG_INPUT,
		ENUMS(INPUT_JACKS, 10),
		RANDOMIZE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUTS,    
		NUM_OUTPUTS = OUTPUTS + 10
	};
  enum LightIds {
		SWITCH_LIGHTS,
    	NUM_LIGHTS = SWITCH_LIGHTS + 200
	};

  SchmittTrigger switch_triggers[10][10];

  SchmittTrigger nextAddressRead;
  SchmittTrigger nextAddressEdit;
  SchmittTrigger prevAddressEdit;
  SchmittTrigger clockTrigger;
  SchmittTrigger randomizeTrigger;

  int address = 0;
  int editAddress = 0;
  int addressPlusOne = 1;
  int editAddressPlusOne = 1;
  
  int numAddresses = 2;
  bool switch_states[16][10][10] = 
  {{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}}
};
   
  
  float input_values[10] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  float sums[10] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}; 
  
ComputerscarePatchSequencer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;



	json_t *toJson() override
  {
		json_t *rootJ = json_object();
    
    // button states
		json_t *button_statesJ = json_array();
    for(int k = 0; k < 16; k++) {
		for (int i = 0; i < 10; i++)
    {
			for (int j = 0; j < 10; j++)
      {
        json_t *button_stateJ = json_integer((int) switch_states[k][i][j]);
			   json_array_append_new(button_statesJ, button_stateJ);
        }
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
          for(int k = 0; k < 16; k++) {

			for (int i = 0; i < 10; i++)
      {
        for (int j = 0; j < 10; j++)
        {
				  json_t *button_stateJ = json_array_get(button_statesJ, k*100+i*10 + j);
				  if (button_stateJ)
					  switch_states[k][i][j] = !!json_integer_value(button_stateJ);
        }
      }
    }
		}  
  } 
  	void onRandomize() override {
  		randomizePatchMatrix();
  	}
	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu

	void randomizePatchMatrix()
	{
		int randomIndex;
		for (int i = 0; i < 10; i++) 
		{
			randomIndex = floor(randomUniform()*10);

			for (int j = 0; j < 10; j++) 
			{
				if(j==randomIndex) 
					switch_states[editAddress][j][i] = 1;
				
				else 
					switch_states[editAddress][j][i]=0;
				
			//fully randomize everything.  a bit intense for its use in patching
			//switch_states[editAddress][i][j] =  (randomUniform() > 0.5f);
			}			
		}	

	}; // end randomize()
	void onReset() override
	{
		for(int k =0; k < 16; k++) {

				
		for (int i = 0; i < 10; i++) 
		{
			for (int j = 0; j < 10; j++) 
		{
			switch_states[k][i][j] =  0;
			}			
		}	
	}
	}; // end randomize()

};


void ComputerscarePatchSequencer::step() {

  int numStepsKnobPosition = (int) clamp(roundf(params[STEPS_PARAM].value), 1.0f, 16.0f);


  for ( int i = 0 ; i < 10 ; i++)
  {
   sums[i] = 0.0;
  }

  for (int i = 0 ; i < 10 ; i++)
  {
   for (int j = 0 ; j < 10 ; j++)
   {
     if (switch_triggers[i][j].process(params[SWITCHES+j*10 + i].value))
     {
     	// handle button clicks in the patch matrix
		switch_states[editAddress][i][j] = !switch_states[editAddress][i][j];
	 }

	 // update the green lights (step you are editing) and the red lights (current active step)
     lights[SWITCH_LIGHTS + i + j * 10].value  = (switch_states[editAddress][i][j]) ? 1.0 : 0.0;
   	 lights[SWITCH_LIGHTS + i + j * 10+100].value  = (switch_states[address][i][j]) ? 1.0 : 0.0;
   	
   }
  }

  if(numStepsKnobPosition != numAddresses) {
  	numAddresses = numStepsKnobPosition;
  }

  if(randomizeTrigger.process(inputs[RANDOMIZE_INPUT].value / 2.f)) {
  	randomizePatchMatrix();
  }
  if(nextAddressEdit.process(params[EDIT_PARAM].value) ) {
    editAddress = editAddress + 1;
    editAddress = editAddress % 16;
  }
  if(prevAddressEdit.process(params[EDIT_PREV_PARAM].value) ) {
    editAddress = editAddress - 1;
    editAddress = editAddress + 16;
    editAddress = editAddress % 16;
  }

  if(nextAddressRead.process(params[MANUAL_CLOCK_PARAM].value) || clockTrigger.process(inputs[TRG_INPUT].value / 2.f)) {
    numAddresses =  (int) clamp(roundf(params[STEPS_PARAM].value /*+ inputs[STEPS_INPUT].value*/), 1.0f, 16.0f);

    address = address + 1;
    address = address % numAddresses;
  }
  addressPlusOne = address + 1;
  editAddressPlusOne = editAddress + 1;

  for (int i = 0 ; i < 10 ; i++)
  {
    input_values[i] = inputs[INPUT_JACKS + i].value;
  }
  
  for (int i = 0 ; i < 10 ; i++)
  {
   for (int j = 0 ; j < 10 ; j++)
   {
   	// todo: toggle for each output of how to combine multiple active signals in a column
   	// sum, average, and, or etc
     if (switch_states[address][j][i]) sums[i] += input_values[j];
   }
  }
  /// outputs  
  for (int i = 0 ; i < 10 ; i++)
  {
    outputs[OUTPUTS + i].value = sums[i];    
  }  
}

////////////////////////////////////
struct NumberDisplayWidget3 : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  NumberDisplayWidget3() {
    font = Font::load(assetPlugin(plugin, "res/digital-7.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
    // Background
    NVGcolor backgroundColor = nvgRGB(0x00, 0x00, 0x00);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);    
    
    // text 
    nvgFontSize(vg, 13);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;   
    to_display << std::setw(3) << *value;

    Vec textPos = Vec(6.0f, 17.0f);   
    NVGcolor textColor = nvgRGB(0xC0, 0xE7, 0xDE);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};



struct ComputerscarePatchSequencerWidget : ModuleWidget {

  ComputerscarePatchSequencerWidget(ComputerscarePatchSequencer *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/ComputerscarePatchSequencerPanel.svg")));

  int top_row = 70;
  int row_spacing = 26; 
  int column_spacing = 26;

	for (int i = 0 ; i < 10 ; i++)
  {
	 addInput(Port::create<InPort>(Vec(3, i * row_spacing + top_row), Port::INPUT, module, ComputerscarePatchSequencer::INPUT_JACKS + i));  
   	 addOutput(Port::create<InPort>(Vec(33 + i * column_spacing , top_row + 10 * row_spacing), Port::OUTPUT, module, ComputerscarePatchSequencer::OUTPUTS + i));
	   for(int j = 0 ; j < 10 ; j++ )
	   {
	   	 // the part you click
	     addParam(ParamWidget::create<LEDButton>(Vec(35 + column_spacing * j, top_row + row_spacing * i), module, ComputerscarePatchSequencer::SWITCHES + i + j * 10, 0.0, 1.0, 0.0));
	     
	     // green light indicates the state of the matrix that is being edited
	     addChild(ModuleLightWidget::create<LargeLight<GreenLight>>(Vec(35 + column_spacing * j +1.4, top_row + row_spacing * i +1.4 ), module, ComputerscarePatchSequencer::SWITCH_LIGHTS  + i + j * 10));
	   	 
	   	 // red light indicates the state of the matrix that is the active step
	   	 addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(35 + column_spacing * j + 4.3, top_row + row_spacing * i + 4.3), module, ComputerscarePatchSequencer::SWITCH_LIGHTS  + i + j * 10+100));

	   		}
		} 

	//clock input
  	addInput(Port::create<InPort>(Vec(3, 0), Port::INPUT, module, ComputerscarePatchSequencer::TRG_INPUT));
  	
  	//manual clock button
 	addParam(ParamWidget::create<LEDButton>(Vec(7 , 41), module, ComputerscarePatchSequencer::MANUAL_CLOCK_PARAM, 0.0, 1.0, 0.0)); 

  	//randomize input
  	addInput(Port::create<InPort>(Vec(270, 0), Port::INPUT, module, ComputerscarePatchSequencer::RANDOMIZE_INPUT));

  //active step display
  NumberDisplayWidget3 *display = new NumberDisplayWidget3();
  display->box.pos = Vec(30,40);
  display->box.size = Vec(50, 20);
  display->value = &module->addressPlusOne;
  addChild(display);

  // number of steps display
  NumberDisplayWidget3 *stepsDisplay = new NumberDisplayWidget3();
  stepsDisplay->box.pos = Vec(150,40);
  stepsDisplay->box.size = Vec(50, 20);
  stepsDisplay->value = &module->numAddresses;
  addChild(stepsDisplay);
  
  //number-of-steps dial.   Discrete, 16 positions
  ParamWidget* stepsKnob =  ParamWidget::create<LrgKnob>(Vec(108,30), module, ComputerscarePatchSequencer::STEPS_PARAM, 1.0f, 16.0f, 2.0f);
  addParam(stepsKnob);

  //editAddressNext button
  addParam(ParamWidget::create<LEDButton>(Vec(227 , 41), module, ComputerscarePatchSequencer::EDIT_PARAM, 0.0, 1.0, 0.0));

  //editAddressPrevious button
  addParam(ParamWidget::create<LEDButton>(Vec(208 , 41), module, ComputerscarePatchSequencer::EDIT_PREV_PARAM, 0.0, 1.0, 0.0));
  
  // currently editing step #:
  NumberDisplayWidget3 *displayEdit = new NumberDisplayWidget3();
  displayEdit->box.pos = Vec(245,40);
  displayEdit->box.size = Vec(50, 20);
  displayEdit->value = &module->editAddressPlusOne;
  addChild(displayEdit);
  }
};

} // namespace rack_plugin_computerscare

using namespace rack_plugin_computerscare;

RACK_PLUGIN_MODEL_INIT(computerscare, ComputerscarePatchSequencer) {
   Model *modelComputerscarePatchSequencer = Model::create<ComputerscarePatchSequencer, ComputerscarePatchSequencerWidget>("computerscare", "computerscare-patch-sequencer", "Father & Son Patch Sequencer", UTILITY_TAG,SEQUENCER_TAG);
   return modelComputerscarePatchSequencer;
}
