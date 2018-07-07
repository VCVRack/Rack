# Copyright 2012 Olivier Gillet.
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

# ------------------------------------------------------------------------------
# Files and directories
# ------------------------------------------------------------------------------

TOOLCHAIN_PATH ?= /usr/local/gcc-arm-none-eabi-5_4-2016q3/

# Supported: arm-usb-ocd, arm-usb-ocd-h, arm-usb-tiny-h, stlink-v2, stlink-v2-1
PGM_INTERFACE ?= stlink-v2-1
# hla for stlink-v2+jtag; hla-swd for stlink-v2+swd; jtag otherwise
PGM_INTERFACE_TYPE ?= hla

PGM_SERIAL_PORT ?= /dev/tty.usbserial-AH02F1B0
PGM_SERIAL_BAUD_RATE ?= 115200
PGM_SERIAL_VERIFY ?= -v

# --- You should not have to modify these --------------------------------------

TOOLCHAIN_BIN  = $(TOOLCHAIN_PATH)bin/
STM32_PATH     = stmlib/third_party/STM

# --- Do not modify anything below this point ----------------------------------

BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/

CC             = $(TOOLCHAIN_BIN)arm-none-eabi-gcc
CXX            = $(TOOLCHAIN_BIN)arm-none-eabi-g++
AS             = $(TOOLCHAIN_BIN)arm-none-eabi-as
OBJCOPY        = $(TOOLCHAIN_BIN)arm-none-eabi-objcopy
DB             = $(TOOLCHAIN_BIN)arm-none-eabi-gdb
OBJDUMP        = $(TOOLCHAIN_BIN)arm-none-eabi-objdump
AR             = $(TOOLCHAIN_BIN)arm-none-eabi-ar
SIZE           = $(TOOLCHAIN_BIN)arm-none-eabi-size
NM             = $(TOOLCHAIN_BIN)arm-none-eabi-nm
REMOVE         = rm -f
CAT            = cat
FORMAT_SIZE    = | figlet | cowsay -n -f moose
FORMAT_RAMSIZE = | figlet | cowsay -n -f small

STM32LOADER_PATH = stmlib/programming/serial/
OPENOCD_SCRIPTS_PATH = stmlib/programming/jtag/


# ------------------------------------------------------------------------------
# Files and directories for the user code
# ------------------------------------------------------------------------------

VPATH          = $(PACKAGES)

CC_FILES       = $(notdir $(wildcard $(patsubst %,%/*.cc,$(PACKAGES))))
C_FILES        = $(notdir $(wildcard $(patsubst %,%/*.c,$(PACKAGES))))
AS_FILES       = $(notdir $(wildcard $(patsubst %,%/*.s,$(PACKAGES))))

TARGET_BIN     = $(BUILD_DIR)$(TARGET).bin
TARGET_ELF     = $(BUILD_DIR)$(TARGET).elf
TARGET_HEX     = $(BUILD_DIR)$(TARGET).hex
COMBO_HEX      = $(BUILD_DIR)$(TARGET)_bootloader_combo.hex
COMBO_BIN      = $(BUILD_DIR)$(TARGET)_bootloader_combo.bin
BOOTLOADER_HEX = $(BUILD_ROOT)$(BOOTLOADER)/$(BOOTLOADER).hex
TARGET_SIZE    = $(BUILD_DIR)$(TARGET).size
TARGETS        = $(BUILD_DIR)$(TARGET).*

# ------------------------------------------------------------------------------
# Platform specific flags
# ------------------------------------------------------------------------------

MCU ?= STM32F40_41xxx

ifeq ($(FAMILY),f4xx)

MODEL_DEFINE = $(MCU) -DSTM32F4XX -DARM_MATH_CM4 -D__FPU_PRESENT
STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f4xx.o
LINKER_SCRIPT ?= stmlib/linker_scripts/stm32f4xx_flash$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F4xx_StdPeriph_Driver
ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
            -mthumb-interwork -funroll-loops

ARCH_DEFINE = GCC_ARMCM4

else ifeq ($(FAMILY),f37x)

MODEL_DEFINE = STM32F37X -DARM_MATH_CM4 -D__FPU_PRESENT
STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f37x.o
LINKER_SCRIPT = stmlib/linker_scripts/stm32f373x_flash$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F37x_StdPeriph_Driver
ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb-interwork -funroll-loops
ARCH_DEFINE = GCC_ARMCM4

else ifeq ($(FAMILY),f30x)

MODEL_DEFINE = STM32F30X -DARM_MATH_CM4 -D__FPU_PRESENT
STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f30x.o
LINKER_SCRIPT = stmlib/linker_scripts/stm32f373x_flash$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F30x_StdPeriph_Driver
ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb-interwork -funroll-loops
ARCH_DEFINE = GCC_ARMCM4

else

ifeq ($(DENSITY),hd)
MODEL_DEFINE = STM32F10X_HD -D STM32F10X
else ifeq ($(DENSITY),md)
MODEL_DEFINE = STM32F10X_MD -D STM32F10X
else
MODEL_DEFINE = STM32F10X_CL -D STM32F10X
endif

STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f10x_$(DENSITY).o
LINKER_SCRIPT ?= stmlib/linker_scripts/stm32f10x_flash_$(DENSITY)$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F10x_StdPeriph_Driver
ARCHFLAGS = -mcpu=cortex-m3 -mthumb -fno-unroll-loops
ARCH_DEFINE = GCC_ARMCM3

endif

ifeq ($(APPLICATION),TRUE)
APPLICATION_DEFINE = -DAPPLICATION
LD_SUFFIX = _application
BASE_ADDRESS = 0x08004000
else ifeq ($(APPLICATION_SMALL),TRUE)
APPLICATION_DEFINE = -DAPPLICATION
LD_SUFFIX = _application_small
BASE_ADDRESS = 0x08001000
else ifeq ($(APPLICATION_LARGE),TRUE)
APPLICATION_DEFINE = -DAPPLICATION
LD_SUFFIX = _application_large
BASE_ADDRESS = 0x08008000
else
APPLICATION_DEFINE = -DNO_APPLICATION
LD_SUFFIX =
BASE_ADDRESS = 0x08000000
endif

# ------------------------------------------------------------------------------
# Flags for gcc/binutils
# ------------------------------------------------------------------------------

INCFLAGS       = \
			-I. \
			-D$(ARCH_DEFINE) \
			-D$(MODEL_DEFINE) \
			$(APPLICATION_DEFINE) \
			$(PROJECT_CONFIGURATION) \
			-DF_CPU=$(F_CPU) \
			-DF_CRYSTAL=$(F_CRYSTAL) \
			-DSAMPLE_RATE=$(SAMPLE_RATE) \
			-DUSE_STDPERIPH_DRIVER \
			-D$(SYSCLOCK)=$(F_CPU)

CCFLAGS         = \
			-g -O2 -Wall -Werror \
                        -Wno-unused-local-typedefs \
                        -fasm \
                        -finline \
                        -finline-functions \
                        -finline-functions-called-once \
                        -ffast-math \
                        -fdata-sections \
                        -ffunction-sections \
                        $(INCFLAGS) \
			$(ARCHFLAGS)

CPPFLAGS      = -fno-exceptions -fno-rtti

ASFLAGS       = $(ARCHFLAGS)

LDFLAGS       = \
			-Wl,-Map=$(BUILD_DIR)$(TARGET).map \
			-Wl,--gc-sections \
			-T $(LINKER_SCRIPT) \
			$(ARCHFLAGS) \
			$(INCFLAGS) \
			-L$(STM32_PATH)


# ------------------------------------------------------------------------------
# Files and directories for the system firmware
# ------------------------------------------------------------------------------

FW_BUILD_DIR = $(BUILD_ROOT)stm_firmware/
FW_CORE_DIR = $(STM32_PATH)/CMSIS/CM3_$(FAMILY)
FW_STARTUP_DIR = $(FW_CORE_DIR)/startup/gcc

FW_STDDRIVER_SRCDIR = $(FW_STDDRIVER_DIR)/src
FW_STDDRIVER_INCDIR = $(FW_STDDRIVER_DIR)/inc

FW_USB_DIR = $(STM32_PATH)/STM32_USB-FS-Device_Driver
FW_USB_SRCDIR = $(FW_USB_DIR)/src
FW_USB_INCDIR = $(FW_USB_DIR)/inc

INCFLAGS += -I$(STM32_PATH) -I$(FW_CORE_DIR) -I$(FW_STARTUP_DIR) -I$(FW_STDDRIVER_INCDIR)
VPATH += $(FW_CORE_DIR) $(FW_STARTUP_DIR) $(FW_STDDRIVER_SRCDIR) $(STM32_PATH) $(FW_BUILD_DIR)

CORE_C_FILES = $(notdir $(wildcard $(patsubst %,%/*.c,$(FW_CORE_DIR))))
DRIVERS_C_FILES = $(notdir $(wildcard $(patsubst %,%/*.c,$(FW_STDDRIVER_SRCDIR))))
USB_C_FILES = $(notdir $(wildcard $(patsubst %,%/*.c,$(FW_USB_SRCDIR))))

C_FILES += $(CORE_C_FILES)
C_FILES += $(DRIVERS_C_FILES)

ifeq ($(USB),enabled)
INCFLAGS += -I$(FW_USB_INCDIR)
VPATH += $(FW_USB_SRCDIR)
C_FILES += $(USB_C_FILES)
endif


# ------------------------------------------------------------------------------
# Source compiling and dependency analysis
# ------------------------------------------------------------------------------

$(BUILD_DIR)%.o: %.s
	$(CC) -c -x assembler-with-cpp $(ASFLAGS) $< -o $@

$(BUILD_DIR)%.o: %.c
	$(CC) -c $(CCFLAGS) -std=c99 $< -o $@

$(BUILD_DIR)%.o: %.cc
	$(CXX) -c $(CCFLAGS) $(CPPFLAGS) $< -o $@

$(BUILD_DIR)%.d: %.s
	$(CC) -c -x assembler-with-cpp -MM $(ASFLAGS) $< -MF $@ -MT $(@:.d=.o)

$(BUILD_DIR)%.d: %.c
	$(CC) -MM $(CCFLAGS) $< -MF $@ -MT $(@:.d=.o)

$(BUILD_DIR)%.d: %.cc
	$(CXX) -MM $(CCFLAGS) $(CPPFLAGS) $< -MF $@ -MT $(@:.d=.o)


# ------------------------------------------------------------------------------
# Object file conversion
# ------------------------------------------------------------------------------

$(BUILD_DIR)%.hex: $(BUILD_DIR)%.elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)%.bin: $(BUILD_DIR)%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)%.lss: $(BUILD_DIR)%.elf
	$(OBJDUMP) -d -h -S $< > $@

$(BUILD_DIR)%.sym: $(BUILD_DIR)%.elf
	$(NM) -n $< > $@


# ------------------------------------------------------------------------------
# What to build
# ------------------------------------------------------------------------------

OBJ_FILES      = $(CC_FILES:.cc=.o) $(C_FILES:.c=.o) $(AS_FILES:.s=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES)) $(STARTUP_OBJ)
DEPS           = $(OBJS:.o=.d)

all:	$(BUILD_DIR) $(TARGET_HEX)


# ------------------------------------------------------------------------------
# Resources
# ------------------------------------------------------------------------------

RESOURCE_COMPILER = stmlib/tools/resources_compiler.py

resources:	$(TARGET)/resources.cc

$(TARGET)/resources.cc:	$(wildcard $(RESOURCES)/*.py)
	SAMPLE_RATE=$(SAMPLE_RATE) \
	python2.7 $(RESOURCE_COMPILER) $(RESOURCES)/resources.py


# ------------------------------------------------------------------------------
# Main rules
# ------------------------------------------------------------------------------

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET_ELF):  $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(LDFLAGS) -o $(TARGET_ELF) $(OBJS)

bin:	$(TARGET_BIN)

hex:	$(TARGET_HEX)

clean:
	$(REMOVE) $(OBJS) $(TARGETS) $(DEPS)

depends:  $(DEPS)

$(TARGET_SIZE):  $(TARGET_ELF)
	$(SIZE) $(TARGET_ELF) > $(TARGET_SIZE)

$(BUILD_DIR)$(TARGET).top_symbols: $(TARGET_ELF)
	$(NM) $(TARGET_ELF) --size-sort -C -f bsd -r > $@

size: $(TARGET_SIZE)
	cat $(TARGET_SIZE) | awk '{ print $$1+$$2 }' | tail -n1 $(FORMAT_SIZE)

ramsize: $(TARGET_SIZE)
	cat $(TARGET_SIZE) | awk '{ print $$2+$$3 }' | tail -n1 $(FORMAT_RAMSIZE)

disassemble:  build/$(TARGET)/$(TARGET).lss build/$(TARGET)/$(TARGET).top_symbols

.PHONY: all bin clean depends resources

# ------------------------------------------------------------------------------
# Bootloader merging
# ------------------------------------------------------------------------------

$(COMBO_BIN):  $(TARGET_HEX) $(BOOTLOADER_HEX)
	cat  $(TARGET_HEX) $(BOOTLOADER_HEX) | \
			awk -f stmlib/programming/merge_hex.awk > $(COMBO_HEX)
	$(OBJCOPY) -I ihex -O binary $(COMBO_HEX) $(COMBO_BIN)


# ------------------------------------------------------------------------------
# Firmware flashing
# ------------------------------------------------------------------------------

upload:	$(UPLOAD_COMMAND)

image_serial:
	python2.7 $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-r -l 131072 image.bin

upload_serial: $(TARGET_BIN)
	python2.7 $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-e $(PGM_SERIAL_VERIFY) \
		-a $(BASE_ADDRESS) \
		-w $(TARGET_BIN)

upload_serial_no_erase: $(TARGET_BIN)
	python2.7 $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) $(PGM_SERIAL_VERIFY)\
		-a $(BASE_ADDRESS) \
		-w $(TARGET_BIN)

upload_combo_serial: $(COMBO_BIN)
	python2.7 $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-e $(PGM_SERIAL_VERIFY) \
		-w $(COMBO_BIN)

upload_combo_serial_no_erase: $(COMBO_BIN)
	python2.7 $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) $(PGM_SERIAL_VERIFY)\
		-w $(COMBO_BIN)

BASE_JTAG_CMD = openocd \
			-f $(OPENOCD_SCRIPTS_PATH)interface_$(PGM_INTERFACE).cfg \
			-f $(OPENOCD_SCRIPTS_PATH)stm32$(FAMILY)_$(PGM_INTERFACE_TYPE).cfg \
			-f $(OPENOCD_SCRIPTS_PATH)prelude_$(FAMILY).cfg \

ERASE_JTAG_CMD = $(BASE_JTAG_CMD) \
			-f $(OPENOCD_SCRIPTS_PATH)erase_$(FAMILY).cfg \
			-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

UPLOAD_JTAG_CMD = $(BASE_JTAG_CMD) \
			-f $(OPENOCD_SCRIPTS_PATH)erase_$(FAMILY).cfg \
			-c "flash write_bank 0 $(TARGET_BIN) 0x0" \
			-c "verify_image $(TARGET_BIN)" \
			-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

UPLOAD_JTAG_COMBO_CMD = $(BASE_JTAG_CMD) \
			-f $(OPENOCD_SCRIPTS_PATH)erase_$(FAMILY).cfg \
			-c "flash write_bank 0 $(COMBO_BIN) 0x0" \
			-c "verify_image $(COMBO_BIN)" \
			-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

UPLOAD_JTAG_NO_ERASE_CMD = $(BASE_JTAG_CMD) \
			-c "flash write_image erase $(TARGET_BIN) 0x08000000" \
			-c "verify_image $(TARGET_BIN)" \
			-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

UPLOAD_JTAG_COMBO_NO_ERASE_CMD = $(BASE_JTAG_CMD) \
			-c "flash write_image erase $(COMBO_BIN) 0x08000000" \
			-c "verify_image $(COMBO_BIN)" \
			-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

UPLOAD_JTAG_ERASE_FIRST_CMD = $(BASE_JTAG_CMD) \
			-c "flash erase_address 0x08000000 16384" \
			-c "reset" \
			-c "flash write_image erase $(TARGET_BIN) 0x08000000" \
			-c "verify_image $(TARGET_BIN)" \
			-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

UPLOAD_JTAG_COMBO_ERASE_FIRST_CMD = $(BASE_JTAG_CMD) \
			-c "flash erase_address 0x08000000 16384" \
			-c "reset" \
			-c "flash write_image erase $(COMBO_BIN) 0x08000000" \
			-c "verify_image $(COMBO_BIN)" \
			-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

image_jtag:
	$(BASE_JTAG_CMD) \
		-c "dump_image image.bin 0x08000000 131072" \
		-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

image_jtag_256k:
	$(BASE_JTAG_CMD) \
		-c "dump_image image.bin 0x08000000 262144" \
		-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

image_jtag_1M:
	$(BASE_JTAG_CMD) \
		-c "dump_image image.bin 0x08000000 1048576" \
		-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

debug_server: $(TARGET_BIN)
	openocd \
		-f $(OPENOCD_SCRIPTS_PATH)interface_$(PGM_INTERFACE).cfg \
		-f $(OPENOCD_SCRIPTS_PATH)stm32$(FAMILY)_$(PGM_INTERFACE_TYPE).cfg \
		-c "init" \
		-c "reset" \
		-c "halt"

debug_client:
	$(DB) --eval-command="target remote localhost:3333" $(TARGET_ELF)

# Because of a weird OpenOCD bug, this command sometimes fails with an
# "Error: error executing stm32x flash write algorithm" error. Retrying
# immediately always solves it.

erase_jtag:
	$(ERASE_JTAG_CMD)

upload_jtag: $(TARGET_BIN)
	$(UPLOAD_JTAG_CMD)

upload_combo_jtag: $(COMBO_BIN)
	$(UPLOAD_JTAG_COMBO_CMD)

upload_jtag_no_erase: $(TARGET_BIN)
	$(UPLOAD_JTAG_NO_ERASE_CMD)

upload_combo_jtag_no_erase: $(COMBO_BIN)
	$(UPLOAD_JTAG_COMBO_NO_ERASE_CMD)

upload_jtag_erase_first: $(TARGET_BIN)
	$(UPLOAD_JTAG_ERASE_FIRST_CMD)

upload_combo_jtag_erase_first: $(COMBO_BIN)
	$(UPLOAD_JTAG_COMBO_ERASE_FIRST_CMD)

# Phony target for killing the system FTDI driver on OS X.
kill_ftdi:
	sudo kextunload -b com.FTDI.driver.FTDIUSBSerialDriver

reload_ftdi:
	sudo kextload -b com.FTDI.driver.FTDIUSBSerialDriver

.PHONY: upload

.PHONY: image_serial
.PHONY: upload_serial upload_serial_no_erase
.PHONY: upload_combo_serial upload_combo_serial_no_erase

.PHONY: image_jtag image_jtag_1M
.PHONY: erase_jtag
.PHONY: upload_jtag upload_jtag_no_erase upload_jtag_erase_first
.PHONY: upload_combo_jtag upload_combo_jtag_no_erase upload_combo_jtag_erase_first

.PHONY: kill_ftdi reload_ftdi
.PHONY: debug_client debug_server

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
