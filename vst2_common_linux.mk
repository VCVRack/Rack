#
# Makefile for VCV rack VST2 plugin
# 
#

EXTRAFLAGS+= -DVERSION=0.6.1 -D_USE_MATH_DEFINES -Iinclude/ -Idep/include -Idep/ -DUSE_VST2

EXTRALIBS+= src/plugin_static.o

## See setenv_linux.sh (export RACK_STATIC_MODULES=y/n)
ifeq ($(RACK_STATIC_MODULES),y)
include vst2_common_staticlibs.mk
endif
##EXTRALIBS+= $(call plugin_lib,Template)

EXTRALIBS+= Rack.a

include vst2_common_linux_pre.mk

PLAF_OBJ= 

include make.objects

ALL_OBJ= $(MAIN_OBJ) 

include vst2_common_linux_post.mk
