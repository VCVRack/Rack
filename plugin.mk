# All paths here assume the PWD is plugin/something

FLAGS += -fPIC \
	-I../../include


include ../../arch.mk

ifeq ($(ARCH), lin)
LDFLAGS += -shared
TARGET = plugin.so
endif

ifeq ($(ARCH), mac)
LDFLAGS += -shared -undefined dynamic_lookup
TARGET = plugin.dylib
endif

ifeq ($(ARCH), win)
LDFLAGS += -shared -L../../ -lRack
TARGET = plugin.dll
endif


all: $(TARGET)

clean:
	rm -rfv build $(TARGET)

include ../../compile.mk
