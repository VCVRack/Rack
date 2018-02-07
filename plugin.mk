RACK_DIR ?= ../..

FLAGS += -fPIC \
	-I$(RACK_DIR)/include -I$(RACK_DIR)/dep/include

ifdef SLUG
	FLAGS += -DSLUG=$(SLUG)
endif


include $(RACK_DIR)/arch.mk

ifeq ($(ARCH), lin)
	LDFLAGS += -shared
	TARGET = plugin.so
endif

ifeq ($(ARCH), mac)
	LDFLAGS += -shared -undefined dynamic_lookup
	TARGET = plugin.dylib
endif

ifeq ($(ARCH), win)
	LDFLAGS += -shared -L$(RACK_DIR) -lRack
	TARGET = plugin.dll
endif

DISTRIBUTABLES += $(TARGET)


all: $(TARGET)

include $(RACK_DIR)/compile.mk

clean:
	rm -rfv build $(TARGET) dist

dist: all
	rm -rf dist
	mkdir -p dist/$(SLUG)
	cp -R $(DISTRIBUTABLES) dist/$(SLUG)/
	cd dist && zip -5 -r $(SLUG)-$(VERSION)-$(ARCH).zip $(SLUG)

.PHONY: clean dist