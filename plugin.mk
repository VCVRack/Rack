ifndef RACK_DIR
$(error RACK_DIR is not defined)
endif

SLUG := $(shell jq .slug plugin.json)
VERSION := $(shell jq .version plugin.json)

ifndef SLUG
$(error SLUG could not be found in manifest)
endif
ifndef VERSION
$(error VERSION could not be found in manifest)
endif

DISTRIBUTABLES += plugin.json

FLAGS += -fPIC
FLAGS += -I$(RACK_DIR)/include -I$(RACK_DIR)/dep/include

include $(RACK_DIR)/arch.mk

ifdef ARCH_LIN
	LDFLAGS += -shared
	TARGET := plugin.so
	RACK_USER_DIR ?= $(HOME)/.Rack
	# Link to glibc 2.23
# 	FLAGS += -include force_link_glibc_2.23.h
endif

ifdef ARCH_MAC
	LDFLAGS += -shared -undefined dynamic_lookup
	TARGET := plugin.dylib
	RACK_USER_DIR ?= $(HOME)/Documents/Rack
endif

ifdef ARCH_WIN
	LDFLAGS += -shared -L$(RACK_DIR) -lRack
	TARGET := plugin.dll
	RACK_USER_DIR ?= "$(USERPROFILE)"/Documents/Rack
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
	cd dist && zip -q -9 -r $(SLUG)-$(VERSION)-$(ARCH).zip $(SLUG)

install: dist
	cp dist/$(SLUG)-$(VERSION)-$(ARCH).zip $(RACK_USER_DIR)/plugins-v1/

.PHONY: clean dist
.DEFAULT_GOAL := all
