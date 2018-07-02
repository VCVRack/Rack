EXTRAFLAGS+= -DARCH_WIN 
EXTRAFLAGS+= -I../../dev/vstsdk2.4/pluginterfaces/vst2.x/

EXTRALIBS= Rack.lib -LIBPATH:dep/lib/msvc/ libspeexdsp.lib glew.lib glfw.lib opengl32.lib gdi32.lib user32.lib kernel32.lib Comdlg32.lib Shell32.lib

plugin_lib = $(PLUGIN_DIR)/$(1)/$(1).lib
