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
  
  num_channels_ = 1;
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
  }
}

void GranularProcessor::ProcessGranular(
    FloatFrame* input,
    FloatFrame* output,
    size_t size) {
  
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
  
  ProcessGranular(in_, out_, size);
  
    
  // This is what is fed back. Reverb is not fed back.
  copy(&out_[0], &out_[size], &fb_[0]);
  
  
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
  persistent_state_.write_head[0] = buffer_16_[0].head();
  persistent_state_.write_head[1] = buffer_16_[1].head();
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
        set_playback_mode(PLAYBACK_MODE_SPECTRAL);
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
  buffer_16_[0].Resync(persistent_state_.write_head[0]);
  buffer_16_[1].Resync(persistent_state_.write_head[1]);
  parameters_.freeze = true;
  silence_ = false;
  return true;
}

void GranularProcessor::Prepare() {
  bool playback_mode_changed = previous_playback_mode_ != playback_mode_;
  bool benign_change = false;
  
  if (!reset_buffers_ && playback_mode_changed && benign_change) {
    ResetFilters();
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
    
    phase_vocoder_.Init(
        buffer, buffer_size,
        lut_sine_window_4096, 4096,
        num_channels_, resolution(), sr);
    reset_buffers_ = false;
    previous_playback_mode_ = playback_mode_;
  }
  
  phase_vocoder_.Buffer();
}

}  // namespace clouds
