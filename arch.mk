MACHINE := $(shell $(CC) -dumpmachine)
MACHINE_LIST := $(subst -, ,$(MACHINE))
MACHINE_ARCH := $(word 1, $(MACHINE_LIST))
MACHINE_VENDOR := $(word 2, $(MACHINE_LIST))
MACHINE_OS := $(word 3, $(MACHINE_LIST))

ifeq ($(MACHINE_ARCH),x86_64)
	ARCH_X64 := 1
	ARCH_NAME := x64
else ifeq ($(MACHINE_ARCH),arm64)
	ARCH_ARM64 := 1
	ARCH_NAME := arm64
else
$(error CPU architecture $(MACHINE_ARCH) not supported)
endif

ifneq (,$(findstring darwin,$(MACHINE_OS)))
	ARCH_MAC := 1
	ARCH_OS_NAME := mac
else ifneq (,$(findstring mingw32,$(MACHINE_OS)))
	ARCH_WIN := 1
	ARCH_OS_NAME := win
else ifneq (,$(findstring linux,$(MACHINE_OS)))
	ARCH_LIN := 1
	ARCH_OS_NAME := lin
else
$(error Operating system $(MACHINE_OS) not supported)
endif


ARCH_FULL_NAME := $(ARCH_OS_NAME)
ifndef ARCH_X64
	ARCH_FULL_NAME := $(ARCH_FULL_NAME)_$(ARCH_NAME)
endif
