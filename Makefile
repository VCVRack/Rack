include Makefile-arch.inc

FLAGS += \
	-Iinclude \
	-Idep/include -Idep/lib/libzip/include

SOURCES = $(wildcard src/*.cpp src/*/*.cpp) \
	ext/nanovg/src/nanovg.c


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
	CXXFLAGS += -DGLEW_STATIC \
		-I$(HOME)/pkg/portaudio-r1891-build/include -I/mingw64/lib/libzip/include -I$(HOME)/local/include
	LDFLAGS += \
		-Wl,-Bstatic,--whole-archive \
		-ljansson -lsamplerate \
		-Wl,-Bdynamic,--no-whole-archive \
		-lpthread -lglfw3 -lgdi32 -lglew32 -lopengl32 -lcomdlg32 -lole32 -lzip \
		-L $(HOME)/local/lib -lcurl \
		-lportmidi \
		-L$(HOME)/pkg/portaudio-r1891-build/lib/x64/ReleaseMinDependency -lportaudio_x64 \
		-Wl,--export-all-symbols,--out-implib,libRack.a -mwindows
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

clean:
	rm -rf $(TARGET) build

# For Windows resources
%.res: %.rc
	windres $^ -O coff -o $@

include Makefile.inc
