# Copyright 2011 Olivier Gillet.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

VERSION        = 0.1
MCU_NAME       = 328
TARGET         = grids_bootloader
PACKAGES       = avrlib grids/bootloader avr_audio_bootloader/fsk
EXTRA_DEFINES  = -funsigned-char -fno-inline-small-functions -DDISABLE_DEFAULT_UART_RX_ISR
EXTRA_LD_FLAGS = ,--section-start=.text=0x7000,--relax

LFUSE          = ff
HFUSE          = d8
EFUSE          = 05
LOCK           = 2f

include avrlib/makefile.mk

include $(DEP_FILE)
