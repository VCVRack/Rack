# Detect architecture if ARCH is not defined

ifndef ARCH

MACHINE = $(shell $(CC) -dumpmachine)
ifneq (, $(findstring linux, $(MACHINE)))
	# Linux
	ARCH = lin
else ifneq (, $(findstring apple, $(MACHINE)))
	# Mac
	ARCH = mac
else ifneq (, $(findstring mingw, $(MACHINE)))
	# Windows
	ARCH = win
ifneq ( ,$(findstring x86_64, $(MACHINE)))
	BITS = 64
else ifneq (, $(findstring i686, $(MACHINE)))
	BITS = 32
endif
else
$(error Could not determine machine type. Try hacking around in arch.mk)
endif

endif