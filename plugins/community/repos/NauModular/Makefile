SLUG = NauModular
VERSION = 0.6.1

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
ifeq ($(ARCH), win)
	LDFLAGS += -lws2_32
else
	LDFLAGS +=
endif
