SLUG = Befaco
VERSION = 0.6.0

FLAGS = -I./pffft -DPFFFT_SIMD_DISABLE

SOURCES += $(wildcard src/*.cpp) pffft/pffft.c

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk
