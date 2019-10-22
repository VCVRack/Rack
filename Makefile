RACK_DIR ?= .
VERSION := 1.dev.$(shell git rev-parse --short HEAD)
# VERSION := 1.1.5

FLAGS += -DVERSION=$(VERSION)
FLAGS += -Iinclude -Idep/include

include arch.mk

SED := perl -pi -e

# Sources and build flags

SOURCES += dep/nanovg/src/nanovg.c
SOURCES += dep/osdialog/osdialog.c
SOURCES += $(wildcard dep/jpommier-pffft-*/pffft.c) $(wildcard dep/jpommier-pffft-*/fftpack.c)
SOURCES += $(wildcard src/*.cpp src/*/*.cpp)

ifdef ARCH_LIN
	SOURCES += dep/osdialog/osdialog_gtk2.c
build/dep/osdialog/osdialog_gtk2.c.o: FLAGS += $(shell pkg-config --cflags gtk+-2.0)

	LDFLAGS += -rdynamic \
		dep/lib/libGLEW.a dep/lib/libglfw3.a dep/lib/libjansson.a dep/lib/libcurl.a dep/lib/libssl.a dep/lib/libcrypto.a dep/lib/libzip.a dep/lib/libz.a dep/lib/libspeexdsp.a dep/lib/libsamplerate.a dep/lib/librtmidi.a dep/lib/librtaudio.a \
		-lpthread -lGL -ldl -lX11 -lasound -ljack \
		$(shell pkg-config --libs gtk+-2.0)
	TARGET := Rack
endif

ifdef ARCH_MAC
	SOURCES += dep/osdialog/osdialog_mac.m
	LDFLAGS += -lpthread -ldl \
		-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework CoreAudio -framework CoreMIDI \
		dep/lib/libGLEW.a dep/lib/libglfw3.a dep/lib/libjansson.a dep/lib/libcurl.a dep/lib/libssl.a dep/lib/libcrypto.a dep/lib/libzip.a dep/lib/libz.a dep/lib/libspeexdsp.a dep/lib/libsamplerate.a dep/lib/librtmidi.a dep/lib/librtaudio.a
	# For LuaJIT
	LDFLAGS += -Wl,-pagezero_size,10000 -Wl,-image_base,100000000
	TARGET := Rack
endif

ifdef ARCH_WIN
	SOURCES += dep/osdialog/osdialog_win.c
	LDFLAGS += -Wl,--export-all-symbols,--out-implib,libRack.a -mwindows \
		dep/lib/libglew32.a dep/lib/libglfw3.a dep/lib/libjansson.a dep/lib/libspeexdsp.a dep/lib/libsamplerate.a dep/lib/libzip.a dep/lib/libz.a dep/lib/libcurl.a dep/lib/libssl.a dep/lib/libcrypto.a dep/lib/librtaudio.a dep/lib/librtmidi.a \
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
	lldb -- ./$< -d
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
	hotspot perf.data
	rm perf.data

valgrind: $(TARGET)
	# --gen-suppressions=yes
	# --leak-check=full
	valgrind --suppressions=valgrind.supp ./$< -d

clean:
	rm -rfv $(TARGET) libRack.a Rack.res build dist


# For Windows resources
%.res: %.rc
ifdef ARCH_WIN
	windres $^ -O coff -o $@
endif


DIST_RES := LICENSE* CHANGELOG.md res cacert.pem Core.json template.vcv
DIST_NAME := Rack-$(VERSION)-$(ARCH)
DIST_SDK := Rack-SDK-$(VERSION).zip

# This target is not intended for public use
dist: $(TARGET)
	rm -rf dist
	mkdir -p dist

ifdef ARCH_LIN
	mkdir -p dist/Rack
	cp $(TARGET) dist/Rack/
	$(STRIP) -s dist/Rack/$(TARGET)
	cp -R $(DIST_RES) dist/Rack/
	# Manually check that no nonstandard shared libraries are linked
	ldd dist/Rack/$(TARGET)
	cp Fundamental.zip dist/Rack/
	# Make ZIP
	cd dist && zip -q -9 -r $(DIST_NAME).zip Rack
endif
ifdef ARCH_MAC
	mkdir -p dist/Rack.app
	mkdir -p dist/Rack.app/Contents
	cp Info.plist dist/Rack.app/Contents/
	$(SED) 's/{VERSION}/$(VERSION)/g' dist/Rack.app/Contents/Info.plist
	mkdir -p dist/Rack.app/Contents/MacOS
	cp $(TARGET) dist/Rack.app/Contents/MacOS/
	$(STRIP) -S dist/Rack.app/Contents/MacOS/$(TARGET)
	mkdir -p dist/Rack.app/Contents/Resources
	cp -R $(DIST_RES) icon.icns dist/Rack.app/Contents/Resources/

	# Manually check that no nonstandard shared libraries are linked
	otool -L dist/Rack.app/Contents/MacOS/$(TARGET)

	cp Fundamental.zip dist/Rack.app/Contents/Resources/Fundamental.txt
	# Clean up and sign bundle
	xattr -cr dist/Rack.app
	# This will only work if you have the private key to my certificate
	codesign --verbose --sign "Developer ID Application: Andrew Belt (VRF26934X5)" --options runtime --entitlements Entitlements.plist --deep dist/Rack.app
	codesign --verify --deep --strict --verbose=2 dist/Rack.app
	# Make ZIP
	cd dist && zip -q -9 -r $(DIST_NAME).zip Rack.app
endif
ifdef ARCH_WIN
	mkdir -p dist/Rack
	cp $(TARGET) dist/Rack/
	$(STRIP) -s dist/Rack/$(TARGET)
	cp -R $(DIST_RES) dist/Rack/
	cp /mingw64/bin/libwinpthread-1.dll dist/Rack/
	cp /mingw64/bin/libstdc++-6.dll dist/Rack/
	cp /mingw64/bin/libgcc_s_seh-1.dll dist/Rack/
	cp Fundamental.zip dist/Rack/
	# Make ZIP
	cd dist && zip -q -9 -r $(DIST_NAME).zip Rack
	# Make NSIS installer
	# pacman -S mingw-w64-x86_64-nsis
	makensis -DVERSION=$(VERSION) installer.nsi
	mv installer.exe dist/$(DIST_NAME).exe
endif

	# Rack SDK
	mkdir -p dist/Rack-SDK
	cp -R LICENSE* *.mk include helper.py dist/Rack-SDK/
	mkdir -p dist/Rack-SDK/dep/
	cp -R dep/include dist/Rack-SDK/dep/
ifdef ARCH_WIN
	cp libRack.a dist/Rack-SDK/
endif
	cd dist && zip -q -9 -r $(DIST_SDK) Rack-SDK


notarize:
ifdef ARCH_MAC
	# This will only work if you have my Apple ID password in your keychain
	xcrun altool --notarize-app -f dist/Rack-$(VERSION)-$(ARCH).zip --primary-bundle-id=com.vcvrack.rack -u "andrewpbelt@gmail.com" -p @keychain:notarize --output-format xml > dist/UploadInfo.plist
	# Wait for Apple's servers to approve the app
	while true; do \
		echo "Waiting on Apple servers..." ; \
		xcrun altool --notarization-info `/usr/libexec/PlistBuddy -c "Print :notarization-upload:RequestUUID" dist/UploadInfo.plist` -u "andrewpbelt@gmail.com" -p @keychain:notarize --output-format xml > dist/RequestInfo.plist ; \
		if [ "`/usr/libexec/PlistBuddy -c "Print :notarization-info:Status" dist/RequestInfo.plist`" != "in progress" ]; then \
			break ; \
		fi ; \
		sleep 10 ; \
	done
	# Mark app as notarized, check, and re-zip
	xcrun stapler staple dist/Rack.app
	spctl --assess --type execute --ignore-cache --no-cache -vv dist/Rack.app
	cd dist && zip -q -9 -r $(DIST_NAME).zip Rack.app
endif


UPLOAD_URL := vortico@vcvrack.com:files/
upload:
	# This will only work if you have a private key to my server
ifdef ARCH_MAC
	rsync dist/$(DIST_NAME).zip $(UPLOAD_URL) -zP
endif
ifdef ARCH_WIN
	rsync dist/$(DIST_NAME).zip dist/$(DIST_NAME).exe dist/$(DIST_SDK) $(UPLOAD_URL) -P
endif
ifdef ARCH_LIN
	rsync dist/$(DIST_NAME).zip $(UPLOAD_URL) -zP
endif


# Plugin helpers

plugins:
ifdef CMD
	for f in plugins/*; do (cd "$$f" && $(CMD)); done
else
	for f in plugins/*; do $(MAKE) -C "$$f"; done
endif


# Includes

include compile.mk

.DEFAULT_GOAL := all
.PHONY: all dep run debug clean dist upload src plugins
