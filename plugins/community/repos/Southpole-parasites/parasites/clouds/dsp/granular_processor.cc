// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Main processing class.

#include "clouds/dsp/granular_processor.h"

#include <cstring>

#include "clouds/drivers/debug_pin.h"

#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/utils/buffer_allocator.h"

#include "clouds/resources.h"

namespace clouds {

using namespace std;
using namespace stmlib;

void GranularProcessor::Init(
    void* large_buffer, size_t large_buffer_size,
    void* small_buffer, size_t small_buffer_size) {
  buffer_[0] = large_buffer;
  buffer_[1] = small_buffer;
  buffer_size_[0] = large_buffer_size;
  buffer_size_[1] = small_buffer_size;
  
  num_channels_ = 2;
  low_fidelity_ = false;
  bypass_ = false;
  
  src_down_.Init();
  src_up_.Init();
  
  ResetFilters();
  
  previous_playback_mode_ = PLAYBACK_MODE_LAST;
  reset_buffers_ = true;
  dry_wet_ = 0.0f;
}

void GranularProcessor::ResetFilters() {
  for (int32_t i = 0; i < 2; ++i) {
    fb_filter_[i].Init();
    lp_filter_[i].Init();
    hp_filter_[i].Init();
  }
}

void GranularProcessor::ProcessGranular(
    FloatFrame* input,
    FloatFrame* output,
    size_t size) {
  // At the exception of the spectral mode, all modes require the incoming
  // audio signal to be written to the recording buffer.
  if (playback_mode_ != PLAYBACK_MODE_SPECTRAL &&
      playback_mode_ != PLAYBACK_MODE_RESONESTOR) {
    const float* input_samples = &input[0].l;
    const bool play = !parameters_.freeze ||
      playback_mode_ == PLAYBACK_MODE_OLIVERB;
    for (int32_t i = 0; i < num_channels_; ++i) {
      if (resolution() == 8) {
        buffer_8_[i].WriteFade(&input_samples[i], size, 2, play);
      } else {
        buffer_16_[i].WriteFade(&input_samples[i], size, 2, play);
      }
    }
  }
  
  switch (playback_mode_) {
    case PLAYBACK_MODE_GRANULAR:
      // In Granular mode, DENSITY is a meta parameter.
      parameters_.granular.use_deterministic_seed = parameters_.density < 0.5f;
      if (parameters_.density >= 0.53f) {
        parameters_.granular.overlap = (parameters_.density - 0.53f) * 2.12f;
      } else if (parameters_.density <= 0.47f) {
        parameters_.granular.overlap = (0.47f - parameters_.density) * 2.12f;
      } else {
        parameters_.granular.overlap = 0.0f;
      }

#ifdef CLOUDS_QUANTIZE_SEMITONES
      // Quantize pitch to closest semitone
      if (parameters_.pitch < 0.5f) parameters_.pitch -= 0.5f;
      parameters_.pitch = static_cast<int>(parameters_.pitch + 0.5f);
#endif

      // And TEXTURE too.
      parameters_.granular.window_shape = parameters_.texture < 0.75f
          ? parameters_.texture * 1.333f : 1.0f;
  
      if (resolution() == 8) {
        player_.Play(buffer_8_, parameters_, &output[0].l, size);
      } else {
        player_.Play(buffer_16_, parameters_, &output[0].l, size);
      }
      break;

    case PLAYBACK_MODE_STRETCH:
      if (resolution() == 8) {
        ws_player_.Play(buffer_8_, parameters_, &output[0].l, size);
      } else {
        ws_player_.Play(buffer_16_, parameters_, &output[0].l, size);
      }
      break;

    case PLAYBACK_MODE_LOOPING_DELAY:
      if (resolution() == 8) {
        looper_.Play(buffer_8_, parameters_, &output[0].l, size);
      } else {
        looper_.Play(buffer_16_, parameters_, &output[0].l, size);
      }
      break;

    case PLAYBACK_MODE_SPECTRAL:
      {
        parameters_.spectral.quantization = parameters_.texture;
        parameters_.spectral.refresh_rate = 0.01f + 0.99f * parameters_.density;
        float warp = parameters_.size - 0.5f;
        parameters_.spectral.warp = 4.0f * warp * warp * warp + 0.5f;
        
        float randomization = parameters_.density - 0.5f;
        randomization *= randomization * 4.2f;
        randomization -= 0.05f;
        CONSTRAIN(randomization, 0.0f, 1.0f);
        parameters_.spectral.phase_randomization = randomization;
        phase_vocoder_.Process(parameters_, input, output, size);
        
        if (num_channels_ == 1) {
          for (size_t i = 0; i < size; ++i) {
            output[i].r = output[i].l;
          }
        }
      }
      break;

    case PLAYBACK_MODE_OLIVERB:
      {

        // Pre-delay, controlled by position or tap tempo sync
        Parameters p = {
          ws_player_.synchronized() ?
          parameters_.position :
          parameters_.position * 0.25f, // position;
          0.1f, // size;
          0.0f, // pitch;
          0.0f, // density;
          0.5f, // texture;
          1.0f, // dry_wet;
          0.0f, // stereo_spread;
          0.0f, // feedback;
          0.0f, // reverb;
          false, // freeze;
          parameters_.trigger, // trigger;
          false // gate;
        };

        if (resolution() == 8) {
          ws_player_.Play(buffer_8_, p, &output[0].l, size);
        } else {
          ws_player_.Play(buffer_16_, p, &output[0].l, size);
        }

        // Settings of the reverb
        oliverb_.set_diffusion(0.3f + 0.5f * parameters_.stereo_spread);
        oliverb_.set_size(0.05f + 0.94f * parameters_.size);
        oliverb_.set_mod_rate(parameters_.feedback);
        oliverb_.set_mod_amount(parameters_.reverb * 300.0f);
        oliverb_.set_ratio(SemitonesToRatio(parameters_.pitch));

        float x = parameters_.pitch;
        const float limit = 0.7f;
        const float slew = 0.4f;

        float wet =
          x < -limit ? 1.0f :
          x < -limit + slew ? 1.0f - (x + limit) / slew:
          x < limit - slew ? 0.0f :
          x < limit ? 1.0f + (x - limit) / slew:
          1.0f;
        oliverb_.set_pitch_shift_amount(wet);

        if (parameters_.freeze) {
          oliverb_.set_input_gain(0.0f);
          oliverb_.set_decay(1.0f);
          oliverb_.set_lp(1.0f);
          oliverb_.set_hp(0.0f);
        } else {
          oliverb_.set_decay(parameters_.density * 1.3f
                           + 0.15f * abs(parameters_.pitch) / 24.0f);
          oliverb_.set_input_gain(0.5f);
          float lp = parameters_.texture < 0.5f ?
            parameters_.texture * 2.0f : 1.0f;
          float hp = parameters_.texture > 0.5f ?
            (parameters_.texture - 0.5f) * 2.0f : 0.0f;
          oliverb_.set_lp(0.03f + 0.9f * lp);
          oliverb_.set_hp(0.01f + 0.2f * hp); // the small offset
                                                  // gets rid of
                                                  // feedback of large
                                                  // DC offset.
        }
        oliverb_.Process(output, size);
      }
      break;

  case PLAYBACK_MODE_RESONESTOR:
    {
      copy(&input[0], &input[size], &output[0]);

      resonestor_.set_pitch(parameters_.pitch);
      resonestor_.set_chord(parameters_.size);
      resonestor_.set_trigger(parameters_.trigger);
      resonestor_.set_burst_damp(parameters_.position);
      resonestor_.set_burst_comb((1.0f - parameters_.position));
      resonestor_.set_burst_duration((1.0f - parameters_.position));
      resonestor_.set_spread_amount(parameters_.reverb);
      resonestor_.set_stereo(parameters_.stereo_spread < 0.5f ? 0.0f :
        (parameters_.stereo_spread - 0.5f) * 2.0f);
      resonestor_.set_separation(parameters_.stereo_spread > 0.5f ? 0.0f :
                                (0.5f - parameters_.stereo_spread) * 2.0f);
      resonestor_.set_freeze(parameters_.freeze);
      resonestor_.set_harmonicity(1.0f - (parameters_.feedback * 0.5f));
      resonestor_.set_distortion(parameters_.dry_wet);

      float t = parameters_.texture;
      if (t < 0.5f) {
        resonestor_.set_narrow(0.001f);
        float l = 1.0f - (0.5f - t) / 0.5f;
        l = l * (1.0f - 0.08f) + 0.08f;
        resonestor_.set_damp(l * l);
      } else {
        resonestor_.set_damp(1.0f);
        float n = (t - 0.5f) / 0.5f * 1.35f;
        n *= n * n * n;
        resonestor_.set_narrow(0.001f + n * n * 0.6f);
      }

      float d = (parameters_.density - 0.05f) / 0.9f;
      if (d < 0.0f) d = 0.0f;
      d *= d * d;
      d *= d * d;
      d *= d * d;
      resonestor_.set_feedback(d * 20.0f);

      resonestor_.Process(output, size);
    }
    break;

    default:
      break;
  }
}

void GranularProcessor::Process(
    ShortFrame* input,
    ShortFrame* output,
    size_t size) {
  // TIC
  if (bypass_) {
    copy(&input[0], &input[size], &output[0]);
    return;
  }
  
  if (silence_ || reset_buffers_ ||
      previous_playback_mode_ != playback_mode_) {
    short* output_samples = &output[0].l;
    fill(&output_samples[0], &output_samples[size << 1], 0);
    return;
  }
  
  // Convert input buffers to float, and mixdown for mono processing.
  for (size_t i = 0; i < size; ++i) {
    in_[i].l = static_cast<float>(input[i].l) / 32768.0f;
    in_[i].r = static_cast<float>(input[i].r) / 32768.0f;
  }

  if (num_channels_ == 1) {
    for (size_t i = 0; i < size; ++i) {
      float xfade = 0.5f;
      // in mono delay modes, stereo spread controls input crossfade
      if (playback_mode_ == PLAYBACK_MODE_LOOPING_DELAY ||
          playback_mode_ == PLAYBACK_MODE_STRETCH)
        xfade = parameters_.stereo_spread;

      in_[i].l = in_[i].l * (1.0f - xfade) + in_[i].r * xfade;
      in_[i].r = in_[i].l;
    }
  }
  
  // Apply feedback, with high-pass filtering to prevent build-ups at very
  // low frequencies (causing large DC swings).
  float feedback = parameters_.feedback;

  if (playback_mode_ != PLAYBACK_MODE_OLIVERB &&
      playback_mode_ != PLAYBACK_MODE_RESONESTOR) {
    ONE_POLE(freeze_lp_, parameters_.freeze ? 1.0f : 0.0f, 0.0005f)
    float cutoff = (20.0f + 100.0f * feedback * feedback) / sample_rate();
    fb_filter_[0].set_f_q<FREQUENCY_FAST>(cutoff, 0.75f);
    fb_filter_[1].set(fb_filter_[0]);
    fb_filter_[0].Process<FILTER_MODE_HIGH_PASS>(&fb_[0].l, &fb_[0].l, size, 2);
    fb_filter_[1].Process<FILTER_MODE_HIGH_PASS>(&fb_[0].r, &fb_[0].r, size, 2);
    float fb_gain = feedback * (2.0f - feedback) * (1.0f - freeze_lp_);
    for (size_t i = 0; i < size; ++i) {
      in_[i].l += fb_gain * (
                             SoftLimit(fb_gain * 1.4f * fb_[i].l + in_[i].l) - in_[i].l);
      in_[i].r += fb_gain * (
                             SoftLimit(fb_gain * 1.4f * fb_[i].r + in_[i].r) - in_[i].r);
    }
  }
  
  if (low_fidelity_) {
    size_t downsampled_size = size / kDownsamplingFactor;
    src_down_.Process(in_, in_downsampled_,size);
    ProcessGranular(in_downsampled_, out_downsampled_, downsampled_size);
    src_up_.Process(out_downsampled_, out_, downsampled_size);
  } else {
    ProcessGranular(in_, out_, size);
  }
  
  // Diffusion and pitch-shifting post-processings.
  if (playback_mode_ != PLAYBACK_MODE_SPECTRAL &&
      playback_mode_ != PLAYBACK_MODE_OLIVERB &&
      playback_mode_ != PLAYBACK_MODE_RESONESTOR) {
    float texture = parameters_.texture;
    float diffusion = playback_mode_ == PLAYBACK_MODE_GRANULAR 
        ? texture > 0.75f ? (texture - 0.75f) * 4.0f : 0.0f
        : parameters_.density;
    diffuser_.set_amount(diffusion);
    diffuser_.Process(out_, size);
  }

  if (playback_mode_ == PLAYBACK_MODE_LOOPING_DELAY &&
      (!parameters_.freeze || looper_.synchronized())) {
    pitch_shifter_.set_ratio(SemitonesToRatio(parameters_.pitch));
    pitch_shifter_.set_size(parameters_.size);
    float x = parameters_.pitch;
    const float limit = 0.7f;
    const float slew = 0.4f;
    float wet =
      x < -limit ? 1.0f :
      x < -limit + slew ? 1.0f - (x + limit) / slew:
      x < limit - slew ? 0.0f :
      x < limit ? 1.0f + (x - limit) / slew:
      1.0f;
    pitch_shifter_.set_dry_wet(wet);
    pitch_shifter_.Process(out_, size);
  }
  
  // Apply filters.
  if (playback_mode_ == PLAYBACK_MODE_LOOPING_DELAY ||
      playback_mode_ == PLAYBACK_MODE_STRETCH) {
    float cutoff = parameters_.texture;
    float lp_cutoff = 0.5f * SemitonesToRatio(
        (cutoff < 0.5f ? cutoff - 0.5f : 0.0f) * 216.0f);
    float hp_cutoff = 0.25f * SemitonesToRatio(
        (cutoff < 0.5f ? -0.5f : cutoff - 1.0f) * 216.0f);
    CONSTRAIN(lp_cutoff, 0.0f, 0.499f);
    CONSTRAIN(hp_cutoff, 0.0f, 0.499f);

    lp_filter_[0].set_f_q<FREQUENCY_FAST>(lp_cutoff, 0.9f);
    lp_filter_[0].Process<FILTER_MODE_LOW_PASS>(
        &out_[0].l, &out_[0].l, size, 2);

    lp_filter_[1].set(lp_filter_[0]);
    lp_filter_[1].Process<FILTER_MODE_LOW_PASS>(
        &out_[0].r, &out_[0].r, size, 2);

    hp_filter_[0].set_f_q<FREQUENCY_FAST>(hp_cutoff, 0.9f);
    hp_filter_[0].Process<FILTER_MODE_HIGH_PASS>(
        &out_[0].l, &out_[0].l, size, 2);

    hp_filter_[1].set(hp_filter_[0]);
    hp_filter_[1].Process<FILTER_MODE_HIGH_PASS>(
        &out_[0].r, &out_[0].r, size, 2);
  }
  
  // This is what is fed back. Reverb is not fed back.
  copy(&out_[0], &out_[size], &fb_[0]);

  const float post_gain = 1.2f;

  if (playback_mode_ != PLAYBACK_MODE_RESONESTOR) {
    ParameterInterpolator dry_wet_mod(&dry_wet_, parameters_.dry_wet, size);
    for (size_t i = 0; i < size; ++i) {
      float dry_wet = dry_wet_mod.Next();
      float fade_in = Interpolate(lut_xfade_in, dry_wet, 16.0f);
      float fade_out = Interpolate(lut_xfade_out, dry_wet, 16.0f);
      float l = static_cast<float>(input[i].l) / 32768.0f;
      float r = static_cast<float>(input[i].r) / 32768.0f;
      out_[i].l = l * fade_out + out_[i].l * post_gain * fade_in;
      out_[i].r = r * fade_out + out_[i].r * post_gain * fade_in;
    }
  }

  // Apply the simple post-processing reverb.
  if (playback_mode_ != PLAYBACK_MODE_OLIVERB &&
      playback_mode_ != PLAYBACK_MODE_RESONESTOR) {
    float reverb_amount = parameters_.reverb;

    reverb_.set_amount(reverb_amount * 0.54f);
    reverb_.set_diffusion(0.7f);
    reverb_.set_time(0.35f + 0.63f * reverb_amount);
    reverb_.set_input_gain(0.2f);
    reverb_.set_lp(0.6f + 0.37f * feedback);

    reverb_.Process(out_, size);
  }

  for (size_t i = 0; i < size; ++i) {
    output[i].l = SoftConvert(out_[i].l);
    output[i].r = SoftConvert(out_[i].r);
  }

  // TOC
}

void GranularProcessor::PreparePersistentData() {
  persistent_state_.write_head[0] = low_fidelity_ ?
      buffer_8_[0].head() : buffer_16_[0].head();
  persistent_state_.write_head[1] = low_fidelity_ ?
      buffer_8_[1].head() : buffer_16_[1].head();
  persistent_state_.quality = quality();
  persistent_state_.spectral = playback_mode() == PLAYBACK_MODE_SPECTRAL;
}

void GranularProcessor::GetPersistentData(
      PersistentBlock* block, size_t *num_blocks) {
  PersistentBlock* first_block = block;
  
  block->tag = FourCC<'s', 't', 'a', 't'>::value;
  block->data = &persistent_state_;
  block->size = sizeof(PersistentState);
  ++block;

  // Create save block holding the audio buffers.
  for (int32_t i = 0; i < num_channels_; ++i) {
    block->tag = FourCC<'b', 'u', 'f', 'f'>::value;
    block->data = buffer_[i];
    block->size = buffer_size_[num_channels_ - 1];
    ++block;
  }
  *num_blocks = block - first_block;
}

bool GranularProcessor::LoadPersistentData(const uint32_t* data) {
  // Force a silent output while the swapping of buffers takes place.
  silence_ = true;
  
  PersistentBlock block[4];
  size_t num_blocks;
  GetPersistentData(block, &num_blocks);
  
  for (size_t i = 0; i < num_blocks; ++i) {
    // Check that the format is correct.
    if (block[i].tag != data[0] || block[i].size != data[1]) {
      silence_ = false;
      return false;
    }
    
    // All good. Load the data. 2 words have already been used for the block tag
    // and the block size.
    data += 2;
    memcpy(block[i].data, data, block[i].size);
    data += block[i].size / sizeof(uint32_t);
    
    if (i == 0) {
      // We now know from which mode the data was saved.
      bool currently_spectral = playback_mode_ == PLAYBACK_MODE_SPECTRAL;
      bool requires_spectral = persistent_state_.spectral;
      if (currently_spectral ^ requires_spectral) {
        set_playback_mode(requires_spectral
            ? PLAYBACK_MODE_SPECTRAL
            : PLAYBACK_MODE_GRANULAR);
      }
      set_quality(persistent_state_.quality);

      // We can force a switch to this mode, and once everything has been
      // initialized for this mode, we continue with the loop to copy the
      // actual buffer data - with all state variables correctly initialized.
      Prepare();
      GetPersistentData(block, &num_blocks);
    }
  }
  
  // We can finally reset the position of the write heads.
  if (low_fidelity_) {
    buffer_8_[0].Resync(persistent_state_.write_head[0]);
    buffer_8_[1].Resync(persistent_state_.write_head[1]);
  } else {
    buffer_16_[0].Resync(persistent_state_.write_head[0]);
    buffer_16_[1].Resync(persistent_state_.write_head[1]);
  }
  parameters_.freeze = true;
  silence_ = false;
  return true;
}

void GranularProcessor::Prepare() {
  bool playback_mode_changed = previous_playback_mode_ != playback_mode_;
  bool benign_change = previous_playback_mode_ != PLAYBACK_MODE_SPECTRAL
    && playback_mode_ != PLAYBACK_MODE_SPECTRAL
    && playback_mode_ != PLAYBACK_MODE_RESONESTOR
    && previous_playback_mode_ != PLAYBACK_MODE_RESONESTOR
    && playback_mode_ != PLAYBACK_MODE_OLIVERB
    && previous_playback_mode_ != PLAYBACK_MODE_OLIVERB
    && previous_playback_mode_ != PLAYBACK_MODE_LAST;
  
  if (!reset_buffers_ && playback_mode_changed && benign_change) {
    ResetFilters();
    pitch_shifter_.Clear();
    previous_playback_mode_ = playback_mode_;
  }
  
  if ((playback_mode_changed && !benign_change) || reset_buffers_) {
    parameters_.freeze = false;
  }

  if (reset_buffers_ || (playback_mode_changed && !benign_change)) {
    void* buffer[2];
    size_t buffer_size[2];
    void* workspace;
    size_t workspace_size;
    if (num_channels_ == 1) {
      // Large buffer: 120k of sample memory.
      // small buffer: fully allocated to FX workspace.
      buffer[0] = buffer_[0];
      buffer_size[0] = buffer_size_[0];
      buffer[1] = NULL;
      buffer_size[1] = 0;
      workspace = buffer_[1];
      workspace_size = buffer_size_[1];
    } else {
      // Large buffer: 64k of sample memory + FX workspace.
      // small buffer: 64k of sample memory.
      buffer_size[0] = buffer_size[1] = buffer_size_[1];
      buffer[0] = buffer_[0];
      buffer[1] = buffer_[1];
      
      workspace_size = buffer_size_[0] - buffer_size_[1];
      workspace = static_cast<uint8_t*>(buffer[0]) + buffer_size[0];
    }
    float sr = sample_rate();

    BufferAllocator allocator(workspace, workspace_size);
    diffuser_.Init(allocator.Allocate<float>(2048));

    uint16_t* reverb_buffer = allocator.Allocate<uint16_t>(16384);
    if (playback_mode_ == PLAYBACK_MODE_OLIVERB) {
      oliverb_.Init(reverb_buffer);
    } else {
      reverb_.Init(reverb_buffer);
    }

    size_t correlator_block_size = (kMaxWSOLASize / 32) + 2;
    uint32_t* correlator_data = allocator.Allocate<uint32_t>(
        correlator_block_size * 3);
    correlator_.Init(
        &correlator_data[0],
        &correlator_data[correlator_block_size]);
    pitch_shifter_.Init((uint16_t*)correlator_data);
    
    if (playback_mode_ == PLAYBACK_MODE_SPECTRAL) {
      phase_vocoder_.Init(
          buffer, buffer_size,
          lut_sine_window_4096, 4096,
          num_channels_, resolution(), sr);
    } else if (playback_mode_ == PLAYBACK_MODE_RESONESTOR) {
      float* buf = (float*)buffer[0];
      resonestor_.Init(buf);
    } else {
      for (int32_t i = 0; i < num_channels_; ++i) {
        if (resolution() == 8) {
          buffer_8_[i].Init(
              buffer[i],
              (buffer_size[i]),
              tail_buffer_[i]);
        } else {
          buffer_16_[i].Init(
              buffer[i],
              ((buffer_size[i]) >> 1),
              tail_buffer_[i]);
        }
      }
      int32_t num_grains = (num_channels_ == 1 ? 32 : 26) * \
          (low_fidelity_ ? 20 : 16) >> 4;
      player_.Init(num_channels_, num_grains);
      ws_player_.Init(&correlator_, num_channels_);
      looper_.Init(num_channels_);
    }
    reset_buffers_ = false;
    previous_playback_mode_ = playback_mode_;
  }
  
  if (playback_mode_ == PLAYBACK_MODE_SPECTRAL) {
    phase_vocoder_.Buffer();
  } else if (playback_mode_ == PLAYBACK_MODE_STRETCH ||
             playback_mode_ == PLAYBACK_MODE_OLIVERB) {
    if (resolution() == 8) {
      ws_player_.LoadCorrelator(buffer_8_);
    } else {
      ws_player_.LoadCorrelator(buffer_16_);
    }
    correlator_.EvaluateSomeCandidates();
  }
}

}  // namespace clouds
