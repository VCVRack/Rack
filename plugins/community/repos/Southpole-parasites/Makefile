
SLUG = Southpole-parasites
VERSION = 0.6.0

FLAGS += \
	-DTEST -DPARASITES \
	-I./parasites \
	-Wno-unused-local-typedefs

# SOURCES += $(wildcard src/*.cpp)
SOURCES += src/Southpole.cpp
SOURCES += parasites/stmlib/utils/random.cc
SOURCES += parasites/stmlib/dsp/units.cc
SOURCES += parasites/stmlib/dsp/atan.cc

SOURCES += src/Smoke.cpp
SOURCES += parasites/clouds/dsp/correlator.cc
SOURCES += parasites/clouds/dsp/granular_processor.cc
SOURCES += parasites/clouds/dsp/mu_law.cc
SOURCES += parasites/clouds/dsp/pvoc/frame_transformation.cc
SOURCES += parasites/clouds/dsp/pvoc/phase_vocoder.cc
SOURCES += parasites/clouds/dsp/pvoc/stft.cc
SOURCES += parasites/clouds/resources.cc 

SOURCES += src/Splash.cpp
SOURCES += parasites/tides/generator.cc
SOURCES += parasites/tides/resources.cc

#SOURCES += src/Cestoda.cpp
#SOURCES += parasites/warps/dsp/modulator.cc
#SOURCES += parasites/warps/dsp/oscillator.cc
#SOURCES += parasites/warps/dsp/vocoder.cc
#SOURCES += parasites/warps/dsp/filter_bank.cc
#SOURCES += parasites/warps/resources.cc

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

