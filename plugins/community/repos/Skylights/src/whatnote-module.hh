#pragma once

#include "skylights.hh"

namespace rack_plugin_Skylights {

#define AUDIO_OUTPUTS 0
#define AUDIO_INPUTS 1

struct whatnote_module : Module {
  enum ParamIds {
		 NUM_PARAMS
  };
  enum InputIds {
		 ENUMS(AUDIO_INPUT, AUDIO_INPUTS),
		 NUM_INPUTS
  };
  enum OutputIds {
		  NUM_OUTPUTS
  };
  enum LightIds {
		 NUM_LIGHTS
  };

  whatnote_module();
  virtual ~whatnote_module();

  int octave;			// what octave did we just read
  int semitone;			// what semitone did we just read
  int cents;			// how many cents are left?
  double voltage;		// what was the last sampled voltage
  
  void step() override;
};

} // namespace rack_plugin_Skylights
