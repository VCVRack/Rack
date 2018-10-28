// Copyright 2017 Olivier Gillet.
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
// Keeps track of the state of the chain of modules.

#ifndef STAGES_CHAIN_STATE_H_
#define STAGES_CHAIN_STATE_H_

#include "stmlib/stmlib.h"

#include "stages/io_buffer.h"
#include "stages/segment_generator.h"

namespace stages {

const size_t kMaxChainSize = 6;
const size_t kMaxNumChannels = kMaxChainSize * kNumChannels;
const size_t kPacketSize = 24;

class SerialLink;
class Settings;

class ChainState {
 public:
  ChainState() { }
  ~ChainState() { }
  
  typedef uint8_t ChannelBitmask;
  
  void Init(SerialLink* left, SerialLink* right);
  void Update(
      const IOBuffer::Block& block,
      Settings* settings,
      SegmentGenerator* segment_generator,
      SegmentGenerator::Output* out);
  
  // Index of the module in the chain, and size of the chain.
  inline size_t index() const { return index_; }
  inline size_t size() const { return size_; }
  
  inline bool discovering_neighbors() const { return discovering_neighbors_; }
  inline bool ouroboros() const { return ouroboros_; }
  
  // Internally, we only store a loop bit for each channel - but the UI needs
  // to know more than that. It needs to know whether a channel with a loop bit
  // set to 1 is a loop start, a loop end, or self-looping channel. This
  // extra information is called the LoopStatus. It is stored in the ChainState,
  // (in loop_status_[]) and populated everytime we reconfigure a channel.
  enum LoopStatus {
    LOOP_STATUS_NONE,
    LOOP_STATUS_START,
    LOOP_STATUS_END,
    LOOP_STATUS_SELF
  };

  inline LoopStatus loop_status(size_t i) const {
    return loop_status_[i];
  }

  inline void set_local_switch_pressed(ChannelBitmask bitmask) {
    switch_pressed_[index_] = bitmask;
  }
  
 private:
  void DiscoverNeighbors();

  void TransmitRight();
  void TransmitLeft();
  void ReceiveRight();
  void ReceiveLeft();
  
  void UpdateLocalState(
      const IOBuffer::Block& block,
      const Settings& settings,
      const SegmentGenerator::Output& last_out);
  void UpdateLocalPotCvSlider(const IOBuffer::Block& block);
  void Configure(SegmentGenerator* segment_generator);
  void PollSwitches();
  void BindRemoteParameters(SegmentGenerator* segment_generator);
  void BindLocalParameters(
      const IOBuffer::Block& block, SegmentGenerator* segment_generator);
  void HandleRequest(Settings* settings);
  
  struct ChannelState {
    // 7 6 5 4 3 2 1 0
    // 8 4 2 1 8 4 2 1
    //
    // S S S S I L T T
    //
    // SSSS: index of the module sending this packet.
    // I: gate/trigger input patched?
    // L: loop enabled?
    // TT: segment type
    uint8_t flags;
    uint8_t pot;
    uint16_t cv_slider;
    
    inline bool input_patched() const {
      return flags & 0x08;
    }
    
    inline segment::Configuration configuration() const {
      segment::Configuration c;
      c.loop = flags & 0x04;
      c.type = segment::Type(flags & 0x03);
      return c;
    }
    
    inline size_t index() const {
      return size_t(flags) >> 4;
    }
    
    inline bool UpdateFlags(
        uint8_t index,
        uint8_t configuration,
        bool input_patched) {
      uint8_t new_flags = index << 4;
      new_flags |= configuration;
      new_flags |= input_patched ? 0x08 : 0;
      bool dirty = new_flags != flags;
      flags = new_flags;
      return dirty;
    }
  };
  
  struct Loop {
    int8_t start;
    int8_t end;
  };
  
  struct LeftToRightPacket {
    uint8_t last_patched_channel;
    int8_t segment;
    float phase;
    Loop last_loop;
    ChannelBitmask switch_pressed[kMaxChainSize];
    ChannelBitmask input_patched[kMaxChainSize];
  };
  
  struct RightToLeftPacket {
    ChannelState channel[kNumChannels];
  };
  
  enum Request {
    REQUEST_NONE,
    REQUEST_SET_SEGMENT_TYPE = 0xfe,
    REQUEST_SET_LOOP = 0xff
  };
  
  struct RequestPacket {
    uint8_t request;
    uint8_t argument[4];
  };
  
  struct DiscoveryPacket {
    uint32_t key;
    uint8_t counter;
  };
  
  union Packet {
    RightToLeftPacket to_left;
    LeftToRightPacket to_right;
    DiscoveryPacket discovery;
    RequestPacket request;
    uint8_t bytes[kPacketSize];
  };
  
  struct ParameterBinding {
    size_t generator;
    size_t source;
    size_t destination;
  };
  
  inline size_t remote_channel_index(size_t i, size_t j) const {
    return i * kNumChannels + j;
  }
  
  inline size_t local_channel_index(size_t i) const {
    return index_ * kNumChannels + i;
  }
  
  ChannelState* local_channel(size_t i) {
    return &channel_state_[local_channel_index(i)];
  }
  
  ChannelState* remote_channel(size_t i, size_t j) {
    return &channel_state_[remote_channel_index(i, j)];
  }
  
  inline void set_loop_status(int channel, int segment, Loop loop) {
    if (segment == loop.start) {
      loop_status_[channel] = segment == loop.end ?
          LOOP_STATUS_SELF : LOOP_STATUS_START;
    } else if (segment == loop.end) {
      loop_status_[channel] = LOOP_STATUS_END;
    } else {
      loop_status_[channel] = LOOP_STATUS_NONE;
    }
  }
  
  RequestPacket MakeLoopChangeRequest(size_t loop_start, size_t loop_end);
  
  size_t index_;
  size_t size_;
  
  SerialLink* left_;
  SerialLink* right_;
  
  ChannelState channel_state_[kMaxNumChannels];
  bool dirty_[kMaxNumChannels];

  int16_t switch_press_time_[kMaxNumChannels];
  uint16_t unpatch_counter_[kNumChannels];
  LoopStatus loop_status_[kNumChannels];

  ChannelBitmask switch_pressed_[kMaxChainSize];
  ChannelBitmask input_patched_[kMaxChainSize];
  
  size_t rx_last_patched_channel_;
  size_t tx_last_patched_channel_;
  Loop rx_last_loop_;
  Loop tx_last_loop_;
  SegmentGenerator::Output rx_last_sample_;
  SegmentGenerator::Output tx_last_sample_;

  RequestPacket request_;
  
  bool discovering_neighbors_;
  bool ouroboros_;
  uint32_t counter_;
  
  Packet left_tx_packet_;
  Packet right_tx_packet_;
  Packet left_rx_packet_[2];
  Packet right_rx_packet_[2];
  
  size_t num_internal_bindings_;
  size_t num_bindings_;
  ParameterBinding binding_[kMaxNumChannels];
  
  DISALLOW_COPY_AND_ASSIGN(ChainState);
};

}  // namespace stages

#endif  // STAGES_CHAIN_STATE_H_
