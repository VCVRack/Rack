# All paths here assume the PWD is plugin/something

include ../../Makefile-arch.inc

FLAGS += -fPIC \
	-I../../include


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

include ../../Makefile.inc
