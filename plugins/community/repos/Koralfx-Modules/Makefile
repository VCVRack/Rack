# Must follow the format in the Naming section of https://vcvrack.com/manual/PluginDevelopmentTutorial.html
SLUG = Koralfx-Modules

# Must follow the format in the Versioning section of https://vcvrack.com/manual/PluginDevelopmentTutorial.html
VERSION = 0.6.9

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES = $(wildcard src/*.cpp src/*.c src/*/*.cpp src/*/*.c)

# Must include the VCV plugin Makefile framework
RACK_DIR ?= ../..

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk


# Convenience target for including files in the distributable release
#.PHONY: dist
#dist: all

ifndef VERSION
	$(error VERSION must be defined when making distributables)
endif
	DISTRIBUTABLES += $(wildcard LICENSE* *.pdf README*) res
