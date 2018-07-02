TARGET_BASENAME=$(SLUG)

EXTRAFLAGS+= -DVERSION=0.6.1 -DARCH_WIN -D_USE_MATH_DEFINES -DUSE_VST2 -DRACK_PLUGIN -DSLUG=$(SLUG) -I../../../../include/ -I../../../../dep/include -Idep/include 
EXTRAFLAGS+=

EXTRALIBS+=
# ../../Rack.lib -LIBPATH:../../dep/lib/msvc/ glew.lib glfw.lib opengl32.lib gdi32.lib user32.lib kernel32.lib Comdlg32.lib Shell32.lib

PLAF_OBJ+= 

# (note) " data with thread storage duration may not have dll interface"
#include ../../dep/yac/sharedlib_msvc.mk
include ../../../../dep/yac/staticlib_msvc.mk
