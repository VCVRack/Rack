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
// MIDI event handler.

#include "yarns/midi_handler.h"

#include <algorithm>

#include "yarns/multi.h"
#ifndef TEST
#include "yarns/storage_manager.h"
#endif // TEST

namespace yarns {

using namespace std;

/* static */
MidiHandler::MidiBuffer MidiHandler::input_buffer_; 

/* static */
MidiHandler::MidiBuffer MidiHandler::output_buffer_;

/* static */
MidiHandler::SmallMidiBuffer MidiHandler::high_priority_output_buffer_;

/* static */
stmlib_midi::MidiStreamParser<MidiHandler> MidiHandler::parser_;

/* static */
const MidiHandler::SysExDescription MidiHandler::accepted_sysex_[] = {
  { { 0xf0, 0x00, 0x21, 0x02, 0x00, 0x0b }, 6, 0xff,
      &MidiHandler::HandleYarnsSpecificMessage },
  { { 0xf0, 0x7f, 0xff, 0x08, 0x08 }, 5, 21,
      &MidiHandler::HandleScaleOctaveTuning1ByteForm },
  { { 0xf0, 0x7e, 0xff, 0x08, 0x08 }, 5, 21,
      &MidiHandler::HandleScaleOctaveTuning1ByteForm },
  { { 0xf0, 0x7f, 0xff, 0x08, 0x09 }, 5, 33,
      &MidiHandler::HandleScaleOctaveTuning2ByteForm },
  { { 0xf0, 0x7e, 0xff, 0x08, 0x09 }, 5, 33,
      &MidiHandler::HandleScaleOctaveTuning2ByteForm }
};

/* static */
uint8_t MidiHandler::sysex_rx_write_ptr_;

/* static */
uint8_t MidiHandler::previous_packet_index_;

/* static */
uint8_t MidiHandler::sysex_rx_buffer_[kSysexRxBufferSize];

/* static */
uint8_t MidiHandler::calibration_voice_;

/* static */
uint8_t MidiHandler::calibration_note_;

/* static */
bool MidiHandler::factory_testing_requested_;

/* static */
void MidiHandler::Init() {
  input_buffer_.Init();
  output_buffer_.Init();
  high_priority_output_buffer_.Init();
  sysex_rx_write_ptr_ = 0;
  previous_packet_index_ = 0;
  calibration_voice_ = 0xff;
  calibration_note_ = 0xff;
  factory_testing_requested_ = false;
}

/* static */
void MidiHandler::DecodeSysExMessage() {
  uint8_t length = sysex_rx_write_ptr_;

  if (sysex_rx_buffer_[length - 1] != 0xf7) {
    // Discard long messages that have been truncated.
    return;
  }
  
  for (uint8_t i = 0;
       i < sizeof(accepted_sysex_) / sizeof(SysExDescription);
       ++i) {
    const SysExDescription& description = accepted_sysex_[i];
    if (description.expected_size == length ||
        description.expected_size == 0xff) {
      bool match = true;
      for (uint8_t j = 0; j < description.prefix_length; ++j) {
        if (description.prefix[j] != sysex_rx_buffer_[j] &&
            description.prefix[j] != 0xff) {
          match = false;
          break;
        }
      }
      if (match) {
        (*description.handler)();
        break;
      }
    }
  }
}

/* static */
void MidiHandler::HandleScaleOctaveTuning1ByteForm() {
  for (uint8_t pitch_class = 0; pitch_class < 12; ++pitch_class) {
    int16_t correction = sysex_rx_buffer_[8 + pitch_class];
    correction -= 64;
    correction = (correction * 128 + (correction > 0 ? 64 : -64)) / 100;
    multi.set_custom_pitch(pitch_class, correction);
  }
}

/* static */
void MidiHandler::HandleScaleOctaveTuning2ByteForm() {
  for (uint8_t pitch_class = 0; pitch_class < 12; ++pitch_class) {
    int16_t correction = sysex_rx_buffer_[8 + pitch_class * 2] << 7;
    correction += sysex_rx_buffer_[9 + pitch_class * 2];
    correction -= 8192;
    correction >>= 6;
    multi.set_custom_pitch(pitch_class, correction);
  }
}


enum SysExCommand {
  SYSEX_COMMAND_DUMP_PACKET = 1,
  SYSEX_COMMAND_REQUEST_PACKETS = 17,
  SYSEX_COMMAND_FACTORY_TESTING_MODE = 32,
  SYSEX_COMMAND_CALIBRATE = 33,
};

/* static */
void MidiHandler::HandleYarnsSpecificMessage() {
#ifndef TEST
  uint8_t command = sysex_rx_buffer_[6];
  if (command == SYSEX_COMMAND_DUMP_PACKET) {
    uint8_t packet_index = sysex_rx_buffer_[7];
    
    // Handle packet reception.
    if (packet_index != 0 && packet_index != previous_packet_index_ + 1) {
      // Packet not in sequence!
      return;
    }
    previous_packet_index_ = packet_index;
    
    // Denibblize.
    uint8_t* data = &sysex_rx_buffer_[8];
    uint8_t* byte_ptr = data;
    uint8_t* nibble_ptr = data;
    uint8_t checksum = 0;
    while (*nibble_ptr != 0xf7 &&
           static_cast<size_t>(byte_ptr - data) <= kSysexMaxChunkSize + 1) {
      *byte_ptr = (*nibble_ptr++) << 4;
      *byte_ptr |= (*nibble_ptr++);
      // Warning! The last byte of the block, which is the checksum
      // is summed here!
      checksum += *byte_ptr++;
    }
    size_t size = byte_ptr - data - 1;
    checksum -= data[size];
    if (checksum != data[size]) {
      previous_packet_index_ = 0xff;
      return;
    }
    if (size != 0) {
      storage_manager.AppendData(data, size, packet_index == 0);
    } else if (packet_index) {
      storage_manager.DeserializeMulti();
    }
  } else if (command == SYSEX_COMMAND_REQUEST_PACKETS) {
    if (sysex_rx_buffer_[7] == 0 &&
        sysex_rx_buffer_[8] == 0 && 
        sysex_rx_buffer_[9] == 0 &&
        sysex_rx_buffer_[10] == 0xf7) {
      storage_manager.SysExSendMulti();
    }
  } else if (command == SYSEX_COMMAND_FACTORY_TESTING_MODE) {
    if (sysex_rx_buffer_[7] == 0 &&
        sysex_rx_buffer_[8] == 0 && 
        sysex_rx_buffer_[9] == 0 &&
        sysex_rx_buffer_[10] == 0xf7) {
      factory_testing_requested_ = true;
    }
  } else if (command == SYSEX_COMMAND_CALIBRATE) {
    calibration_voice_ = sysex_rx_buffer_[7] >> 4;
    calibration_note_ = sysex_rx_buffer_[7] & 0xf;
    if (calibrating()) {
      uint16_t dac_code = (sysex_rx_buffer_[8] << 12) | (sysex_rx_buffer_[9] << 8) |
        (sysex_rx_buffer_[10] << 4) | (sysex_rx_buffer_[11] << 0);
      Voice* voice = multi.mutable_voice(calibration_voice_);
      voice->set_calibration_dac_code(calibration_note_, dac_code);
    } else {
      storage_manager.SaveCalibration();
    }
  }
#endif  // TEST
}

/* static */
void MidiHandler::SysExSendPacket(
    uint8_t packet_index,
    const uint8_t* data,
    size_t size) {
  Flush();
  
  for (uint8_t i = 0; i < 6; ++i) {
    SendBlocking(accepted_sysex_[0].prefix[i]);
  }
  SendBlocking(SYSEX_COMMAND_DUMP_PACKET);
  SendBlocking(packet_index);
  
  // Outputs the data.
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < size; ++i) {
    checksum += data[i];
    SendBlocking(data[i] >> 4);
    SendBlocking(data[i] & 0x0f);
  }
  // Outputs a checksum.
  SendBlocking(checksum >> 4);
  SendBlocking(checksum & 0x0f);

  // End of SysEx block.
  SendBlocking(0xf7);
  Flush();
}

/* static */
void MidiHandler::SysExSendPackets(const uint8_t* data, size_t size) {
  uint8_t block_index = 0;
  while (size) {
    size_t chunk_size = min(size, kSysexMaxChunkSize);
    SysExSendPacket(block_index, data, chunk_size);
    size -= chunk_size;
    data += chunk_size;
    ++block_index;
  }
  // Send a NULL packet to indicate end of transmission.
  SysExSendPacket(block_index, NULL, 0);
}

/* extern */
MidiHandler midi_handler;

}  // namespace yarns