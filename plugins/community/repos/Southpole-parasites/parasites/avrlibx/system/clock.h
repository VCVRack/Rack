// Copyright 2011 Olivier Gillet.
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
// Clock configuration.

#ifndef AVRLIBX_SYSTEM_CLOCK_H_
#define AVRLIBX_SYSTEM_CLOCK_H_

#include <avr/io.h>

#include "avrlibx/avrlibx.h"

#define CCP_WRITE(x, y) CCP = CCP_IOREG_gc; x = y;

namespace avrlibx {
  
enum ClockSource {
  CLOCK_INTERNAL_32M,
  CLOCK_EXTERNAL_8M,
  CLOCK_EXTERNAL_16M
};

template<ClockSource source, uint8_t pll_factor> void SetupClock() {
  if (source == CLOCK_INTERNAL_32M) {
    OSC.CTRL = OSC_RC32MEN_bm;
    while (!(OSC.STATUS & OSC_RC32MRDY_bm));
    
    if (pll_factor == 0) {
      CCP_WRITE(CLK.CTRL, CLK_SCLKSEL_RC32M_gc);
    } else {
      // The 32 Mhz RC oscillator has a base frequency of 8 MHz when fed to the
      // PLL.
      OSC.PLLCTRL = OSC_PLLSRC_RC32M_gc | (pll_factor << 2);
    }
  } else {
    if (source == CLOCK_EXTERNAL_8M) {
      OSC.XOSCCTRL = OSC_FRQRANGE_2TO9_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
    } else if (source == CLOCK_EXTERNAL_16M) {
      OSC.XOSCCTRL = OSC_FRQRANGE_2TO9_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
    }
    OSC.CTRL = OSC_XOSCEN_bm;
    while (!(OSC.STATUS & OSC_XOSCRDY_bm));

    if (pll_factor == 0) {
      CCP_WRITE(CLK.CTRL, CLK_SCLKSEL_XOSC_gc);
    } else {
      OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | pll_factor;
    }
  }
  
  if (pll_factor) {
    OSC.CTRL |= OSC_PLLEN_bm;
    while (!(OSC.STATUS & OSC_PLLRDY_bm));
    CCP_WRITE(CLK.CTRL, CLK_SCLKSEL_PLL_gc);
  }

  CCP_WRITE(CLK.PSCTRL, 0x00);
}

inline void SetupRTC() {
  CCP = CCP_IOREG_gc;
  CLK.RTCCTRL = CLK_RTCSRC_ULP_gc | CLK_RTCEN_bm;
  RTC.CTRL = RTC_PRESCALER_DIV1_gc;
  RTC.PER = 0xffff;
  RTC.CNT = 0;
  while(RTC.STATUS & RTC_SYNCBUSY_bm);
}

inline void SetupRTCMillisecondTick(uint8_t int_level) {
  CCP = CCP_IOREG_gc;
  CLK.RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;
  RTC.CTRL = RTC_PRESCALER_DIV1_gc;
  RTC.INTCTRL = int_level;
  RTC.PER = 100;
  RTC.CNT = 0;
  while(RTC.STATUS & RTC_SYNCBUSY_bm);
}

}  // avrlibx

#endif  // AVRLIBX_SYSTEM_CLOCK_H_
