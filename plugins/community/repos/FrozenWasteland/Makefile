SLUG = FrozenWasteland
VERSION = 0.6.7

# FLAGS will be passed to both the C and C++ compiler
FLAGS += \
	-DTEST \
	-I./eurorack \
	-I./src/dsp-delay \
	-I./src/dsp-filter/utils -I./src/dsp-filter/filters -I./src/dsp-filter/third-party/falco \
	-Wno-unused-local-typedefs

CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
#SOURCES += eurorack/stmlib/utils/random.cc
#SOURCES += eurorack/stmlib/dsp/atan.cc
#SOURCES += eurorack/stmlib/dsp/units.cc
#SOURCES += eurorack/clouds/dsp/correlator.cc
#SOURCES += eurorack/clouds/dsp/granular_processor.cc
#SOURCES += eurorack/clouds/dsp/mu_law.cc
#SOURCES += eurorack/clouds/dsp/pvoc/frame_transformation.cc
#SOURCES += eurorack/clouds/dsp/pvoc/phase_vocoder.cc
#SOURCES += eurorack/clouds/dsp/pvoc/stft.cc
#SOURCES += eurorack/clouds/resources.cc
SOURCES += $(wildcard src/*.cpp src/filters/*.cpp src/dsp-noise/*.cpp src/dsp-filter/*.cpp rc/dsp-delay/*.hpp src/stmlib/*.cc)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

# Include the VCV plugin Makefile framework
RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk
