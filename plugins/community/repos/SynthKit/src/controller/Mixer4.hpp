#include <cstdint>

#include "../../deps/SynthDevKit/src/CV.hpp"
#include "../SynthKit.hpp"

namespace rack_plugin_SynthKit {

#define MIXER_CHANNELS 4

struct Mixer4Module : Module {
  enum ParamIds {
    VOLUME_PARAM,
    PAN_PARAM = MIXER_CHANNELS,
    SOLO_PARAM = MIXER_CHANNELS * 2,
    MUTE_PARAM = MIXER_CHANNELS * 3,
    MIXL_PARAM = MIXER_CHANNELS * 4,
    MIXR_PARAM = MIXER_CHANNELS * 5,
    MASTERL_PARAM = MIXER_CHANNELS * 6,
    MASTERR_PARAM = MIXER_CHANNELS * 7,
    MASTERL_MIX_PARAM = MIXER_CHANNELS * 8,
    MASTERR_MIX_PARAM,
    MUTEL_PARAM,
    MUTER_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    MIXER_INPUT,
    MIXER_CV_INPUT = MIXER_CHANNELS,
    PAN_CV_INPUT = MIXER_CHANNELS * 2,
    RECVL_INPUT = MIXER_CHANNELS * 3,
    RECVR_INPUT = MIXER_CHANNELS * 4,
    MASTERL_RECV_INPUT = MIXER_CHANNELS * 5,
    MASTERR_RECV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    SENDL_OUTPUT,
    SENDR_OUTPUT = MIXER_CHANNELS,
    MASTERL_SEND_OUTPUT = MIXER_CHANNELS * 2,
    MASTERR_SEND_OUTPUT,
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    SOLO_LIGHT = MIXER_CHANNELS,
    MUTE_LIGHT = MIXER_CHANNELS * 2,
    MUTEL_LIGHT = MIXER_CHANNELS * 3,
    MUTER_LIGHT,
    NUM_LIGHTS
  };

  Mixer4Module();
  void step() override;

  float channel_led_l[MIXER_CHANNELS];
  float channel_led_r[MIXER_CHANNELS];

  float master_led_l = 0.0f;
  float master_led_r = 0.0f;

  SynthDevKit::CV *mute_trigger[MIXER_CHANNELS];
  SynthDevKit::CV *solo_trigger[MIXER_CHANNELS];
  SynthDevKit::CV *mute_trigger_l;
  SynthDevKit::CV *mute_trigger_r;

  bool mute[MIXER_CHANNELS];
  bool solo[MIXER_CHANNELS];
  bool mute_l = false;
  bool mute_r = false;
};

} // namespace rack_plugin_SynthKit
