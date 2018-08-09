EXTRAFLAGS+= -DARCH_WIN
EXTRAFLAGS+= -DVST2_REPARENT_WINDOW_HACK 
EXTRAFLAGS+= -I../../dev/vstsdk2.4/pluginterfaces/vst2.x/

EXTRALIBS= 

ifeq ($(BUILD_64),y)
EXTRALIBS+= -LIBPATH:dep/lib/msvc/x64
else
EXTRALIBS+= -LIBPATH:dep/lib/msvc/x86
endif

EXTRALIBS+= libspeexdsp.lib glew.lib opengl32.lib gdi32.lib user32.lib kernel32.lib Comdlg32.lib Shell32.lib ws2_32.lib winmm.lib ole32.lib
#glfw.lib

plugin_lib = $(PLUGIN_DIR)/$(1)/$(1).lib
