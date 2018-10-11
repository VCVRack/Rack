# called from plugin directory (plugins/community/repos/<pluginname>/)
#
include ../../../../dep/yac/install_msvc.mk

TARGET_BASENAME=$(SLUG)

EXTRAFLAGS+= -DVERSION=0.6.1 -D_USE_MATH_DEFINES -DUSE_VST2 -DRACK_PLUGIN -DSLUG=$(SLUG) -I../../../../include/ -I../../../../dep/include -I../../../../dep/ -Idep/include

EXTRALIBS+=

PLAF_OBJ+= 

EXTRAFLAGS+= -DARCH_WIN 

EXTRALIBS+=
# ../../Rack.lib -LIBPATH:../../dep/lib/msvc/ glew.lib glfw.lib opengl32.lib gdi32.lib user32.lib kernel32.lib Comdlg32.lib Shell32.lib
