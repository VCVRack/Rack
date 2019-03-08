SLUG = ML_modules
VERSION = 0.6.4

FLAGS += -D v_dev

SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard freeverb/*.cpp)

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk
