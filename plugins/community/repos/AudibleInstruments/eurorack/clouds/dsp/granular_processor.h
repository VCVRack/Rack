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

#ifndef CLOUDS_DSP_GRANULAR_PROCESSOR_H_
#define CLOUDS_DSP_GRANULAR_PROCESSOR_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/filter.h"

#include "clouds/dsp/correlator.h"
#include "clouds/dsp/frame.h"
#include "clouds/dsp/fx/diffuser.h"
#include "clouds/dsp/fx/pitch_shifter.h"
#include "clouds/dsp/fx/reverb.h"
#include "clouds/dsp/granular_processor.h"
#include "clouds/dsp/granular_sample_player.h"
#include "clouds/dsp/looping_sample_player.h"
#include "clouds/dsp/pvoc/phase_vocoder.h"
#include "clouds/dsp/sample_rate_converter.h"
#include "clouds/dsp/wsola_sample_player.h"

namespace clouds {

const int32_t kDownsamplingFactor = 2;

enum PlaybackMode {
  PLAYBACK_MODE_GRANULAR,
  PLAYBACK_MODE_STRETCH,
  PLAYBACK_MODE_LOOPING_DELAY,
  PLAYBACK_MODE_SPECTRAL,
  PLAYBACK_MODE_LAST
};

// State of the recording buffer as saved in one of the 4 sample memories.
struct PersistentState {
  int32_t write_head[2];
  uint8_t quality;
  uint8_t spectral;
};

// Data block as saved in one of the 4 sample memories.
struct PersistentBlock {
  uint32_t tag;
  uint32_t size;
  void* data;
};

class GranularProcessor {
 public:
  GranularProcessor() { }
  ~GranularProcessor() { }
  
  void Init(
      void* large_buffer,
      size_t large_buffer_size,
      void* small_buffer,
      size_t small_buffer_size);

  void Process(ShortFrame* input, ShortFrame* output, size_t size);
  void Prepare();
  
  inline Parameters* mutable_parameters() {
    return &parameters_;
  }

  inline const Parameters& parameters() const {
    return parameters_;
  }
  
  inline void ToggleFreeze() {
    parameters_.freeze = !parameters_.freeze;
  }
  
  inline void set_freeze(bool freeze) {
    parameters_.freeze = freeze;
  }

  inline bool frozen() const {
    return parameters_.freeze;
  }

  inline void set_silence(bool silence) {
    silence_ = silence;
  }
  
  inline void set_bypass(bool bypass) {
    bypass_ = bypass;
  }
  
  inline bool bypass() const {
    return bypass_;
  }
  
  inline void set_playback_mode(PlaybackMode playback_mode) {
    playback_mode_ = playback_mode;
  }
  
  inline PlaybackMode playback_mode() const { return playback_mode_; }
  
  inline void set_quality(int32_t quality) {
    set_num_channels(quality & 1 ? 1 : 2);
    set_low_fidelity(quality >> 1 ? true : false);
  }
  
  inline void set_num_channels(int32_t num_channels) {
    reset_buffers_ = reset_buffers_ || num_channels_ != num_channels;
    num_channels_ = num_channels;
  }
  
  inline void set_low_fidelity(bool low_fidelity) {
    reset_buffers_ = reset_buffers_ || low_fidelity != low_fidelity_;
    low_fidelity_ = low_fidelity;
  }
  
  inline int32_t quality() const {
    int32_t quality = 0;
    if (num_channels_ == 1) quality |= 1;
    if (low_fidelity_) quality |= 2;
    return quality;
  }
  
  void GetPersistentData(PersistentBlock* block, size_t *num_blocks);
  bool LoadPersistentData(const uint32_t* data);
  void PreparePersistentData();

 private:
  inline int32_t resolution() const {
    return low_fidelity_ ? 8 : 16;
  }

  inline float sample_rate() const {
    return 32000.0f / \
        (low_fidelity_ ? kDownsamplingFactor : 1);
  }
     
  void ResetFilters();
  void ProcessGranular(FloatFrame* input, FloatFrame* output, size_t size);

  PlaybackMode playback_mode_;
  PlaybackMode previous_playback_mode_;
  int32_t num_channels_;
  bool low_fidelity_;
  
  bool silence_;
  bool bypass_;
  bool reset_buffers_;
  float freeze_lp_;
  float dry_wet_;
  
  void* buffer_[2];
  size_t buffer_size_[2];
  
  Correlator correlator_;
  
  GranularSamplePlayer player_;
  WSOLASamplePlayer ws_player_;
  LoopingSamplePlayer looper_;
  PhaseVocoder phase_vocoder_;
  
  Diffuser diffuser_;
  Reverb reverb_;
  PitchShifter pitch_shifter_;
  stmlib::Svf fb_filter_[2];
  stmlib::Svf hp_filter_[2];
  stmlib::Svf lp_filter_[2];
  
  AudioBuffer<RESOLUTION_8_BIT_MU_LAW> buffer_8_[2];
  AudioBuffer<RESOLUTION_16_BIT> buffer_16_[2];
  
  FloatFrame in_[kMaxBlockSize];
  FloatFrame in_downsampled_[kMaxBlockSize / kDownsamplingFactor];
  FloatFrame out_downsampled_[kMaxBlockSize / kDownsamplingFactor];
  FloatFrame out_[kMaxBlockSize];
  FloatFrame fb_[kMaxBlockSize];
  
  int16_t tail_buffer_[2][256];
  
  Parameters parameters_;
  
  SampleRateConverter<-kDownsamplingFactor, 45, src_filter_1x_2_45> src_down_;
  SampleRateConverter<+kDownsamplingFactor, 45, src_filter_1x_2_45> src_up_;
  
  PersistentState persistent_state_;
  
  DISALLOW_COPY_AND_ASSIGN(GranularProcessor);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_GRANULAR_PROCESSOR_H_
