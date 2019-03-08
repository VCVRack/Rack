# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

SLUG = EH_modules
VERSION = 0.6.1

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res fx

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
