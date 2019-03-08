#pragma once

#include "skylights.hh"

#define AUDIO_OUTPUTS 0
#define AUDIO_INPUTS 8
#define TRACKS 4

struct recorder_module : Module {
  enum ParamIds {
		 NUM_PARAMS
  };
  enum InputIds {
		 ENUMS(AUDIO_INPUT, AUDIO_INPUTS),
		 NUM_INPUTS
  };
  enum OutputIds {
		  ENUMS(AUDIO_OUTPUT, AUDIO_OUTPUTS),
		  NUM_OUTPUTS
  };
  enum LightIds {
		 ENUMS(ARM_LIGHT, TRACKS),
		 NUM_LIGHTS
  };

  recorder_module();
  virtual ~recorder_module();

  void step() override;
};
