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

TARGET := plugin
ifndef ARCH_X64
	# On non-x64, append CPU name to plugin binary
	TARGET := $(TARGET)-$(ARCH_CPU)
endif

ifdef ARCH_LIN
	TARGET := $(TARGET).so
	# This prevents static variables in the DSO (dynamic shared object) from being preserved after dlclose().
	FLAGS += -fno-gnu-unique
	# When Rack loads a plugin, it symlinks /tmp/Rack2 to its system dir, so the plugin can link to libRack.
	LDFLAGS += -Wl,-rpath=/tmp/Rack2
	# Since the plugin's compiler could be a different version than Rack's compiler, link libstdc++ and libgcc statically to avoid ABI issues.
	LDFLAGS += -static-libstdc++ -static-libgcc
	RACK_USER_DIR ?= $(HOME)/.Rack2
endif

ifdef ARCH_MAC
	TARGET := $(TARGET).dylib
	LDFLAGS += -undefined dynamic_lookup
	RACK_USER_DIR ?= $(HOME)/Documents/Rack2
endif

ifdef ARCH_WIN
	TARGET := $(TARGET).dll
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
	$(INSTALL_NAME_TOOL) -change libRack.dylib /tmp/Rack2/libRack.dylib dist/$(SLUG)/$(TARGET)
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
	cd dist && tar -c $(SLUG) | zstd -$(ZSTD_COMPRESSION_LEVEL) -o "$(SLUG)"-"$(VERSION)"-$(ARCH_NAME).vcvplugin

install: dist
	mkdir -p "$(RACK_USER_DIR)"/plugins/
	cp dist/*.vcvplugin "$(RACK_USER_DIR)"/plugins/

.PHONY: clean dist
.DEFAULT_GOAL := all
