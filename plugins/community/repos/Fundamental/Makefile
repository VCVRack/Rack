RACK_DIR ?= ../..
SLUG = Fundamental
VERSION = 0.6.0

FLAGS += -Idep/include
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += $(wildcard LICENSE*) res

libsamplerate = dep/lib/libsamplerate.a
OBJECTS += $(libsamplerate)

include $(RACK_DIR)/plugin.mk

# Dependencies

$(shell mkdir -p dep)
DEP_FLAGS += -fPIC
DEP_LOCAL := dep

DEPS += $(libsamplerate)
include $(RACK_DIR)/dep.mk

$(libsamplerate):
	cd dep && $(WGET) http://www.mega-nerd.com/SRC/libsamplerate-0.1.9.tar.gz
	cd dep && $(UNTAR) libsamplerate-0.1.9.tar.gz
	cd dep/libsamplerate-0.1.9 && $(CONFIGURE)
	cd dep/libsamplerate-0.1.9 && $(MAKE)
	cd dep/libsamplerate-0.1.9 && $(MAKE) install
