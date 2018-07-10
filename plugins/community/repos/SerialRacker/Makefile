RACK_DIR ?= ../..
SLUG = SerialRacker
VERSION = 0.6.1

FLAGS += -Idep/include
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += $(wildcard LICENSE*) res

# Dependencies
$(shell mkdir -p dep)
DEP_LOCAL := dep

include $(RACK_DIR)/plugin.mk
