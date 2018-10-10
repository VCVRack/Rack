EXTRAFLAGS+= -DARCH_LIN
EXTRAFLAGS+= -I../../dev/vstsdk2.4/pluginterfaces/vst2.x/

EXTRALIBS= 

ifeq ($(BUILD_64),y)
EXTRALIBS_DEP= dep/lib/linux_gcc/x64
else
EXTRALIBS_DEP= dep/lib/linux_gcc/x86
endif

EXTRALIBS+= $(EXTRALIBS_DEP)/libspeexdsp.a $(EXTRALIBS_DEP)/glew.a -lGL

plugin_lib = $(PLUGIN_DIR)/$(1)/$(1).a
