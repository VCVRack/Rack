ifndef RACK_DIR
$(error RACK_DIR is not defined)
endif

SLUG := $(shell jq -r .slug plugin.json)
VERSION := $(shell jq -r .version plugin.json)

ifndef SLUG
$(error SLUG could not be found in manifest)
endif
ifndef VERSION
$(error VERSION could not be found in manifest)
endif

DISTRIBUTABLES += plugin.json

FLAGS += -fPIC
FLAGS += -I$(RACK_DIR)/include -I$(RACK_DIR)/dep/include

LDFLAGS += -shared
# Plugins must link to libRack because when Rack is used as a plugin of another application, its symbols are not available to subsequently loaded shared libraries.
LDFLAGS += -L$(RACK_DIR) -lRack

include $(RACK_DIR)/arch.mk

ifdef ARCH_LIN
	TARGET := plugin.so
	# This prevents static variables in the DSO (dynamic shared object) from being preserved after dlclose().
	# I don't really understand the side effects (see GCC manual), but so far tests are positive.
	FLAGS += -fno-gnu-unique
	LDFLAGS += -Wl,-rpath=.
	# Since the plugin's compiler could be a different version than Rack's compiler, link libstdc++ and libgcc statically to avoid ABI issues.
	LDFLAGS += -static-libstdc++ -static-libgcc
	RACK_USER_DIR ?= $(HOME)/.Rack2
endif

ifdef ARCH_MAC
	TARGET := plugin.dylib
	LDFLAGS += -undefined dynamic_lookup
	RACK_USER_DIR ?= $(HOME)/Documents/Rack2
endif

ifdef ARCH_WIN
	TARGET := plugin.dll
	LDFLAGS += -static-libstdc++
	RACK_USER_DIR ?= $(USERPROFILE)/Documents/Rack2
endif


DEP_FLAGS += -fPIC
include $(RACK_DIR)/dep.mk


all: $(TARGET)

include $(RACK_DIR)/compile.mk

clean:
	rm -rfv build $(TARGET) dist

ZSTD_COMPRESSION_LEVEL ?= 19

dist: all
	rm -rf dist
	mkdir -p dist/$(SLUG)
	@# Strip and copy plugin binary
	cp $(TARGET) dist/$(SLUG)/
ifdef ARCH_MAC
	$(STRIP) -S dist/$(SLUG)/$(TARGET)
	$(INSTALL_NAME_TOOL) -change libRack.dylib @rpath/libRack.dylib dist/$(SLUG)/$(TARGET)
	$(INSTALL_NAME_TOOL) -add_rpath . dist/$(SLUG)/$(TARGET)
	$(OTOOL) -L dist/$(SLUG)/$(TARGET)
else
	$(STRIP) -s dist/$(SLUG)/$(TARGET)
endif
	@# Copy distributables
ifdef ARCH_MAC
	rsync -rR $(DISTRIBUTABLES) dist/$(SLUG)/
else
	cp -r --parents $(DISTRIBUTABLES) dist/$(SLUG)/
endif
	@# Create ZIP package
	cd dist && tar -c $(SLUG) | zstd -$(ZSTD_COMPRESSION_LEVEL) -o "$(SLUG)"-"$(VERSION)"-$(ARCH_OS_NAME).vcvplugin

install: dist
	mkdir -p "$(RACK_USER_DIR)"/plugins/
	cp dist/*.vcvplugin "$(RACK_USER_DIR)"/plugins/

.PHONY: clean dist
.DEFAULT_GOAL := all
