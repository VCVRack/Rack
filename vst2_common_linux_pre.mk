EXTRAFLAGS+= -DARCH_LIN
EXTRAFLAGS+= -I"$(VST2_SDK_DIR)"

ifeq ($(BUILD_64),y)
EXTRALIBS_DEP= dep/lib/linux_gcc/x64
else
EXTRALIBS_DEP= dep/lib/linux_gcc/x86
endif

EXTRALIBS+= $(EXTRALIBS_DEP)/libspeexdsp.a $(EXTRALIBS_DEP)/glew.a $(EXTRALIBS_DEP)/jansson.a `pkg-config gtk+-2.0 --libs` -lGL -ldl

plugin_lib = $(PLUGIN_DIR)/$(1)/$(1).a
