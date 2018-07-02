# Must follow the format in the Naming section of https://vcvrack.com/manual/PluginDevelopmentTutorial.html
SLUG = squinkylabs-plug1

# Must follow the format in the Versioning section of https://vcvrack.com/manual/PluginDevelopmentTutorial.html
VERSION = 0.6.5

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -I./dsp/generators -I./dsp/utils -I./dsp/filters
FLAGS += -I./dsp/third-party/falco -I./dsp/third-party/kiss_fft130  -I./dsp/third-party/kiss_fft130/tools
FLAGS += -I./sqsrc/thread -I./dsp/fft -I./composites
FLAGS += -I./sqsrc/noise -I./sqsrc/util -I./sqsrc/clock
CFLAGS +=
CXXFLAGS +=

# Command line variable to turn on "experimental" modules
ifdef _EXP
	FLAGS += -D _EXP
endif
ifdef _CPU_HOG
	FLAGS += -D _CPU_HOG
endif
# Macro to use on any target where we don't normally want asserts
ASSERTOFF = -D NDEBUG

# Make _ASSERT=true will nullify our ASSERTOFF flag, thus allowing them
ifdef _ASSERT
	ASSERTOFF =
endif

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS += -lpthread

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard dsp/**/*.cpp)
SOURCES += $(wildcard dsp/third-party/falco/*.cpp)
SOURCES += dsp/third-party/kiss_fft130/kiss_fft.c
SOURCES += dsp/third-party/kiss_fft130/tools/kiss_fftr.c
SOURCES += $(wildcard sqsrc/**/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

# If RACK_DIR is not defined when calling the Makefile, default to two levels above
RACK_DIR ?= ../..

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

# This turns asserts off for make (plugin), not for test or perf
$(TARGET) :  FLAGS += $(ASSERTOFF)

include test.mk

