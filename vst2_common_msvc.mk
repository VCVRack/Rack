#
# Makefile for VCV rack VST2 plugin
# 
#

EXTRAFLAGS+= -DVERSION=0.6.1 -D_USE_MATH_DEFINES -Iinclude/ -Idep/include -Idep/ -DUSE_VST2

include vst2_common_msvc_pre.mk

include vst2_common_staticlibs.mk

PLAF_OBJ= 

include make.objects

ALL_OBJ= $(MAIN_OBJ) 

include vst2_common_msvc_post.mk
