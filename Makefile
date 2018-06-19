RACK_DIR ?= .
VERSION = 0.6.1

FLAGS += \
	-Iinclude \
	-Idep/include -Idep/lib/libzip/include

include arch.mk

STRIP ?= strip

# Sources and build flags

SOURCES += dep/nanovg/src/nanovg.c
SOURCES += dep/osdialog/osdialog.c
SOURCES += $(wildcard dep/jpommier-pffft-*/pffft.c) $(wildcard dep/jpommier-pffft-*/fftpack.c)
SOURCES += $(wildcard src/*.cpp src/*/*.cpp)

ifdef ARCH_MAC
	SOURCES += dep/osdialog/osdialog_mac.m
	CXXFLAGS += -stdlib=libc++
	LDFLAGS += -stdlib=libc++ -lpthread -ldl \
		-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework CoreAudio -framework CoreMIDI \
		-Ldep/lib dep/lib/libglfw3.a dep/lib/libGLEW.a dep/lib/libjansson.a dep/lib/libspeexdsp.a dep/lib/libzip.a dep/lib/libz.a dep/lib/librtaudio.a dep/lib/librtmidi.a dep/lib/libcrypto.a dep/lib/libssl.a dep/lib/libcurl.a
	TARGET := Rack
	BUNDLE := dist/$(TARGET).app
endif

ifdef ARCH_WIN
	SOURCES += dep/osdialog/osdialog_win.c
	LDFLAGS += -static \
		-Wl,--export-all-symbols,--out-implib,libRack.a -mwindows \
		-Ldep/lib -lglew32 -lglfw3 -ljansson -lspeexdsp -lzip -lz -lcurl -lssl -lcrypto -lrtaudio -lrtmidi \
		-lpthread -lopengl32 -lgdi32 -lws2_32 -lcomdlg32 -lole32 -ldsound -lwinmm -lksuser -lshlwapi
	TARGET := Rack.exe
	OBJECTS += Rack.res
endif

ifdef ARCH_LIN
	SOURCES += dep/osdialog/osdialog_gtk2.c
	CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
	LDFLAGS += -rdynamic \
		-Ldep/lib \
		-Wl,-Bstatic -lglfw3 -lGLEW -ljansson -lspeexdsp -lzip -lz -lrtmidi -lrtaudio -lcurl -lssl -lcrypto \
		-Wl,-Bdynamic -lpthread -lGL -ldl -lX11 -lasound -ljack \
		$(shell pkg-config --libs gtk+-2.0)
	TARGET := Rack
endif


# Convenience targets

all: $(TARGET)

dep:
	$(MAKE) -C dep

run: $(TARGET)
	./$< -d

runr: $(TARGET)
	./$<

debug: $(TARGET)
ifdef ARCH_MAC
	lldb --args ./$< -d
endif
ifdef ARCH_WIN
	gdb --args ./$< -d
endif
ifdef ARCH_LIN
	gdb --args ./$< -d
endif

perf: $(TARGET)
ifdef ARCH_LIN
	perf record --call-graph dwarf ./$< -d
endif

clean:
	rm -rfv $(TARGET) libRack.a Rack.res build dist

ifdef ARCH_WIN
# For Windows resources
%.res: %.rc
	windres $^ -O coff -o $@
endif


# This target is not intended for public use
dist: all
	rm -rf dist
	mkdir -p dist

	# Rack SDK
	mkdir -p dist/Rack-SDK
	cp LICENSE* dist/Rack-SDK/
	cp *.mk dist/Rack-SDK/
	cp -R include dist/Rack-SDK/
	mkdir -p dist/Rack-SDK/dep/
	cp -R dep/include dist/Rack-SDK/dep/
ifdef ARCH_WIN
	cp libRack.a dist/Rack-SDK/
endif
	cd dist && zip -5 -r Rack-SDK-$(VERSION)-$(ARCH).zip Rack-SDK

	# Rack
	$(MAKE) -C plugins/Fundamental dist

ifdef ARCH_MAC
	mkdir -p $(BUNDLE)
	mkdir -p $(BUNDLE)/Contents
	mkdir -p $(BUNDLE)/Contents/Resources
	cp Info.plist $(BUNDLE)/Contents/
	cp -R LICENSE* icon.icns res $(BUNDLE)/Contents/Resources

	mkdir -p $(BUNDLE)/Contents/MacOS
	cp $(TARGET) $(BUNDLE)/Contents/MacOS/
	$(STRIP) -S $(BUNDLE)/Contents/MacOS/$(TARGET)

	otool -L $(BUNDLE)/Contents/MacOS/$(TARGET)

	cp plugins/Fundamental/dist/Fundamental-*.zip $(BUNDLE)/Contents/Resources/Fundamental.zip
	cp -R Bridge/AU/dist/VCV-Bridge.component dist/
	cp -R Bridge/VST/dist/VCV-Bridge.vst dist/
	# Make DMG image
	cd dist && ln -s /Applications Applications
	cd dist && ln -s /Library/Audio/Plug-Ins/Components Components
	cd dist && ln -s /Library/Audio/Plug-Ins/VST VST
	cd dist && hdiutil create -srcfolder . -volname Rack -ov -format UDZO Rack-$(VERSION)-$(ARCH).dmg
endif
ifdef ARCH_WIN
	mkdir -p dist/Rack
	mkdir -p dist/Rack/Bridge
	cp Bridge/VST/dist/VCV-Bridge-64.dll dist/Rack/Bridge/
	cp Bridge/VST/dist/VCV-Bridge-32.dll dist/Rack/Bridge/
	cp -R LICENSE* res dist/Rack/
	cp $(TARGET) dist/Rack/
	$(STRIP) -s dist/Rack/$(TARGET)
	cp /mingw64/bin/libwinpthread-1.dll dist/Rack/
	cp /mingw64/bin/libstdc++-6.dll dist/Rack/
	cp /mingw64/bin/libgcc_s_seh-1.dll dist/Rack/
	cp plugins/Fundamental/dist/Fundamental-*.zip dist/Rack/Fundamental.zip
	# Make ZIP
	cd dist && zip -5 -r Rack-$(VERSION)-$(ARCH).zip Rack
	# Make NSIS installer
	makensis installer.nsi
	mv Rack-setup.exe dist/Rack-$(VERSION)-$(ARCH).exe
endif
ifdef ARCH_LIN
	mkdir -p dist/Rack
	mkdir -p dist/Rack/Bridge
	cp Bridge/VST/dist/VCV-Bridge.so dist/Rack/Bridge/
	cp -R LICENSE* res dist/Rack/
	cp $(TARGET) dist/Rack/
	$(STRIP) -s dist/Rack/$(TARGET)
	ldd dist/Rack/$(TARGET)
	cp plugins/Fundamental/dist/Fundamental-*.zip dist/Rack/Fundamental.zip
	# Make ZIP
	cd dist && zip -5 -r Rack-$(VERSION)-$(ARCH).zip Rack
endif


# Obviously this will only work if you have the private keys to my server
UPLOAD_URL := vortico@vcvrack.com:files/
upload:
ifdef ARCH_MAC
	rsync dist/*.{dmg,zip} $(UPLOAD_URL) -zP
endif
ifdef ARCH_WIN
	rsync dist/*.{exe,zip} $(UPLOAD_URL) -P
endif
ifdef ARCH_LIN
	rsync dist/*.zip $(UPLOAD_URL) -zP
endif


# Plugin helpers

allplugins:
	for f in plugins/*; do $(MAKE) -C "$$f"; done

cleanplugins:
	for f in plugins/*; do $(MAKE) -C "$$f" clean; done

distplugins:
	for f in plugins/*; do $(MAKE) -C "$$f" dist; done

plugins:
	for f in plugins/*; do (cd "$$f" && ${CMD}); done


# Includes

include compile.mk

.PHONY: all dep run debug clean dist allplugins cleanplugins distplugins plugins
.DEFAULT_GOAL := all
