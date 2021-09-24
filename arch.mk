MACHINE = $(shell $(CC) -dumpmachine)

ifneq (,$(findstring x86_64-,$(MACHINE)))
	ARCH_x64 := 1
	ARCH_NAME := x64
else ifneq (,$(findstring i686-,$(MACHINE)))
	ARCH_x86 := 1
	ARCH_NAME := x86
else
$(error Could not determine CPU architecture of $(MACHINE). Try hacking around in arch.mk)
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
$(error Could not determine OS of $(MACHINE). Try hacking around in arch.mk)
endif
