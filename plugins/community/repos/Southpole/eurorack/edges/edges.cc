// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (olivier@mutable-instruments.net)
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

#include "avrlibx/system/init.h"
#include "avrlibx/system/time.h"
#include "avrlibx/system/timer.h"
#include "avrlibx/utils/op.h"

#include "edges/adc_acquisition.h"
#include "edges/audio_buffer.h"
#include "edges/digital_oscillator.h"
#include "edges/hardware_config.h"
#include "edges/midi.h"
#include "edges/midi_handler.h"
#include "edges/settings.h"
#include "edges/timer_oscillator.h"
#include "edges/ui.h"

using namespace avrlibx;
using namespace edges;

MidiIO midi_io;
MidiBuffer midi_in_buffer;
midi::MidiStreamParser<MidiHandler> midi_parser;

GateInputs gate_inputs;

DACSS dac_ss;
AudioDACInterface audio_dac;
ADCSS adc_ss;
ADCInterface adc;

AudioRateTimer audio_rate_timer;

SyncSwitch sync_switch;

TimerOscillator channel_0;
TimerOscillator channel_1;
TimerOscillator channel_2;
TimerOscillator channel_3;

DigitalOscillator channel_4;

// Channel 1 overflow. If sync is enabled, reset the phase of the channel 2.
ISR(TCD0_OVF_vect) {
  Channel2Timer::Restart();
}

volatile uint16_t audio_interrupt_count = 0;

uint8_t channel_remapping[] = { 0, 1, 4, 5, 2, 3, 6, 7};

// Audio rate interrupt (48.048kHz).
ISR(TCE0_OVF_vect) {
  // Shove a sample to the DAC.
  uint16_t sample = audio_buffer.ImmediateRead();
  audio_dac.Strobe();
  Word sample_12bits;
  sample_12bits.value = sample | 0x3000;
  audio_dac.Overwrite(sample_12bits.bytes[1]);
  audio_dac.Overwrite(sample_12bits.bytes[0]);
  
  // Step through the ADC pipeline.
  int8_t result = adc_acquisition.Cycle();
  
  if (result != -1) {
    uint16_t value = adc_acquisition.channel(result);
#ifdef OCTAL_ADC
    value = 4095 - value;
    result = channel_remapping[result];
#endif  // OCTAL_ADC
    ui.set_cv(result, value);
  }
  
  // When a conversion is finished, update the corresponding channel
  // Note that the UI can take over the value read from the DAC, for example
  // When programming an arpeggio
  switch (result) {
    case 0:
      channel_1.UpdatePitch<Channel1Timer>(
          midi_handler.shift_pitch(0,
              settings.dac_to_pitch(0, ui.cv(0))) + 
          settings.dac_to_fm(0, ui.cv(4)),
          settings.pw(0));
      channel_0.SubFollow<Channel0Timer>(channel_1);
      break;
      
    case 1:
      channel_2.UpdatePitch<Channel2Timer>(
          midi_handler.shift_pitch(1,
              settings.dac_to_pitch(1, ui.cv(1))) +
          settings.dac_to_fm(1, ui.cv(5)),
          settings.pw(1));
      break;
    
    case 2:
      channel_3.UpdatePitch<Channel3Timer>(
          midi_handler.shift_pitch(2,
              settings.dac_to_pitch(2, ui.cv(2))) +
          settings.dac_to_fm(2, ui.cv(6)),
          settings.pw(2));
      break;
      
    case 3:
      {
        channel_4.UpdatePitch(
            midi_handler.shift_pitch(3,
                settings.dac_to_pitch(3, ui.cv(3))) +
            settings.dac_to_fm(3, ui.cv(7)),
            settings.shape(3));
#ifndef OCTAL_ADC
        uint8_t cv_controlled_pw_8bit = U16ShiftRight4(ui.cv(3));
        channel_1.set_cv_pw(cv_controlled_pw_8bit);
        channel_2.set_cv_pw(cv_controlled_pw_8bit);
        channel_3.set_cv_pw(cv_controlled_pw_8bit);
#endif  // #ifndef OCTAL_ADC
      }
      break;

#ifdef OCTAL_ADC      
    case 4:
      channel_1.set_cv_pw(U16ShiftRight4(ui.cv(4)));
      break;
    case 5:
      channel_2.set_cv_pw(U16ShiftRight4(ui.cv(5)));
      break;
    case 6:
      channel_3.set_cv_pw(U16ShiftRight4(ui.cv(6)));
      break;
    case 7:
      channel_4.set_cv_pw(U16ShiftRight4(ui.cv(7)));
      break;
#endif  // #ifdef OCTAL_ADC

    default:
      {
        // Otherwise, scan the gate.
        uint8_t gate = ~gate_inputs.value();
        bool g_1 = (gate & 1) || midi_handler.gate(0) || settings.arpeggio(0);
        channel_0.Gate<Channel0>(g_1);
        channel_1.Gate<Channel1>(g_1);
        
        bool g_2 = (gate & 2) || midi_handler.gate(1) || settings.arpeggio(1);
        channel_2.Gate<Channel2>(g_2);
        
        bool g_3 = (gate & 4) || midi_handler.gate(2) || settings.arpeggio(2);
        channel_3.Gate<Channel3>(g_3);
        
        bool g_4 = (gate & 8) || midi_handler.gate(3) || settings.arpeggio(3);
        channel_4.Gate(g_4);
    
        // Read some MIDI bytes, but process them later.
        if (midi_io.readable()) {
          midi_in_buffer.NonBlockingWrite(midi_io.ImmediateRead());
        }
      }
      break;
  }
  ++audio_interrupt_count;
}

inline void Init() {
  SysInit();
  
  sync_switch.set_direction(INPUT);
  sync_switch.set_mode(PORT_MODE_PULL_UP);

  gate_inputs.set_direction(INPUT);
  gate_inputs.set_mode(PORT_MODE_PULL_UP);
  
  ui.Init();
  
  // Reset to factory default if any switch is pressed during boot.
  settings.Init(!(Switches::Read() & 8));
  midi_io.Init();
  
  midi_handler.Init();
  
  audio_dac.Init();
  audio_dac.Strobe();
  audio_dac.Overwrite(0x1f);
  audio_dac.Overwrite(0xff);
  
  channel_0.Init<Channel0, Channel0Timer>();
  channel_1.Init<Channel1, Channel1Timer>();
  channel_2.Init<Channel2, Channel2Timer>();
  channel_3.Init<Channel3, Channel3Timer>();
  channel_4.Init();
  
  adc_acquisition.Init();
  
  audio_rate_timer.set_prescaler(TIMER_PRESCALER_CLK);
  audio_rate_timer.set_period(665); // 48kHz
  audio_rate_timer.set_mode(TIMER_MODE_NORMAL);
  audio_rate_timer.EnableOverflowInterrupt(2);
}

int main(void) {
  Init();
  while (1) {
    // Render audio.
    channel_4.Render();
    
    // Scan MIDI in.
    while (midi_in_buffer.readable()) {
      midi_parser.PushByte(midi_in_buffer.ImmediateRead());
    }
    
    // Do some UI stuff.
    if (audio_interrupt_count >= 48) {
      audio_interrupt_count -= 48;
      uint8_t gate = ~gate_inputs.value();
      // Detect new notes and forward to UI subsystem for calibration functions.
      uint8_t mask = 1;
      for (uint8_t channel = 0; channel < kNumChannels; ++channel) {
        if ((gate & mask) && !(ui.gate() & mask)) {
          settings.StepArpeggio(channel);
        }
        mask <<= 1;
      }
      ui.set_gate(gate);
      ui.Poll();      
    }
    
    // Check the status of the sync switch
    if (sync_switch.value()) {
      Channel1Timer::EnableOverflowInterrupt(3);
    } else {
      Channel1Timer::DisableOverflowInterrupt();
    }
  }
}
