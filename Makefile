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
	BUNDLE = $(TARGET).app
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
	rm -rf $(TARGET) build dist

# For Windows resources
%.res: %.rc
	windres $^ -O coff -o $@

include compile.mk


dist: all
ifndef VERSION
	$(error VERSION is not set.)
endif
	mkdir -p dist/Rack
	mkdir -p dist/Rack/plugins
	cp -R LICENSE* res dist/Rack/
ifeq ($(ARCH), lin)
	cp Rack Rack.sh dist/Rack/
	cp dep/lib/libsamplerate.so.0 dist/Rack/
	cp dep/lib/libjansson.so.4 dist/Rack/
	cp dep/lib/libGLEW.so.2.1 dist/Rack/
	cp dep/lib/libglfw.so.3 dist/Rack/
	cp dep/lib/libcurl.so.4 dist/Rack/
	cp dep/lib/libzip.so.5 dist/Rack/
	cp dep/lib/libportaudio.so.2 dist/Rack/
	cp dep/lib/libportmidi.so dist/Rack/
	cp dep/lib/libsamplerate.so.0 dist/Rack/
	cp dep/lib/libsamplerate.so.0 dist/Rack/
endif
ifeq ($(ARCH), mac)
	mkdir -p $(BUNDLE)/Contents
	cp Info.plist $(BUNDLE)/Contents/

	mkdir -p $(BUNDLE)/Contents/MacOS
	cp Rack $(BUNDLE)/Contents/MacOS/

	otool -L $(BUNDLE)/Contents/MacOS/Rack

	# cp /usr/local/opt/glew/lib/libGLEW.2.0.0.dylib $(BUNDLE)/Contents/MacOS/
	# cp /usr/local/opt/jansson/lib/libjansson.4.dylib $(BUNDLE)/Contents/MacOS/
	# cp /usr/local/opt/portaudio/lib/libportaudio.2.dylib $(BUNDLE)/Contents/MacOS/
	# cp /usr/local/opt/portmidi/lib/libportmidi.dylib $(BUNDLE)/Contents/MacOS/
	# cp /usr/local/opt/libsamplerate/lib/libsamplerate.0.dylib $(BUNDLE)/Contents/MacOS/
	# cp /usr/local/opt/libzip/lib/libzip.4.dylib $(BUNDLE)/Contents/MacOS/

	# install_name_tool -change /usr/local/opt/glew/lib/libGLEW.2.0.0.dylib @executable_path/libGLEW.2.0.0.dylib $(BUNDLE)/Contents/MacOS/Rack
	# install_name_tool -change /usr/local/opt/jansson/lib/libjansson.4.dylib @executable_path/libjansson.4.dylib $(BUNDLE)/Contents/MacOS/Rack
	# install_name_tool -change /usr/local/opt/portaudio/lib/libportaudio.2.dylib @executable_path/libportaudio.2.dylib $(BUNDLE)/Contents/MacOS/Rack
	# install_name_tool -change /usr/local/opt/portmidi/lib/libportmidi.dylib @executable_path/libportmidi.dylib $(BUNDLE)/Contents/MacOS/Rack
	# install_name_tool -change /usr/local/opt/libsamplerate/lib/libsamplerate.0.dylib @executable_path/libsamplerate.0.dylib $(BUNDLE)/Contents/MacOS/Rack
	# install_name_tool -change /usr/local/opt/libzip/lib/libzip.4.dylib @executable_path/libzip.4.dylib $(BUNDLE)/Contents/MacOS/Rack

	otool -L $(BUNDLE)/Contents/MacOS/Rack

	cp -R Rack.app dist/Rack/
endif
ifeq ($(ARCH), win)
	# TODO Copy dlls
	cp Rack/*.dll dist/Rack/
	cp Rack/Rack.exe dist/Rack/
endif

	# Fundamental
	$(MAKE) -C plugins/Fundamental dist
	cp -R plugins/Fundamental/dist/Fundamental dist/Rack/plugins/

	cd dist && zip -5 -r Rack-$(VERSION)-$(ARCH).zip Rack
