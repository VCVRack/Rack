RACK_DIR ?= ../..
SLUG = Fundamental
VERSION = 0.6.2

FLAGS += -I./libsamplerate-0.1.9/src
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard libsamplerate-0.1.9/src/*.c)
DISTRIBUTABLES += $(wildcard LICENSE*) res

libsamplerate := libsamplerate-0.1.9
DEPS += $(libsamplerate)

$(libsamplerate):
	mkdir -p dep
	$(WGET) "http://www.mega-nerd.com/SRC/libsamplerate-0.1.9.tar.gz"
	$(UNTAR) libsamplerate-0.1.9.tar.gz
	cp config.h libsamplerate-0.1.9/src/


include $(RACK_DIR)/plugin.mk
