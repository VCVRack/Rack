// Copyright 2013 Olivier Gillet.
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
// Midi event handler.

#ifndef YARNS_MIDI_HANDLER_H_
#define YARNS_MIDI_HANDLER_H_

#include "stmlib/stmlib.h"

#include "stmlib/utils/ring_buffer.h"
#include "stmlib/midi/midi.h"

#include "yarns/multi.h"

namespace yarns {

const size_t kSysexMaxChunkSize = 64;
const size_t kSysexRxBufferSize = kSysexMaxChunkSize * 2 + 16;

class MidiHandler {
 public:
  typedef stmlib::RingBuffer<uint8_t, 128> MidiBuffer;
  typedef stmlib::RingBuffer<uint8_t, 32> SmallMidiBuffer;
   
  MidiHandler() { }
  ~MidiHandler() { }
  
  static void Init();
  
  static void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (multi.NoteOn(channel, note, velocity) && !multi.direct_thru()) {
      Send3(0x90 | channel, note, velocity);
    }
  }
  
  static void NoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (multi.NoteOff(channel, note, velocity) && !multi.direct_thru()) {
      Send3(0x80 | channel, note, 0);
    }
  }
  
  static void Aftertouch(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (multi.Aftertouch(channel, note, velocity) && !multi.direct_thru()) {
      Send3(0xa0 | channel, note, velocity);
    }
  }
  
  static void Aftertouch(uint8_t channel, uint8_t velocity) {
    if (multi.Aftertouch(channel, velocity) && !multi.direct_thru()) {
      Send2(0xd0 | channel, velocity);
    }
  }
  
  static void ControlChange(
      uint8_t channel,
      uint8_t controller,
      uint8_t value) {
    if (multi.ControlChange(channel, controller, value) && !multi.direct_thru()) {
      Send3(0xb0 | channel, controller, value);
    }
  }
  
  static void ProgramChange(uint8_t channel, uint8_t program) {
    if (!multi.direct_thru()) {
      Send2(0xc0 | channel, program);
    }
  }
  
  static void PitchBend(uint8_t channel, uint16_t pitch_bend) {
    if (multi.PitchBend(channel, pitch_bend) && !multi.direct_thru()) {
      Send3(0xe0 | channel, pitch_bend & 0x7f, pitch_bend >> 7);
    }
  }

  static void SysExStart() {
    sysex_rx_write_ptr_ = 0;
    ProcessSysExByte(0xf0);
  }

  static void SysExByte(uint8_t sysex_byte) {
    ProcessSysExByte(sysex_byte);
  }
  
  static void SysExEnd() {
    ProcessSysExByte(0xf7);
    DecodeSysExMessage();
  }
  
  static void BozoByte(uint8_t bozo_byte) { }

  static void Clock() {
    if (!multi.internal_clock()) {
      multi.Clock();
    }
  }
  
  static void Start() {
    if (!multi.internal_clock()) {
      multi.Start(false);
    }
  }
  
  static void Continue() {
    if (!multi.internal_clock()) {
      multi.Continue();
    }
  }
  
  static void Stop() {
    if (!multi.internal_clock()) {
      multi.Stop();
    }
  }
  
  static void Reset() {
    multi.Reset();
  }
  
  static bool CheckChannel(uint8_t channel) { return true; }

  static void RawByte(uint8_t byte) {
    if (multi.direct_thru()) {
      if (byte != 0xfa && byte != 0xf8 && byte != 0xfc) {
        output_buffer_.Overwrite(byte);
      }
    }
  }
  
  static void RawMidiData(
      uint8_t status,
      uint8_t* data,
      uint8_t data_size,
      uint8_t accepted_channel) {

  }
  
  static void OnInternalNoteOn(
      uint8_t channel,
      uint8_t note,
      uint8_t velocity) {
    Send3(0x90 | channel, note, velocity);
  }
  
  static void OnInternalNoteOff(uint8_t channel, uint8_t note) {
    Send3(0x80 | channel, note, 0);
  }
  
  static void OnClock() {
    SendNow(0xf8);
  }
  
  static void OnStart() {
    SendNow(0xfa);
  }
  
  static void OnStop() {
    SendNow(0xfc);
  }
  
  static void PushByte(uint8_t byte) {
    input_buffer_.Overwrite(byte);
  }
  
  static void ProcessInput() {
    while (input_buffer_.readable()) {
      parser_.PushByte(input_buffer_.ImmediateRead());
    }
  }
  
  static inline MidiBuffer* mutable_output_buffer() { return &output_buffer_; }
  static inline SmallMidiBuffer* mutable_high_priority_output_buffer() {
    return &high_priority_output_buffer_;
  }

  static inline void Send3(uint8_t byte_1, uint8_t byte_2, uint8_t byte_3) {
    output_buffer_.Overwrite(byte_1);
    output_buffer_.Overwrite(byte_2);
    output_buffer_.Overwrite(byte_3);
  }

  static inline void Send2(uint8_t byte_1, uint8_t byte_2) {
    output_buffer_.Overwrite(byte_1);
    output_buffer_.Overwrite(byte_2);
  }

  static inline void Send1(uint8_t byte) {
    output_buffer_.Overwrite(byte);
  }
  
  static inline void SendBlocking(uint8_t byte) {
    output_buffer_.Write(byte);
  }

  static inline void SendNow(uint8_t byte) {
    high_priority_output_buffer_.Overwrite(byte);
  }
  
  typedef void (*SysExHandlerFn)();
  struct SysExDescription {
    uint8_t prefix[8];
    uint8_t prefix_length;
    uint8_t expected_size;
    SysExHandlerFn handler;
  };

  static void Flush() {
    while (output_buffer_.readable());
  }
  
  static void SysExSendPackets(const uint8_t* data, size_t size);
  
  static inline bool calibrating() {
    return calibration_voice_ < kNumVoices && calibration_note_ < kNumOctaves;
  }
  static inline uint8_t calibration_voice() { return calibration_voice_; }
  static inline uint8_t calibration_note() { return calibration_note_; }
  static inline bool factory_testing_requested() {
    return factory_testing_requested_;
  }
  static void AcknowledgeFactoryTestingRequest() {
    factory_testing_requested_ = false;
  }
  
 private:
  static void SysExSendPacket(
      uint8_t packet_index,
      const uint8_t* data,
      size_t size);
  static void DecodeSysExMessage();
  inline static void ProcessSysExByte(uint8_t sysex_byte) {
    if (!multi.direct_thru()) {
      Send1(sysex_byte);
    }
    if (sysex_rx_write_ptr_ < sizeof(sysex_rx_buffer_)) {
      sysex_rx_buffer_[sysex_rx_write_ptr_++] = sysex_byte;
    }
  }
  
  static void HandleScaleOctaveTuning1ByteForm();
  static void HandleScaleOctaveTuning2ByteForm();
  static void HandleYarnsSpecificMessage();
  
  static MidiBuffer input_buffer_; 
  static MidiBuffer output_buffer_; 
  static SmallMidiBuffer high_priority_output_buffer_;
  static stmlib_midi::MidiStreamParser<MidiHandler> parser_;
  
  static uint8_t sysex_rx_buffer_[kSysexRxBufferSize];
  static uint8_t sysex_rx_write_ptr_;
  
  static uint8_t previous_packet_index_;
  
  static uint8_t calibration_voice_;
  static uint8_t calibration_note_;
  
  static bool factory_testing_requested_;
  
  static const SysExDescription accepted_sysex_[];
   
  DISALLOW_COPY_AND_ASSIGN(MidiHandler);
};

extern MidiHandler midi_handler;

}  // namespace yarns

#endif // YARNS_MIDI_HANDLER_H_
