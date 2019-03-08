SLUG = JW-Modules
VERSION = 0.6.3
DISTRIBUTABLES += $(wildcard LICENSE*) res
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES = $(wildcard src/*.cpp)


# Must include the VCV plugin Makefile framework
include $(RACK_DIR)/plugin.mk
