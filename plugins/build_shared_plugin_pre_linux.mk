# called from plugin directory (plugins/community/repos/<pluginname>/)
#
include ../../../../dep/yac/install_linux.mk

TARGET_BASENAME=$(SLUG)

EXTRAFLAGS+= -DVERSION=0.6.1 -D_USE_MATH_DEFINES -DUSE_VST2 -DRACK_PLUGIN -DRACK_PLUGIN_SHARED -DSLUG=$(SLUG) -I../../../../include/ -I../../../../dep/include -Idep/include 
EXTRAFLAGS+=

EXTRALIBS+= ../../../Rack_shared.a
# ../../../../dep/lib/linux_gcc/

PLAF_OBJ+= 

EXTRAFLAGS+= -DARCH_LIN 

