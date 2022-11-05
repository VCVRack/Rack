MACHINE := $(shell $(CC) -dumpmachine)

ifneq (,$(findstring x86_64-,$(MACHINE)))
	ARCH_X64 := 1
	ARCH_CPU := x64
else ifneq (,$(findstring arm64-,$(MACHINE)))
	ARCH_ARM64 := 1
	ARCH_CPU := arm64
else
$(error Could not determine CPU architecture of $(MACHINE))
endif

ifneq (,$(findstring -darwin,$(MACHINE)))
	ARCH_MAC := 1
	ARCH_OS := mac
else ifneq (,$(findstring -mingw32,$(MACHINE)))
	ARCH_WIN := 1
	ARCH_OS := win
else ifneq (,$(findstring -linux,$(MACHINE)))
	ARCH_LIN := 1
	ARCH_OS := lin
else
$(error Could not determine operating system of $(MACHINE))
endif

ARCH_NAME := $(ARCH_OS)-$(ARCH_CPU)
