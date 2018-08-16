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
// Decoding of MIDI messages.

#ifndef STMLIB_MIDI_H_
#define STMLIB_MIDI_H_

namespace stmlib_midi {

const uint8_t kCCModulationWheelMsb = 0x01;
const uint8_t kCCBreathController = 0x02;
const uint8_t kCCFootPedalMsb = 0x04;
const uint8_t kCCPortamentoTimeMsb = 0x05;
const uint8_t kCCDataEntryMsb = 0x06;
const uint8_t kCCVolume = 0x07;
const uint8_t kCCBankLsb = 0x20;
const uint8_t kCCDataEntryLsb = 0x26;
const uint8_t kCCHoldPedal = 0x40;
const uint8_t kCCHarmonicIntensity = 0x47;
const uint8_t kCCRelease = 0x48;
const uint8_t kCCAttack = 0x49;
const uint8_t kCCBrightness = 0x4a;
const uint8_t kCCDataIncrement = 0x60;
const uint8_t kCCDataDecrement = 0x61;
const uint8_t kCCNrpnLsb = 0x62;
const uint8_t kCCNrpnMsb = 0x63;
const uint8_t kCCOmniModeOff = 0x7c;
const uint8_t kCCOmniModeOn = 0x7d;
const uint8_t kCCMonoModeOn = 0x7e;
const uint8_t kCCPolyModeOn = 0x7f;

template<typename Handler>
class MidiStreamParser {
 public:
  MidiStreamParser() {
    running_status_ = 0;
    data_size_ = 0;
    expected_data_size_ = 0;
  }
  void PushByte(uint8_t byte) {
    // Active sensing messages are filtered at the source, the hard way...
    if (byte == 0xfe) {
      return;
    }
    Handler::RawByte(byte);
    // Realtime messages are immediately passed-through, and do not modify the
    // state of the parser.
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
            Handler::SysExEnd();
          }
          running_status_ = 0;
        } else if (byte == 0xf0) {
          running_status_ = 0xf0;
          Handler::SysExStart();
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

 private:
  void MessageReceived(uint8_t status) {
    if (!status) {
      Handler::BozoByte(data_[0]);
    }

    uint8_t hi = status & 0xf0;
    uint8_t lo = status & 0x0f;

    // If this is a channel-specific message, check first that the receiver is
    // tuned to this channel.
    if (hi != 0xf0 && !Handler::CheckChannel(lo)) {
      Handler::RawMidiData(status, data_, data_size_, 0);
      return;
    }
    if (status != 0xf0 && status != 0xf7) {
      Handler::RawMidiData(status, data_, data_size_, 1);
    }
    switch (hi) {
      case 0x80:
        Handler::NoteOff(lo, data_[0], data_[1]);
        break;

      case 0x90:
        if (data_[1]) {
          Handler::NoteOn(lo, data_[0], data_[1]);
        } else {
          Handler::NoteOff(lo, data_[0], 0);
        }
        break;

      case 0xa0:
        Handler::Aftertouch(lo, data_[0], data_[1]);
        break;

      case 0xb0:
        Handler::ControlChange(lo, data_[0], data_[1]);
        break;

      case 0xc0:
        Handler::ProgramChange(lo, data_[0]);
        break;

      case 0xd0:
        Handler::Aftertouch(lo, data_[0]);
        break;

      case 0xe0:
        Handler::PitchBend(lo, (static_cast<uint16_t>(data_[1]) << 7) + data_[0]);
        break;

      case 0xf0:
        switch(lo) {
          case 0x0:
            Handler::SysExByte(data_[0]);
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
            Handler::Clock();
            break;
          case 0x9:
            break;
          case 0xa:
            Handler::Start();
            break;
          case 0xb:
            Handler::Continue();
            break;
          case 0xc:
            Handler::Stop();
            break;
          case 0xf:
            Handler::Reset();
            break;
        }
        break;
    }
  }
 
  uint8_t running_status_;
  uint8_t data_[3];
  uint8_t data_size_;  // Number of non-status byte received.
  uint8_t expected_data_size_;  // Expected number of non-status bytes.

  DISALLOW_COPY_AND_ASSIGN(MidiStreamParser);
};

}  // namespace stmlib_midi

#endif // STMLIB_MIDI_H_
