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
LDFLAGS += -L$(RACK_DIR) -lRack

include $(RACK_DIR)/arch.mk

ifdef ARCH_LIN
	TARGET := plugin.so
	# This prevents static variables in the DSO (dynamic shared object) from being preserved after dlclose().
	# I don't really understand the side effects (see GCC manual), but so far tests are positive.
	FLAGS += -fno-gnu-unique
	LDFLAGS += -Wl,-rpath=.
	# Since the compiler we're using could have a newer version than the minimum supported libstdc++ version, link it statically.
	LDFLAGS += -static-libstdc++
	RACK_USER_DIR ?= $(HOME)/.Rack
endif

ifdef ARCH_MAC
	TARGET := plugin.dylib
	LDFLAGS += -undefined dynamic_lookup
	RACK_USER_DIR ?= $(HOME)/Documents/Rack
endif

ifdef ARCH_WIN
	TARGET := plugin.dll
	LDFLAGS += -static-libstdc++
	RACK_USER_DIR ?= $(USERPROFILE)/Documents/Rack
endif


DEP_FLAGS += -fPIC
include $(RACK_DIR)/dep.mk


all: $(TARGET)

include $(RACK_DIR)/compile.mk

clean:
	rm -rfv build $(TARGET) dist

dist: all
	rm -rf dist
	mkdir -p dist/$(SLUG)
	@# Strip and copy plugin binary
	cp $(TARGET) dist/$(SLUG)/
ifdef ARCH_MAC
	$(STRIP) -S dist/$(SLUG)/$(TARGET)
	install_name_tool -change libRack.dylib @executable_path/libRack.dylib dist/$(SLUG)/$(TARGET)
	otool -L dist/$(SLUG)/$(TARGET)
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
	cd dist && tar -c $(SLUG) | zstd -19 -o $(SLUG)-"$(VERSION)"-$(ARCH).vcvplugin

install: dist
	mkdir -p "$(RACK_USER_DIR)"/plugins-v2/
	cp dist/$(SLUG)-"$(VERSION)"-$(ARCH).vcvplugin "$(RACK_USER_DIR)"/plugins-v2/

.PHONY: clean dist
.DEFAULT_GOAL := all
