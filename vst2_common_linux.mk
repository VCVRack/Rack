#
# Makefile for VCV rack VST2 plugin
# 
#

EXTRAFLAGS+= -DVERSION=0.6.1 -D_USE_MATH_DEFINES -Iinclude/ -Idep/include -Idep/ -DUSE_VST2

EXTRALIBS+= src/plugin_static.o

## note: remove -DSKIP_STATIC_MODULES in makefile_lib.linux and uncomment the following line to link the add-on modules
include vst2_common_staticlibs.mk
##EXTRALIBS+= $(call plugin_lib,Template)

EXTRALIBS+= Rack.a

include vst2_common_linux_pre.mk

PLAF_OBJ= 

include make.objects

ALL_OBJ= $(MAIN_OBJ) 

include vst2_common_linux_post.mk
