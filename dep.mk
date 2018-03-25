include $(RACK_DIR)/arch.mk

# The install location for `make install`
DEP_LOCAL ?= .
DEP_FLAGS += -g -O3 -march=nocona

ifeq ($(ARCH), mac)
	DEP_FLAGS += -mmacosx-version-min=10.7 -stdlib=libc++
	DEP_LDFLAGS += -mmacosx-version-min=10.7 -stdlib=libc++
endif

DEP_CFLAGS += $(DEP_FLAGS)
DEP_CXXFLAGS += $(DEP_FLAGS)

# Commands
WGET := curl -OL
UNTAR := tar xf
UNZIP := unzip
CONFIGURE := ./configure --prefix="$(realpath $(DEP_LOCAL))"
ifeq ($(ARCH), win)
	CMAKE := cmake -G 'MSYS Makefiles' -DCMAKE_INSTALL_PREFIX="$(realpath $(DEP_LOCAL))"
else
	CMAKE := cmake -DCMAKE_INSTALL_PREFIX="$(realpath $(DEP_LOCAL))"
endif

# Export environment for all dependency targets
$(DEPS): export CFLAGS = $(DEP_CFLAGS)
$(DEPS): export CXXFLAGS = $(DEP_CXXFLAGS)
$(DEPS): export LDFLAGS = $(DEP_LDFLAGS)

dep: $(DEPS)

.PHONY: dep
