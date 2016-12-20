ARCH ?= linux
CFLAGS = -MMD -g -Wall -O2
CXXFLAGS = -MMD -g -Wall -std=c++11 -O2 -ffast-math -fno-exceptions \
	-I./lib -I./include
LDFLAGS =

SOURCES = $(wildcard src/*.cpp src/*/*.cpp) \
	lib/nanovg/src/nanovg.c


# Linux
ifeq ($(ARCH), linux)
CC = gcc
CXX = g++
SOURCES += lib/noc/noc_file_dialog.c
CFLAGS += -DNOC_FILE_DIALOG_GTK $(shell pkg-config --cflags gtk+-2.0)
CXXFLAGS += -DLINUX
LDFLAGS += -lpthread -lGL -lGLEW -lglfw -ldl -ljansson -lportaudio -lportmidi \
	$(shell pkg-config --libs gtk+-2.0)
TARGET = Rack
endif

# Apple
ifeq ($(ARCH), apple)
CC = clang
CXX = clang++
SOURCES += lib/noc/noc_file_dialog.m
CFLAGS += -DNOC_FILE_DIALOG_OSX
CXXFLAGS += -DAPPLE -stdlib=libc++ -I$(HOME)/local/include
LDFLAGS += -stdlib=libc++ -L$(HOME)/local/lib -lpthread -lglew -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -ldl -ljansson -lportaudio -lportmidi
TARGET = Rack
endif

# Windows
ifeq ($(ARCH), windows)
CC = x86_64-w64-mingw32-gcc
CXX = x86_64-w64-mingw32-g++
SOURCES += lib/noc/noc_file_dialog.c
CFLAGS += -DNOC_FILE_DIALOG_WIN32
CXXFLAGS += -DWINDOWS -D_USE_MATH_DEFINES \
	-I$(HOME)/pkg/portaudio-r1891-build/include
LDFLAGS += -lpthread \
	-lglfw3 -lgdi32 -lopengl32 -lglew32 \
	-lcomdlg32 -lole32 \
	-ljansson -lportmidi \
	-L$(HOME)/pkg/portaudio-r1891-build/lib/x64/ReleaseMinDependency -lportaudio_x64 \
	-Wl,--export-all-symbols,--out-implib,libRack.a -mwindows
TARGET = Rack.exe
endif


all: $(TARGET)


include Makefile.inc