// Copyright 2009 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Decoding of MIDI messages.

#ifndef EDGES_MIDI_H_
#define EDGES_MIDI_H_

namespace midi {

const uint8_t kBankMsb = 0x00;
const uint8_t kBankLsb = 0x20;
const uint8_t kModulationWheelMsb = 0x01;
const uint8_t kBreathController = 0x02;
const uint8_t kFootPedalMsb = 0x04;
const uint8_t kDataEntryMsb = 0x06;
const uint8_t kVolume = 0x07;
const uint8_t kFootPedalLsb = 0x24;
const uint8_t kDataEntryLsb = 0x26;
const uint8_t kPortamentoTimeMsb = 0x05;
const uint8_t kHoldPedal = 0x40;
const uint8_t kHarmonicIntensity = 0x47;
const uint8_t kRelease = 0x48;
const uint8_t kAttack = 0x49;
const uint8_t kBrightness = 0x4a;
const uint8_t kDataIncrement = 0x60;
const uint8_t kDataDecrement = 0x61;
const uint8_t kNrpnMsb = 0x63;
const uint8_t kNrpnLsb = 0x62;
const uint8_t kAssignableCcA = 0x10;
const uint8_t kAssignableCcB = 0x11;
const uint8_t kAssignableCcC = 0x12;
const uint8_t kAssignableCcD = 0x13;

// A device that responds to MIDI messages should implement this interface.
// Everything is static - this is because the main synth class is a "static
// singleton". Note that this allows all the MIDI processing code to be inlined!
struct MidiDevice {
  static void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) { }
  static void NoteOff(uint8_t channel, uint8_t note, uint8_t velocity) { }
  static void Aftertouch(uint8_t channel, uint8_t note, uint8_t velocity) { }
  static void Aftertouch(uint8_t channel, uint8_t velocity) { }
  static void ControlChange(uint8_t channel, uint8_t controller,
                             uint8_t value) { }
  static void ProgramChange(uint8_t channel, uint8_t program) { }
  static void PitchBend(uint8_t channel, uint16_t pitch_bend) { }

  static void AllSoundOff(uint8_t channel) { }
  static void ResetAllControllers(uint8_t channel) { }
  static void LocalControl(uint8_t channel, uint8_t state) { }
  static void AllNotesOff(uint8_t channel) { }
  static void OmniModeOff(uint8_t channel) { }
  static void OmniModeOn(uint8_t channel) { }
  static void MonoModeOn(uint8_t channel, uint8_t num_channels) { }
  static void PolyModeOn(uint8_t channel) { }
  static void SysExStart() { }
  static void SysExByte(uint8_t sysex_byte) { }
  static void SysExEnd() { }
  static void BozoByte(uint8_t bozo_byte) { }

  static void Clock() { }
  static void Start() { }
  static void Continue() { }
  static void Stop() { }
  static void ActiveSensing() { }
  static void Reset() { }

  static uint8_t CheckChannel(uint8_t channel) { return 1; }
  static void RawByte(uint8_t byte) { }
  static void RawMidiData(
      uint8_t status,
      uint8_t* data,
      uint8_t data_size,
      uint8_t accepted_channel) { }
};

template<typename Device>
class MidiStreamParser {
 public:
  MidiStreamParser();
  void PushByte(uint8_t byte);

 private:
  void MessageReceived(uint8_t status);
 
  uint8_t running_status_;
  uint8_t data_[3];
  uint8_t data_size_;  // Number of non-status byte received.
  uint8_t expected_data_size_;  // Expected number of non-status bytes.

  DISALLOW_COPY_AND_ASSIGN(MidiStreamParser);
};

template<typename Device>
MidiStreamParser<Device>::MidiStreamParser() {
  running_status_ = 0;
  data_size_ = 0;
  expected_data_size_ = 0;
}

template<typename Device>
void MidiStreamParser<Device>::PushByte(uint8_t byte) {
  // Realtime messages are immediately passed-through, and do not modify the
  // state of the parser.
  Device::RawByte(byte);
  if (byte >= 0xf8) {
    MessageReceived(byte);
  } else {
    if (byte >= 0x80) {
      uint8_t hi = byte & 0xf0;
      uint8_t lo = byte & 0x0f;
      data_size_ = 0;
      expected_data_size_ = 1;
      switch (hi) {
        case 0x80:
        case 0x90:
        case 0xa0:
        case 0xb0:
          expected_data_size_ = 2;
          break;
        case 0xc0:
        case 0xd0:
          break;  // default data size of 1.
        case 0xe0:
          expected_data_size_ = 2;
          break;
        case 0xf0:
          if (lo > 0 && lo < 3) {
            expected_data_size_ = 2;
          } else if (lo >= 4) {
            expected_data_size_ = 0;
          }
          break;
      }
      if (byte == 0xf7) {
        if (running_status_ == 0xf0) {
          Device::SysExEnd();
        }
        running_status_ = 0;
      } else if (byte == 0xf0) {
        running_status_ = 0xf0;
        Device::SysExStart();
      } else {
        running_status_ = byte;
      }
    } else {
      data_[data_size_++] = byte;
    }
    if (data_size_ >= expected_data_size_) {
      MessageReceived(running_status_);
      data_size_ = 0;
      if (running_status_ > 0xf0) {
        expected_data_size_ = 0;
        running_status_ = 0;
      }
    }
  }
}

template<typename Device>
void MidiStreamParser<Device>::MessageReceived(uint8_t status) {
  if (!status) {
    Device::BozoByte(data_[0]);
  }

  uint8_t hi = status & 0xf0;
  uint8_t lo = status & 0x0f;

  // If this is a channel-specific message, check first that the receiver is
  // tune to this channel.
  if (hi != 0xf0 && !Device::CheckChannel(lo)) {
    Device::RawMidiData(status, data_, data_size_, 0);
    return;
  }
  Device::RawMidiData(status, data_, data_size_, 1);
  switch (hi) {
    case 0x80:
      Device::NoteOff(lo, data_[0], data_[1]);
      break;

    case 0x90:
      if (data_[1]) {
        Device::NoteOn(lo, data_[0], data_[1]);
      } else {
        Device::NoteOff(lo, data_[0], 0);
      }
      break;

    case 0xa0:
      Device::Aftertouch(lo, data_[0], data_[1]);
      break;

    case 0xb0:
      switch (data_[0]) {
        case 0x78:
          Device::AllSoundOff(lo);
          break;
        case 0x79:
          Device::ResetAllControllers(lo);
          break;
        case 0x7a:
          Device::LocalControl(lo, data_[1]);
          break;
        case 0x7b:
          Device::AllNotesOff(lo);
          break;
        case 0x7c:
          Device::OmniModeOff(lo);
          break;
        case 0x7d:
          Device::OmniModeOn(lo);
          break;
        case 0x7e:
          Device::MonoModeOn(lo, data_[1]);
          break;
        case 0x7f:
          Device::PolyModeOn(lo);
          break;
        default:
          Device::ControlChange(lo, data_[0], data_[1]);
          break;
      }
      break;

    case 0xc0:
      Device::ProgramChange(lo, data_[0]);
      break;

    case 0xd0:
      Device::Aftertouch(lo, data_[0]);
      break;

    case 0xe0:
      Device::PitchBend(lo, (static_cast<uint16_t>(data_[1]) << 7) + data_[0]);
      break;

    case 0xf0:
      switch(lo) {
        case 0x0:
          Device::SysExByte(data_[0]);
          break;
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
          // TODO(pichenettes): implement this if it makes sense.
          break;
        case 0x8:
          Device::Clock();
          break;
        case 0x9:
          break;
        case 0xa:
          Device::Start();
          break;
        case 0xb:
          Device::Continue();
          break;
        case 0xc:
          Device::Stop();
          break;
        case 0xe:
          Device::ActiveSensing();
          break;
        case 0xf:
          Device::Reset();
          break;
      }
      break;
  }
}

}  // namespace midi

#endif  // EDGES_MIDI_H_
