ifndef RACK_DIR
$(error RACK_DIR is not defined)
endif

ifndef SLUG
$(error SLUG is not defined)
endif

FLAGS += -DSLUG=$(SLUG)
FLAGS += -fPIC
FLAGS += -I$(RACK_DIR)/include -I$(RACK_DIR)/dep/include


include $(RACK_DIR)/arch.mk

ifeq ($(ARCH), lin)
	LDFLAGS += -shared
	TARGET := plugin.so
endif

ifeq ($(ARCH), mac)
	LDFLAGS += -shared -undefined dynamic_lookup
	TARGET := plugin.dylib
endif

ifeq ($(ARCH), win)
	LDFLAGS += -shared -L$(RACK_DIR) -lRack
	TARGET := plugin.dll
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
	# Strip and copy plugin binary
	cp $(TARGET) dist/$(SLUG)/
ifeq ($(ARCH), mac)
	strip -x dist/$(SLUG)/$(TARGET)
else
	strip -s dist/$(SLUG)/$(TARGET)
endif
	# Copy distributables
	cp -R $(DISTRIBUTABLES) dist/$(SLUG)/
	# Create ZIP package
	cd dist && zip -5 -r $(SLUG)-$(VERSION)-$(ARCH).zip $(SLUG)


.PHONY: clean dist
.DEFAULT_GOAL := all
