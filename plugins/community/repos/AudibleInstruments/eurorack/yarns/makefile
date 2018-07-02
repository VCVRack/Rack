# Copyright 2013 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.

# System specifications
F_CRYSTAL      = 8000000L
F_CPU          = 72000000L
SYSCLOCK       = SYSCLK_FREQ_72MHz
FAMILY         = f10x
DENSITY        = md
MEMORY_MODE    = flash
# USB            = enabled

BOOTLOADER     = yarns_bootloader

APPLICATION_SMALL = TRUE

# Preferred upload command
UPLOAD_COMMAND  = upload_combo_jtag

# Packages to build
TARGET         = yarns
PACKAGES       = yarns yarns/drivers stmlib/utils stmlib/system
RESOURCES      = yarns/resources

TOOLCHAIN_PATH ?= /usr/local/arm-4.8.3/

include stmlib/makefile.inc


# Rules for building the SysEx update file.
SYSEX_FLAGS    = --page_size=512 --device_id=11

HEX2SYSEX = python tools/hex2sysex/hex2sysex.py

$(BUILD_DIR)%.syx: $(BUILD_DIR)%.bin
	$(HEX2SYSEX) $(SYSEX_FLAGS) --syx -o $@ $<

syx: $(BUILD_DIR)$(TARGET).syx