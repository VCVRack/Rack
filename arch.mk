MACHINE := $(shell $(CC) -dumpmachine)

ifneq (,$(findstring x86_64-,$(MACHINE)))
	ARCH_X64 := 1
	ARCH_NAME := x64
else ifneq (,$(findstring arm64-,$(MACHINE)))
	ARCH_ARM64 := 1
	ARCH_NAME := arm64
else
$(error Could not determine CPU architecture of $(MACHINE))
endif

ifneq (,$(findstring -darwin,$(MACHINE)))
	ARCH_MAC := 1
	ARCH_OS_NAME := mac
else ifneq (,$(findstring -mingw32,$(MACHINE)))
	ARCH_WIN := 1
	ARCH_OS_NAME := win
else ifneq (,$(findstring -linux,$(MACHINE)))
	ARCH_LIN := 1
	ARCH_OS_NAME := lin
else
$(error Could not determine operating system of $(MACHINE))
endif


# The architecture "full name" is used in package filenames.
# Examples: win, lin, mac, mac_arm64
ifdef ARCH_X64
	# Omit arch name for x64
	ARCH_FULL_NAME := $(ARCH_OS_NAME)
else
	ARCH_FULL_NAME := $(ARCH_OS_NAME)_$(ARCH_NAME)
endif
