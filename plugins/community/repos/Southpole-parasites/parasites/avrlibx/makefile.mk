# Copyright 2011 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
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

include avrlibx/config.mk

BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/

MCU            = atxmega$(MCU_NAME)
DMCU           = atxmega$(MCU_NAME)
MCU_DEFINE     = ATXMEGA$(MCU_NAME)

VPATH          = $(PACKAGES)
CC_FILES       = $(notdir $(wildcard $(patsubst %,%/*.cc,$(PACKAGES))))
C_FILES        = $(notdir $(wildcard $(patsubst %,%/*.c,$(PACKAGES))))
AS_FILES       = $(notdir $(wildcard $(patsubst %,%/*.S,$(PACKAGES))))
OBJ_FILES      = $(CC_FILES:.cc=.o) $(C_FILES:.c=.o) $(AS_FILES:.S=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES))
DEPS           = $(OBJS:.o=.d)

TARGET_BIN     = $(BUILD_DIR)$(TARGET).bin
TARGET_ELF     = $(BUILD_DIR)$(TARGET).elf
TARGET_HEX     = $(BUILD_DIR)$(TARGET).hex
TARGET_SIZE    = $(BUILD_DIR)$(TARGET).size
TARGETS        = $(BUILD_DIR)$(TARGET).*
DEP_FILE       = $(BUILD_DIR)depends.mk

CC             = $(AVRLIBX_TOOLS_PATH)avr-gcc
CXX            = $(AVRLIBX_TOOLS_PATH)avr-g++
OBJCOPY        = $(AVRLIBX_TOOLS_PATH)avr-objcopy
OBJDUMP        = $(AVRLIBX_TOOLS_PATH)avr-objdump
AR             = $(AVRLIBX_TOOLS_PATH)avr-ar
SIZE           = $(AVRLIBX_TOOLS_PATH)avr-size
NM             = $(AVRLIBX_TOOLS_PATH)avr-nm
AVRDUDE        = $(AVRDUDE_PATH)avrdude
REMOVE         = rm -f
CAT            = cat

CPPFLAGS      = -mmcu=$(MCU) -I. \
			-g -Os -w -Wall \
			-D__PROG_TYPES_COMPAT__ \
			-DF_CPU=$(F_CPU) \
			-fdata-sections \
			-ffunction-sections \
			-fno-move-loop-invariants \
			$(EXTRA_DEFINES) \
			$(MMC_CONFIG) \
			-D$(MCU_DEFINE) \
			-mcall-prologues
CXXFLAGS      = -fno-exceptions
ASFLAGS       = -mmcu=$(MCU) -I. -x assembler-with-cpp
LDFLAGS       = -mmcu=$(MCU) -lm -Os -Wl,--gc-sections$(EXTRA_LD_FLAGS)

# ------------------------------------------------------------------------------
# Source compiling
# ------------------------------------------------------------------------------

$(BUILD_DIR)%.o: %.cc
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)%.o: %.s
	$(CC) -c $(CPPFLAGS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)%.d: %.cc
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< -MF $@ -MT $(@:.d=.o)

$(BUILD_DIR)%.d: %.c
	$(CC) -MM $(CPPFLAGS) $(CXXFLAGS) $< -MF $@ -MT $(@:.d=.o)

$(BUILD_DIR)%.d: %.S
	$(CC) -MM $(CPPFLAGS) $(ASFLAGS) $< -MF $@ -MT $(@:.d=.o)


# ------------------------------------------------------------------------------
# Object file conversion
# ------------------------------------------------------------------------------

$(BUILD_DIR)%.hex: $(BUILD_DIR)%.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(BUILD_DIR)%.bin: $(BUILD_DIR)%.elf
	$(OBJCOPY) -O binary -R .eeprom $< $@

$(BUILD_DIR)%.eep: $(BUILD_DIR)%.elf
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
		--change-section-lma .eeprom=0 -O ihex $< $@

$(BUILD_DIR)%.lss: $(BUILD_DIR)%.elf
	$(OBJDUMP) -h -S $< > $@

$(BUILD_DIR)%.sym: $(BUILD_DIR)%.elf
	$(NM) -n $< > $@

# ------------------------------------------------------------------------------
# AVRDude
# ------------------------------------------------------------------------------

AVRDUDE_COM_OPTS = -V -p $(DMCU)
AVRDUDE_ISP_OPTS = -c $(PROGRAMMER) -P $(PROGRAMMER_PORT)

# ------------------------------------------------------------------------------
# Main targets
# ------------------------------------------------------------------------------

all:    $(BUILD_DIR) $(TARGET_HEX)

$(BUILD_DIR):
		mkdir -p $(BUILD_DIR)

$(TARGET_ELF):  $(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(SYS_OBJS) -lc

$(DEP_FILE):  $(BUILD_DIR) $(DEPS)
		cat $(DEPS) > $(DEP_FILE)

bin:	$(TARGET_BIN)

upload:    $(TARGET_HEX)
		$(AVRDUDE) $(AVRDUDE_COM_OPTS) $(AVRDUDE_ISP_OPTS) \
			-U flash:w:$(TARGET_HEX):i

upload_boot:    $(TARGET_BIN)
		$(AVRDUDE) $(AVRDUDE_COM_OPTS) $(AVRDUDE_ISP_OPTS) \
			-U boot:w:$(TARGET_BIN)

clean:
		$(REMOVE) $(OBJS) $(TARGETS) $(DEP_FILE) $(DEPS)

depends:  $(DEPS)
		cat $(DEPS) > $(DEP_FILE)

$(TARGET_SIZE):  $(TARGET_ELF)
		$(SIZE) $(TARGET_ELF) > $(TARGET_SIZE)

$(BUILD_DIR)$(TARGET).top_symbols: $(TARGET_ELF)
		$(NM) $(TARGET_ELF) --size-sort -C -f bsd -r > $@

size: $(TARGET_SIZE)
		cat $(TARGET_SIZE) | awk '{ print $$1+$$2 }' | tail -n1 | figlet | cowsay -n -f moose

ramsize: $(TARGET_SIZE)
		cat $(TARGET_SIZE) | awk '{ print $$2+$$3 }' | tail -n1 | figlet | cowsay -n -f small

size_report:  build/$(TARGET)/$(TARGET).lss build/$(TARGET)/$(TARGET).top_symbols

.PHONY: all clean depends upload

include $(DEP_FILE)

# ------------------------------------------------------------------------------
# Midi files for firmware update
# ------------------------------------------------------------------------------

HEX2SYSEX = python avrlibx/tools/hex2sysex/hex2sysex.py

$(BUILD_DIR)%.syx: $(BUILD_DIR)%.hex
	$(HEX2SYSEX) $(SYSEX_FLAGS) --syx -o $@ $<

syx: $(BUILD_DIR)$(TARGET).syx

# ------------------------------------------------------------------------------
# Resources
# ------------------------------------------------------------------------------

RESOURCE_COMPILER = avrlibx/tools/resources_compiler.py

resources:	$(wildcard $(RESOURCES)/*.py) 
		python $(RESOURCE_COMPILER) $(RESOURCES)/resources.py

# ------------------------------------------------------------------------------
# Set fuses
# ------------------------------------------------------------------------------

terminal:
		$(AVRDUDE) $(AVRDUDE_COM_OPTS) $(AVRDUDE_ISP_OPTS) -e -tuF

fuses:
		$(AVRDUDE) $(AVRDUDE_COM_OPTS) $(AVRDUDE_ISP_OPTS) -e -u \
			-U fuse0:w:0x$(FUSE0):m \
			-U fuse1:w:0x$(FUSE1):m \
			-U fuse2:w:0x$(FUSE2):m \
			-U fuse4:w:0x$(FUSE4):m \
			-U fuse5:w:0x$(FUSE5):m \
			-U lock:w:0x$(LOCK):m
