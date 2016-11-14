ARCH ?= linux
CFLAGS = -MMD -g -Wall -O0 \
	-DNOC_FILE_DIALOG_IMPLEMENTATION
CXXFLAGS = -MMD -g -Wall -std=c++11 -O0 -ffast-math \
	-I./lib -I./src
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
LDFLAGS = -lpthread -lGL -lGLEW -lglfw -ldl -lrtaudio -lrtmidi -lprofiler -ljansson \
	$(shell pkg-config --libs gtk+-2.0)
TARGET = 5V
endif

# Apple
ifeq ($(ARCH), apple)
OSXCROSS = $(HOME)/pkg/osxcross/target/bin
CC = $(OSXCROSS)/x86_64-apple-darwin15-cc
CXX = $(OSXCROSS)/x86_64-apple-darwin15-c++
SOURCES += lib/noc/noc_file_dialog.m
CFLAGS += -DNOC_FILE_DIALOG_OSX
CXXFLAGS += -DAPPLE -stdlib=libc++
TARGET = 5V
endif

# Windows
ifeq ($(ARCH), windows)
CC = x86_64-w64-mingw32-gcc
CXX = x86_64-w64-mingw32-g++
SOURCES += lib/noc/noc_file_dialog.c
CFLAGS += -DNOC_FILE_DIALOG_WIN32
CXXFLAGS += -DWINDOWS
LDFLAGS = -lpthread -ljansson \
	./lib/nanovg/build/libnanovg.a -lglfw3 -lgdi32 -lopengl32 -lglew32 \
	./lib/rtaudio/librtaudio.a -lksuser -luuid \
	./lib/rtmidi/librtmidi.a -lwinmm \
	-lcomdlg32 -lole32 \
	-Wl,--export-all-symbols,--out-implib,lib5V.a -mwindows
TARGET = 5V.exe
endif


OBJECTS = $(patsubst %, build/%.o, $(SOURCES))
DEPS = $(patsubst %, build/%.d, $(SOURCES))


# Final targets

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Object targets

-include $(DEPS)

build/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Utilities

clean:
	rm -rf build
