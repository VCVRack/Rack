RACK_DIR ?= .
VERSION = 1.dev.$(shell git rev-parse --short HEAD)

FLAGS += -DVERSION=$(VERSION)
FLAGS += -Iinclude
FLAGS += -Idep/include -Idep/lib/libzip/include

include arch.mk

SED := perl -pi -e

# Sources and build flags

SOURCES += dep/nanovg/src/nanovg.c
SOURCES += dep/osdialog/osdialog.c
SOURCES += $(wildcard dep/jpommier-pffft-*/pffft.c) $(wildcard dep/jpommier-pffft-*/fftpack.c)
SOURCES += $(wildcard src/*.cpp src/*/*.cpp)

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

ifdef ARCH_MAC
	SOURCES += dep/osdialog/osdialog_mac.m
	LDFLAGS += -lpthread -ldl \
		-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework CoreAudio -framework CoreMIDI \
		-Ldep/lib dep/lib/libglfw3.a dep/lib/libGLEW.a dep/lib/libjansson.a dep/lib/libspeexdsp.a dep/lib/libzip.a dep/lib/libz.a dep/lib/librtaudio.a dep/lib/librtmidi.a dep/lib/libcrypto.a dep/lib/libssl.a dep/lib/libcurl.a
	TARGET := Rack
endif

ifdef ARCH_WIN
	SOURCES += dep/osdialog/osdialog_win.c
	LDFLAGS += -Wl,--export-all-symbols,--out-implib,libRack.a -mwindows \
		dep/lib/libglew32.a dep/lib/libglfw3.a dep/lib/libjansson.a dep/lib/libspeexdsp.a dep/lib/libzip.a dep/lib/libz.a dep/lib/libcurl.a dep/lib/libssl.a dep/lib/libcrypto.a dep/lib/librtaudio.a dep/lib/librtmidi.a \
		-lpthread -lopengl32 -lgdi32 -lws2_32 -lcomdlg32 -lole32 -ldsound -lwinmm -lksuser -lshlwapi -lmfplat -lmfuuid -lwmcodecdspuuid -ldbghelp
	TARGET := Rack.exe
	OBJECTS += Rack.res
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
	# Requires gperftools
	perf record --call-graph dwarf -o perf.data ./$< -d
	# Analyze with hotspot (https://github.com/KDAB/hotspot) for example
	# hotspot perf.data

valgrind: $(TARGET)
	# --gen-suppressions=yes
	# --leak-check=full
	valgrind --suppressions=valgrind.supp ./$< -d

clean:
	rm -rfv $(TARGET) libRack.a Rack.res build dist

ifdef ARCH_WIN
# For Windows resources
%.res: %.rc
	windres $^ -O coff -o $@
endif


# This target is not intended for public use
dist: $(TARGET)
	rm -rf dist
	mkdir -p dist

	$(MAKE) -C plugins/Fundamental dist

ifdef ARCH_LIN
	mkdir -p dist/Rack
	cp $(TARGET) dist/Rack/
	$(STRIP) -s dist/Rack/$(TARGET)
	cp -R LICENSE* res template.vcv dist/Rack/
	# Manually check that no nonstandard shared libraries are linked
	ldd dist/Rack/$(TARGET)
	cp plugins/Fundamental/dist/*.zip dist/Rack/Fundamental.zip
	mkdir -p dist/Rack/Bridge
	# cp Bridge/VST/dist/VCV-Bridge{,-fx}.so dist/Rack/Bridge/
	# Make ZIP
	cd dist && zip -5 -r Rack-$(VERSION)-$(ARCH).zip Rack
endif
ifdef ARCH_MAC
	mkdir -p dist/$(TARGET).app
	mkdir -p dist/$(TARGET).app/Contents
	cp Info.plist dist/$(TARGET).app/Contents/
	$(SED) 's/{VERSION}/$(VERSION)/g' dist/$(TARGET).app/Contents/Info.plist
	mkdir -p dist/$(TARGET).app/Contents/MacOS
	cp $(TARGET) dist/$(TARGET).app/Contents/MacOS/
	$(STRIP) -S dist/$(TARGET).app/Contents/MacOS/$(TARGET)
	mkdir -p dist/$(TARGET).app/Contents/Resources
	cp -R LICENSE* res template.vcv icon.icns dist/$(TARGET).app/Contents/Resources

	# Manually check that no nonstandard shared libraries are linked
	otool -L dist/$(TARGET).app/Contents/MacOS/$(TARGET)

	cp plugins/Fundamental/dist/*.zip dist/$(TARGET).app/Contents/Resources/Fundamental.zip
	# cp -R Bridge/AU/dist/VCV-Bridge.component dist/
	# cp -R Bridge/VST/dist/VCV-Bridge.vst dist/
	# cp -R Bridge/VST/dist/VCV-Bridge-fx.vst dist/
	# Make ZIP
	cd dist && zip -5 -r Rack-$(VERSION)-$(ARCH).zip $(TARGET).app
endif
ifdef ARCH_WIN
	mkdir -p dist/Rack
	cp $(TARGET) dist/Rack/
	$(STRIP) -s dist/Rack/$(TARGET)
	cp -R LICENSE* res template.vcv dist/Rack/
	cp /mingw64/bin/libwinpthread-1.dll dist/Rack/
	cp /mingw64/bin/libstdc++-6.dll dist/Rack/
	cp /mingw64/bin/libgcc_s_seh-1.dll dist/Rack/
	cp plugins/Fundamental/dist/*.zip dist/Rack/Fundamental.zip
	# mkdir -p dist/Rack/Bridge
	# cp Bridge/VST/dist/VCV-Bridge-{32,64,fx-32,fx-64}.dll dist/Rack/Bridge/
	# Make ZIP
	cd dist && zip -5 -r Rack-$(VERSION)-$(ARCH).zip Rack
	# Make NSIS installer
	# pacman -S mingw-w64-x86_64-nsis
	# makensis installer.nsi
	# mv Rack-setup.exe dist/Rack-$(VERSION)-$(ARCH).exe
endif

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
	cd dist && zip -5 -r Rack-SDK-$(VERSION).zip Rack-SDK


# Obviously this will only work if you have the private keys to my server
UPLOAD_URL := vortico@vcvrack.com:files/
upload:
ifdef ARCH_MAC
	rsync dist/*.zip $(UPLOAD_URL) -zP
endif
ifdef ARCH_WIN
	rsync dist/*.{exe,zip} $(UPLOAD_URL) -P
endif
ifdef ARCH_LIN
	rsync dist/*.zip $(UPLOAD_URL) -zP
endif


# Plugin helpers

plugins:
	for f in plugins/*; do $(MAKE) -C "$$f"; done

cleanplugins:
	for f in plugins/*; do $(MAKE) -C "$$f" clean; done

distplugins:
	for f in plugins/*; do $(MAKE) -C "$$f" dist; done

cmdplugins:
	for f in plugins/*; do (cd "$$f" && ${CMD}); done


# Includes

include compile.mk

.PHONY: all dep run debug clean dist plugins cleanplugins distplugins cmdplugins
.DEFAULT_GOAL := all
