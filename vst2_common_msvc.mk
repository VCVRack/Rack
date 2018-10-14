#
# Makefile for VCV rack VST2 plugin
# 
#

EXTRAFLAGS+= -DVERSION=0.6.1 -D_USE_MATH_DEFINES -Iinclude/ -Idep/include -Idep/ -DUSE_VST2
# -DSKIP_STATIC_MODULES

include vst2_common_staticlibs.mk

include vst2_common_msvc_pre.mk

PLAF_OBJ= 

include make.objects

ALL_OBJ= $(MAIN_OBJ) 

include vst2_common_msvc_post.mk
