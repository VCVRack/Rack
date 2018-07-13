# called from plugin directory (plugins/community/repos/<pluginname>/)
#
#include ../../../../arch.mk
#ifdef ARCH_WIN
include ../../../../dep/yac/install_msvc.mk
#endif

TARGET_BASENAME=$(SLUG)

EXTRAFLAGS+= -DVERSION=0.6.1 -D_USE_MATH_DEFINES -DUSE_VST2 -DRACK_PLUGIN -DRACK_PLUGIN_SHARED -DSLUG=$(SLUG) -I../../../../include/ -I../../../../dep/include -Idep/include 
EXTRAFLAGS+=

EXTRALIBS+= ../../../Rack_shared.lib

PLAF_OBJ+= 

#ifdef ARCH_WIN
include ../../../build_plugin_msvc_pre.mk
#endif
