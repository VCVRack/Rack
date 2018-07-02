# Copyright 2012 Olivier Gillet.
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
MCU_NAME       = 88
TARGET         = branches
PACKAGES       = avrlib avrlib/devices branches
RESOURCES      = branches/resources
EXTRA_DEFINES  = -DDISABLE_DEFAULT_UART_RX_ISR

LFUSE          = e2
HFUSE          = dd
EFUSE          = f8
LOCK           = 2f

include avrlib/makefile.mk

include $(DEP_FILE)

# Rule for building the firmware update file
wav:  $(TARGET_BIN)
	python avr_audio_bootloader/fsk/encoder.py \
		-s 15625 -b 16 -n 8 -z 4 -p 64 -g 64 -k 30 \
		$(TARGET_BIN)

bootstrap_all:
		make -f branches/makefile
		make -f branches/bootloader/makefile
		make -f branches/bootloader/makefile fuses
		$(AVRDUDE) -B 1 $(AVRDUDE_COM_OPTS) $(AVRDUDE_ISP_OPTS) \
			-U flash:w:build/branches/branches.hex:i \
			-U flash:w:build/branches_bootloader/branches_bootloader.hex:i \
			-U lock:w:0x2f:m

