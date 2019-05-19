# called from plugin directory (plugins/community/repos/<pluginname>/)
#
include ../../../../dep/yac/install_linux.mk

TARGET_BASENAME=$(SLUG)

EXTRAFLAGS+= -DVERSION=0.6.1 -D_USE_MATH_DEFINES -DUSE_VST2 -DRACK_PLUGIN -DRACK_PLUGIN_SHARED -DSLUG=$(SLUG) -I../../../../include/ -I../../../../dep/ -I../../../../dep/include -Idep/include 
EXTRAFLAGS+=

EXTRALIBS+= ../../../Rack_shared.a
# ../../../../dep/lib/linux_gcc/

ifeq ($(BUILD_64),y)
EXTRALIBS_DEP= $(VSVR_BASE_DIR)/dep/lib/linux_gcc/x64
else
EXTRALIBS_DEP= $(VSVR_BASE_DIR)/dep/lib/linux_gcc/x86
endif

EXTRALIBS+= $(EXTRALIBS_DEP)/libspeexdsp.a $(EXTRALIBS_DEP)/glew.a $(EXTRALIBS_DEP)/jansson.a `pkg-config gtk+-2.0 --libs` -lGL

PLAF_OBJ+= 

EXTRAFLAGS+= -DARCH_LIN 

