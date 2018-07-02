SLUG = Valley
VERSION = 0.6.4

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS += -O3 -std=c99
CXXFLAGS += -O3

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES = $(wildcard src/*.cpp src/*.c src/*/*.cpp src/*/*.c)

# Must include the VCV plugin Makefile framework
RACK_DIR ?= ../..

# Convenience target for including files in the distributable release
#.PHONY: dist
#dist: all

ifndef VERSION
	$(error VERSION must be defined when making distributables)
endif
	DISTRIBUTABLES += $(wildcard LICENSE* *.pdf README*) res
	include $(RACK_DIR)/plugin.mk
