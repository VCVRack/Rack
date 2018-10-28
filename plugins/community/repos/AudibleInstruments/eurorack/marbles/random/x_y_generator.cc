// Copyright 2015 Olivier Gillet.
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
// Generator for the X/Y outputs.

#include "marbles/random/x_y_generator.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"

namespace marbles {

using namespace std;
using namespace stmlib;

void XYGenerator::Init(RandomStream* random_stream, float sr) {
  for (size_t i = 0; i < kNumChannels; ++i) {
    random_sequence_[i].Init(random_stream);
    output_channel_[i].Init();
  }
  ramp_extractor_.Init(8000.0f / sr);
  ramp_divider_.Init();
  external_clock_stabilization_counter_ = 16;
}

const uint32_t hashes[kNumXChannels] = {
  0, 0xbeca55e5, 0xf0cacc1a
};

void XYGenerator::Process(
    ClockSource clock_source,
    const GroupSettings& x_settings,
    const GroupSettings& y_settings,
    const GateFlags* external_clock,
    const Ramps& ramps,
    float* output,
    size_t size) {
  float* channel_ramp[kNumChannels];
  
  if (clock_source != CLOCK_SOURCE_EXTERNAL) {
    // For a couple of upcoming blocks, we'll still be receiving garbage from
    // the normalization pin that we need to ignore.
    external_clock_stabilization_counter_ = 16;
  } else {
    if (external_clock_stabilization_counter_) {
      --external_clock_stabilization_counter_;
      if (external_clock_stabilization_counter_ == 0) {
        ramp_extractor_.Reset();
      }
    }
  }
  
  switch (clock_source) {
    case CLOCK_SOURCE_EXTERNAL:
      {
        Ratio r = { 1, 1 };
        ramp_extractor_.Process(r, false, external_clock, ramps.slave[0], size);
        if (external_clock_stabilization_counter_) {
          fill(&ramps.slave[0][0], &ramps.slave[0][size], 0.0f);
        }
      }
      channel_ramp[0] = ramps.slave[0];
      channel_ramp[1] = ramps.slave[0];
      channel_ramp[2] = ramps.slave[0];
      break;
      
    case CLOCK_SOURCE_INTERNAL_T1:
      channel_ramp[0] = ramps.slave[0];
      channel_ramp[1] = ramps.slave[0];
      channel_ramp[2] = ramps.slave[0];
      break;

    case CLOCK_SOURCE_INTERNAL_T2:
      channel_ramp[0] = ramps.master;
      channel_ramp[1] = ramps.master;
      channel_ramp[2] = ramps.master;
      break;
      
    case CLOCK_SOURCE_INTERNAL_T3:
      channel_ramp[0] = ramps.slave[1];
      channel_ramp[1] = ramps.slave[1];
      channel_ramp[2] = ramps.slave[1];
      break;
      
    default:
      channel_ramp[0] = ramps.slave[0];
      channel_ramp[1] = ramps.master;
      channel_ramp[2] = ramps.slave[1];
      break;
  }
  
  ramp_divider_.Process(y_settings.ratio, channel_ramp[1], ramps.external, size);
  channel_ramp[kNumChannels - 1] = ramps.external;
  
  for (size_t i = 0; i < kNumChannels; ++i) {
    OutputChannel& channel = output_channel_[i];
    const GroupSettings& settings = i < kNumXChannels ? x_settings : y_settings;
    
    switch (settings.voltage_range) {
      case VOLTAGE_RANGE_NARROW:
        channel.set_scale_offset(ScaleOffset(2.0f, 0.0f));
        break;
      
      case VOLTAGE_RANGE_POSITIVE:
        channel.set_scale_offset(ScaleOffset(5.0f, 0.0f));
        break;
      
      case VOLTAGE_RANGE_FULL:
        channel.set_scale_offset(ScaleOffset(10.0f, -5.0f));
        break;
      
      default:
        break;
    }
    
    float amount = 1.0f;
    if (settings.control_mode == CONTROL_MODE_BUMP) {
      amount = i == kNumXChannels / 2 ? 1.0f : -1.0f;
    } else if (settings.control_mode == CONTROL_MODE_TILT) {
      amount = 2.0f * static_cast<float>(i) / float(kNumXChannels - 1) - 1.0f;
    }
    
    channel.set_spread(0.5f + (settings.spread - 0.5f) * amount);
    channel.set_bias(0.5f + (settings.bias - 0.5f) * amount);
    channel.set_steps(0.5f + (settings.steps - 0.5f) * \
        (settings.register_mode ? 1.0f : amount));
    channel.set_scale_index(settings.scale_index);
    channel.set_register_mode(settings.register_mode);
    channel.set_register_value(settings.register_value);
    channel.set_register_transposition(
        4.0f * settings.spread * (settings.bias - 0.5f) * amount);
    
    RandomSequence* sequence = &random_sequence_[i];
    sequence->Record();
    sequence->set_length(settings.length);
    sequence->set_deja_vu(settings.deja_vu);
    
    // When all channels follow the same clock, the deja-vu random looping will
    // follow the same pattern and the constant-mode input will be shifted!
    if (clock_source != CLOCK_SOURCE_INTERNAL_T1_T2_T3
        && i > 0 && i < kNumXChannels) {
      sequence = &random_sequence_[0];
      if (settings.register_mode) {
        if (settings.control_mode == CONTROL_MODE_IDENTICAL) {
          sequence->ReplayShifted(i);
        } else if (settings.control_mode == CONTROL_MODE_BUMP) {
          sequence->ReplayShifted(i == 2 ? 1 : 0);
        } else {
          sequence->ReplayShifted(0);
        }
      } else {
        sequence->ReplayPseudoRandom(hashes[i]);
      }
    }
    
    channel.Process(sequence, channel_ramp[i], &output[i], size, kNumChannels);
  }
}

}  // namespace marbles