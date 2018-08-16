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
  if (playback_mode_ != PLAYBACK_MODE_SPECTRAL) {
    const float* input_samples = &input[0].l;
    for (int32_t i = 0; i < num_channels_; ++i) {
      if (resolution() == 8) {
        buffer_8_[i].WriteFade(
            &input_samples[i], size, 2, !parameters_.freeze);
      } else {
        buffer_16_[i].WriteFade(
            &input_samples[i], size, 2, !parameters_.freeze);
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
      in_[i].l = (in_[i].l + in_[i].r) * 0.5f;
      in_[i].r = in_[i].l;
    }
  }
  
  // Apply feedback, with high-pass filtering to prevent build-ups at very
  // low frequencies (causing large DC swings).
  ONE_POLE(freeze_lp_, parameters_.freeze ? 1.0f : 0.0f, 0.0005f)
  float feedback = parameters_.feedback;
  float cutoff = (20.0f + 100.0f * feedback * feedback) / sample_rate();
  fb_filter_[0].set_f_q<FREQUENCY_FAST>(cutoff, 1.0f);
  fb_filter_[1].set(fb_filter_[0]);
  fb_filter_[0].Process<FILTER_MODE_HIGH_PASS>(&fb_[0].l, &fb_[0].l, size, 2);
  fb_filter_[1].Process<FILTER_MODE_HIGH_PASS>(&fb_[0].r, &fb_[0].r, size, 2);
  float fb_gain = feedback * (1.0f - freeze_lp_);
  for (size_t i = 0; i < size; ++i) {
    in_[i].l += fb_gain * (
        SoftLimit(fb_gain * 1.4f * fb_[i].l + in_[i].l) - in_[i].l);
    in_[i].r += fb_gain * (
        SoftLimit(fb_gain * 1.4f * fb_[i].r + in_[i].r) - in_[i].r);
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
  if (playback_mode_ != PLAYBACK_MODE_SPECTRAL) {
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
    float lpq = 1.0f + 3.0f * (1.0f - feedback) * (0.5f - lp_cutoff);
    lp_filter_[0].set_f_q<FREQUENCY_FAST>(lp_cutoff, lpq);
    lp_filter_[0].Process<FILTER_MODE_LOW_PASS>(
        &out_[0].l, &out_[0].l, size, 2);

    lp_filter_[1].set(lp_filter_[0]);
    lp_filter_[1].Process<FILTER_MODE_LOW_PASS>(
        &out_[0].r, &out_[0].r, size, 2);

    hp_filter_[0].set_f_q<FREQUENCY_FAST>(hp_cutoff, 1.0f);
    hp_filter_[0].Process<FILTER_MODE_HIGH_PASS>(
        &out_[0].l, &out_[0].l, size, 2);

    hp_filter_[1].set(hp_filter_[0]);
    hp_filter_[1].Process<FILTER_MODE_HIGH_PASS>(
        &out_[0].r, &out_[0].r, size, 2);
  }
  
  // This is what is fed back. Reverb is not fed back.
  copy(&out_[0], &out_[size], &fb_[0]);
  
  // Apply reverb.
  float reverb_amount = parameters_.reverb * 0.95f;
  reverb_amount += feedback * (2.0f - feedback) * freeze_lp_;
  CONSTRAIN(reverb_amount, 0.0f, 1.0f);
  
  reverb_.set_amount(reverb_amount * 0.54f);
  reverb_.set_diffusion(0.7f);
  reverb_.set_time(0.35f + 0.63f * reverb_amount);
  reverb_.set_input_gain(0.2f);
  reverb_.set_lp(0.6f + 0.37f * feedback);
  reverb_.Process(out_, size);
  
  const float post_gain = 1.2f;
  ParameterInterpolator dry_wet_mod(&dry_wet_, parameters_.dry_wet, size);
  for (size_t i = 0; i < size; ++i) {
    float dry_wet = dry_wet_mod.Next();
    float fade_in = Interpolate(lut_xfade_in, dry_wet, 16.0f);
    float fade_out = Interpolate(lut_xfade_out, dry_wet, 16.0f);
    float l = static_cast<float>(input[i].l) / 32768.0f * fade_out;
    float r = static_cast<float>(input[i].r) / 32768.0f * fade_out;
    l += out_[i].l * post_gain * fade_in;
    r += out_[i].r * post_gain * fade_in;
    output[i].l = SoftConvert(l);
    output[i].r = SoftConvert(r);
  }
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
    reverb_.Init(allocator.Allocate<uint16_t>(16384));
    
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
      int32_t num_grains = (num_channels_ == 1 ? 40 : 32) * \
          (low_fidelity_ ? 23 : 16) >> 4;
      player_.Init(num_channels_, num_grains);
      ws_player_.Init(&correlator_, num_channels_);
      looper_.Init(num_channels_);
    }
    reset_buffers_ = false;
    previous_playback_mode_ = playback_mode_;
  }
  
  if (playback_mode_ == PLAYBACK_MODE_SPECTRAL) {
    phase_vocoder_.Buffer();
  } else if (playback_mode_ == PLAYBACK_MODE_STRETCH) {
    if (resolution() == 8) {
      ws_player_.LoadCorrelator(buffer_8_);
    } else {
      ws_player_.LoadCorrelator(buffer_16_);
    }
    correlator_.EvaluateSomeCandidates();
  }
}

}  // namespace clouds
