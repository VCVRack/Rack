# All paths here assume the PWD is plugin/something

FLAGS += -fPIC \
	-I../../include -I../../dep/include


include ../../arch.mk

ifeq ($(ARCH), lin)
	LDFLAGS += -shared
	PLUGIN_EXTENSION = so
endif

ifeq ($(ARCH), mac)
	LDFLAGS += -shared -undefined dynamic_lookup
	PLUGIN_EXTENSION = dylib
endif

ifeq ($(ARCH), win)
	LDFLAGS += -shared -L../../ -lRack
	PLUGIN_EXTENSION = dll
endif


clean:
	rm -rfv build *.$(PLUGIN_EXTENSION) dist

include ../../compile.mk
