FLAGS += \
	-Iinclude \
	-Idep/include -Idep/lib/libzip/include

SOURCES = $(wildcard src/*.cpp src/*/*.cpp) \
	ext/nanovg/src/nanovg.c


include arch.mk

ifeq ($(ARCH), lin)
	SOURCES += ext/osdialog/osdialog_gtk2.c
	CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
	LDFLAGS += -rdynamic \
		-lpthread -lGL -ldl \
		$(shell pkg-config --libs gtk+-2.0) \
		-Ldep/lib -lGLEW -lglfw -ljansson -lsamplerate -lcurl -lzip -lportaudio -lportmidi
	TARGET = Rack
endif

ifeq ($(ARCH), mac)
	SOURCES += ext/osdialog/osdialog_mac.m
	CXXFLAGS += -DAPPLE -stdlib=libc++
	LDFLAGS += -stdlib=libc++ -lpthread -ldl \
		-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo \
		-Ldep/lib -lGLEW -lglfw -ljansson -lsamplerate -lcurl -lzip -lportaudio -lportmidi
	TARGET = Rack
endif

ifeq ($(ARCH), win)
	SOURCES += ext/osdialog/osdialog_win.c
	LDFLAGS += -static-libgcc -static-libstdc++ -lpthread \
		-Wl,--export-all-symbols,--out-implib,libRack.a -mwindows \
		-lgdi32 -lopengl32 -lcomdlg32 -lole32 \
		-Ldep/lib -lglew32 -lglfw3dll -ljansson -lsamplerate -lcurl -lzip -lportaudio -lportmidi
	TARGET = Rack.exe
	# OBJECTS = Rack.res
endif


all: $(TARGET)

run: $(TARGET)
ifeq ($(ARCH), lin)
	LD_LIBRARY_PATH=dep/lib ./$<
endif
ifeq ($(ARCH), mac)
	DYLD_FALLBACK_LIBRARY_PATH=dep/lib ./$<
endif
ifeq ($(ARCH), win)
	# TODO get rid of the mingw64 path
	env PATH=dep/bin:/mingw64/bin ./$<
endif

clean:
	rm -rf $(TARGET) build

# For Windows resources
%.res: %.rc
	windres $^ -O coff -o $@

include compile.mk
