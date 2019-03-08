SLUG = SonusModular
VERSION = 0.6.2

FLAGS += -fomit-frame-pointer -pipe -O2
CFLAGS +=
CXXFLAGS +=
LDFLAGS +=

SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += $(wildcard LICENSE*) res
RACK_DIR ?= ../..

include $(RACK_DIR)/plugin.mk
