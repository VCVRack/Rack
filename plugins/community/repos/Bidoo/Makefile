RACK_DIR ?= ../..
SLUG = Bidoo
VERSION = 0.6.10
DISTRIBUTABLES += $(wildcard LICENSE*) res

FLAGS += -Idep/include -I./src/dep/dr_wav -I./src/dep/filters -I./src/dep/freeverb -I./src/dep/minimp3

SOURCES = $(wildcard src/*.cpp src/dep/filters/*cpp src/dep/freeverb/*cpp)

include $(RACK_DIR)/plugin.mk
