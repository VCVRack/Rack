include Makefile-arch.inc

FLAGS += \
	-I./ext -I./include

SOURCES = $(wildcard src/*.cpp src/*/*.cpp) \
	ext/nanovg/src/nanovg.c


ifeq ($(ARCH), lin)
SOURCES += ext/noc/noc_file_dialog.c
CFLAGS += -DNOC_FILE_DIALOG_GTK $(shell pkg-config --cflags gtk+-2.0)
LDFLAGS += -rdynamic \
	-lpthread -lGL -ldl \
	-lportaudio -lportmidi \
	-Ldep/lib -lGLEW -lglfw -ljansson -lsamplerate -lcurl -lzip \
	$(shell pkg-config --libs gtk+-2.0)
TARGET = Rack
endif

ifeq ($(ARCH), mac)
SOURCES += ext/noc/noc_file_dialog.m
CFLAGS += -DNOC_FILE_DIALOG_OSX
CXXFLAGS += -DAPPLE -stdlib=libc++ -I$(HOME)/local/include -I/usr/local/lib/libzip/include
LDFLAGS += -stdlib=libc++ -L$(HOME)/local/lib -lpthread -lglew -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -ldl -ljansson -lportaudio -lportmidi -lsamplerate -lcurl -lzip
TARGET = Rack
endif

ifeq ($(ARCH), win)
SOURCES += ext/noc/noc_file_dialog.c
CFLAGS += -DNOC_FILE_DIALOG_WIN32
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

clean:
	rm -rf $(TARGET) build

# For Windows resources
%.res: %.rc
	windres $^ -O coff -o $@

include Makefile.inc
