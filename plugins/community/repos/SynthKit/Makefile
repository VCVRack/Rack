SLUG = SynthKit
VERSION = 0.6.2

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Controllers
CONTROLLERS += $(wildcard src/controller/*.cpp)

# SynthDevKit
SYNTHDEVKIT += $(wildcard deps/SynthDevKit/src/*.cpp)

# Views
VIEWS += $(wildcard src/view/*.cpp)

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp) $(CONTROLLERS) $(VIEWS) $(SYNTHDEVKIT)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res deps/rack-components/res

# Must include the VCV plugin Makefile framework
RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

# Sources to test for ArpTest - this will usually only include your controllers
TEST_SOURCES += $(CONTROLLERS) $(SYNTHDEVKIT)

# Add any tests
TEST_SOURCES += $(wildcard tests/*.cpp)

# By default, ARPTEST_DIR is arptest in your current directory, but you can override it
# In this example, arptest lives one path down
ARPTEST_DIR ?= ./arptest

include $(ARPTEST_DIR)/build.mk
