MACHINE = $(shell $(CC) -dumpmachine)
ifneq (, $(findstring apple, $(MACHINE)))
	ARCH_MAC := 1
	ARCH := mac
else ifneq (, $(findstring mingw, $(MACHINE)))
	ARCH_WIN := 1
	ARCH := win
	ifneq ( ,$(findstring x86_64, $(MACHINE)))
		ARCH_WIN_64 := 1
		BITS := 64
	else ifneq (, $(findstring i686, $(MACHINE)))
		ARCH_WIN_32 := 1
		BITS := 32
	endif
else ifneq (, $(findstring linux, $(MACHINE)))
	ARCH_LIN := 1
	ARCH := lin
else
$(error Could not determine architecture of $(MACHINE). Try hacking around in arch.mk)
endif
