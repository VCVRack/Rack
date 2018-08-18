#include "Mixer4.hpp"

namespace rack_plugin_SynthKit {

Mixer4Module::Mixer4Module()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  for (uint8_t i = 0; i < MIXER_CHANNELS; i++) {
    channel_led_l[i] = 0.0f;
    channel_led_r[i] = 0.0f;
    mute[i] = false;
    solo[i] = false;
    mute_trigger[i] = new SynthDevKit::CV(0.1f);
    solo_trigger[i] = new SynthDevKit::CV(0.1f);
  }

  mute_trigger_l = new SynthDevKit::CV(0.1f);
  mute_trigger_r = new SynthDevKit::CV(0.1f);
}

void Mixer4Module::step() {
  float channel_l_out[MIXER_CHANNELS];
  float channel_r_out[MIXER_CHANNELS];
  float out_l = 0.0f;
  float out_r = 0.0f;
  float has_solo = false;

  // check the master mute states
  mute_trigger_l->update(params[MUTEL_PARAM].value);
  mute_trigger_r->update(params[MUTER_PARAM].value);

  if (mute_trigger_l->newTrigger()) {
    mute_l = !mute_l;
    lights[MUTEL_LIGHT].value = mute_l ? 1.0f : 0.0f;
  }

  if (mute_trigger_r->newTrigger()) {
    mute_r = !mute_r;
    lights[MUTER_LIGHT].value = mute_r ? 1.0f : 0.0f;
  }

  // bypass everything if both are mute
  if (mute_l && mute_r) {
    master_led_l = 0.0f;
    master_led_r = 0.0f;

    outputs[LEFT_OUTPUT].value = 0.0f;
    outputs[RIGHT_OUTPUT].value = 0.0f;

    return;
  }

  // mute / solo checking early for solo
  for (uint8_t i = 0; i < MIXER_CHANNELS; i++) {
    mute_trigger[i]->update(params[MUTE_PARAM + i].value);
    solo_trigger[i]->update(params[SOLO_PARAM + i].value);

    if (solo_trigger[i]->newTrigger()) {
      solo[i] = !solo[i];
      lights[SOLO_LIGHT + i].value = solo[i] ? 1.0f : 0.0f;
    }

    if (mute_trigger[i]->newTrigger()) {
      mute[i] = !mute[i];
      lights[MUTE_LIGHT + i].value = mute[i] ? 1.0f : 0.0f;
    }

    if (solo[i]) {
      has_solo = true;
    }
  }

  for (uint8_t i = 0; i < MIXER_CHANNELS; i++) {
    // initial output settings
    channel_l_out[i] = 0.0f;
    channel_r_out[i] = 0.0f;

    // if the input is muted or not active, continue
    // also continue if there are solo channels and this is not one
    if (mute[i] || inputs[MIXER_INPUT + i].active == false ||
        (has_solo && !solo[i])) {
      channel_led_l[i] = 0.0f;
      channel_led_r[i] = 0.0f;
      continue;
    }

    float input = inputs[MIXER_INPUT + i].value;

    // figure out the pan
    float pan = params[PAN_PARAM + i].value;

    // add any cv input
    if (inputs[PAN_CV_INPUT + i].active) {
      pan = clamp((inputs[PAN_CV_INPUT + i].value / 10.0f) + pan, 0.0f, 1.0f);
    }

    channel_l_out[i] = channel_r_out[i] = input;

    // determine the left/right mixes
    if (pan < 0.5f) {
      channel_r_out[i] = (2.0f * pan) * channel_r_out[i];
    }

    if (pan > 0.5f) {
      channel_l_out[i] = (2.0f * (1.0f - pan)) * channel_l_out[i];
    }

    // if the left send is active, send it
    if (outputs[SENDL_OUTPUT + i].active) {
      outputs[SENDL_OUTPUT + i].value = channel_l_out[i];
    }

    // if the right send is active, send it
    if (outputs[SENDR_OUTPUT + i].active) {
      outputs[SENDR_OUTPUT + i].value = channel_r_out[i];
    }

    // if the left recv is active, get it
    if (inputs[RECVL_INPUT + i].active) {
      float recvl = inputs[RECVL_INPUT + i].value;

      // figure out the wet/dry mix
      float mixl = params[MIXL_PARAM + i].value;

      channel_l_out[i] = ((mixl)*recvl) + ((1.0f - mixl) * channel_l_out[i]);
    }

    // if the right recv is active, get it
    if (inputs[RECVR_INPUT + i].active) {
      float recvr = inputs[RECVR_INPUT + i].value;

      // figure out the wet/dry mix
      float mixr = params[MIXR_PARAM + i].value;

      channel_r_out[i] = ((mixr)*recvr) + ((1.0f - mixr) * channel_r_out[i]);
    }
    // figure out the volume
    float volume = params[VOLUME_PARAM + i].value;

    if (inputs[MIXER_CV_INPUT + i].active) {
      volume = clamp((inputs[MIXER_CV_INPUT + i].value / 10.0f) + volume, 0.0f,
                     1.2f);
    }

    // apply the volume
    channel_l_out[i] *= volume;
    channel_r_out[i] *= volume;

    channel_led_l[i] = channel_l_out[i];
    channel_led_r[i] = channel_r_out[i];

    // add it to the output
    out_l += channel_l_out[i];
    out_r += channel_r_out[i];
  }

  // out_l /= MIXER_CHANNELS;
  // out_r /= MIXER_CHANNELS;

  // if the left send is active, send it
  if (outputs[MASTERL_SEND_OUTPUT].active) {
    outputs[MASTERL_SEND_OUTPUT].value = out_l;
  }

  // if the right send is active, send it
  if (outputs[MASTERR_SEND_OUTPUT].active) {
    outputs[MASTERR_SEND_OUTPUT].value = out_r;
  }

  // if the left recv is active, get it
  if (inputs[MASTERL_RECV_INPUT].active) {
    float recvl = inputs[MASTERL_RECV_INPUT].value;

    // figure out the wet/dry mix
    float mixl = params[MASTERL_MIX_PARAM].value;

    out_l = ((mixl)*recvl) + ((1.0f - mixl) * out_l);
  }

  // if the right recv is active, get it
  if (inputs[MASTERR_RECV_INPUT].active) {
    float recvr = inputs[MASTERR_RECV_INPUT].value;

    // figure out the wet/dry mix
    float mixr = params[MASTERR_MIX_PARAM].value;

    out_r = ((mixr)*recvr) + ((1.0f - mixr) * out_r);
  }

  master_led_l = out_l;
  master_led_r = out_r;

  outputs[LEFT_OUTPUT].value = out_l;
  outputs[RIGHT_OUTPUT].value = out_r;
}

} // namespace rack_plugin_SynthKit
