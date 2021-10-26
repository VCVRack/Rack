include $(RACK_DIR)/arch.mk

# The install location for `make install`
DEP_LOCAL ?= dep
$(shell mkdir -p $(DEP_LOCAL))
DEP_PATH := $(abspath $(DEP_LOCAL))

DEP_FLAGS += -g -O3 -march=nehalem
# This is needed for Rack for DAWs.
# Static libs don't usually compiled with -fPIC, but since we're including them in a shared library, it's needed.
DEP_FLAGS += -fPIC

ifdef ARCH_MAC
	DEP_MAC_SDK_FLAGS := -mmacosx-version-min=10.9
	DEP_FLAGS += $(DEP_MAC_SDK_FLAGS) -stdlib=libc++
	DEP_LDFLAGS += $(DEP_MAC_SDK_FLAGS) -stdlib=libc++
endif

DEP_CFLAGS += $(DEP_FLAGS)
DEP_CXXFLAGS += $(DEP_FLAGS)

# Commands
WGET := wget -c
UNTAR := tar xf
UNZIP := unzip -o
CONFIGURE := ./configure --prefix="$(DEP_PATH)" --host=$(MACHINE)

ifdef ARCH_WIN
	CMAKE := cmake -G 'MSYS Makefiles' -DCMAKE_INSTALL_PREFIX="$(DEP_PATH)"
else
	CMAKE := cmake -DCMAKE_INSTALL_PREFIX="$(DEP_PATH)"
endif
# Some platforms try to install to lib64
CMAKE += -DCMAKE_INSTALL_LIBDIR=lib

ifdef ARCH_MAC
	SHA256SUM := shasum -a 256
	SED := sed -i ''
else
	SHA256SUM := sha256sum
	SED := sed -i
endif
SHA256 := sha256check() { echo "$$2  $$1" | $(SHA256SUM) -c; }; sha256check


# Export environment for all dependency targets
$(DEPS): export CFLAGS = $(DEP_CFLAGS)
$(DEPS): export CXXFLAGS = $(DEP_CXXFLAGS)
$(DEPS): export LDFLAGS = $(DEP_LDFLAGS)

dep: $(DEPS)

cleandep:
ifeq ($(DEP_LOCAL), .)
	$(error Refusing to clean cwd)
endif
	rm -rfv $(DEP_LOCAL)

.PHONY: dep
