RACK_DIR ?= .
EDITION := Free
VERSION_MAJOR := 2
VERSION ?= $(patsubst v%,%,$(shell git describe --tags --match "v$(VERSION_MAJOR).*"))

FLAGS += -Iinclude -Idep/include

include arch.mk

# Sources and build flags

SOURCES += dep/nanovg/src/nanovg.c
SOURCES += dep/osdialog/osdialog.c
SOURCES += dep/oui-blendish/blendish.c
SOURCES += dep/pffft/pffft.c dep/pffft/fftpack.c
SOURCES += dep/tinyexpr/tinyexpr.c
SOURCES += $(wildcard src/*.c src/*/*.c)
SOURCES += $(wildcard src/*.cpp src/*/*.cpp)

build/src/common.cpp.o: FLAGS += -D_APP_VERSION=$(VERSION)
build/dep/tinyexpr/tinyexpr.c.o: FLAGS += -DTE_POW_FROM_RIGHT -DTE_NAT_LOG

FLAGS += -fPIC
LDFLAGS += -shared

ifdef ARCH_LIN
	SED := sed -i
	TARGET := libRack.so

	SOURCES += dep/osdialog/osdialog_zenity.c

	# This prevents static variables in the DSO (dynamic shared object) from being preserved after dlclose().
	# I don't really understand the side effects (see GCC manual), but so far tests are positive.
	FLAGS += -fno-gnu-unique

	LDFLAGS += -Wl,--whole-archive
	LDFLAGS += -static-libstdc++ -static-libgcc
	LDFLAGS += dep/lib/libGLEW.a dep/lib/libglfw3.a dep/lib/libjansson.a dep/lib/libcurl.a dep/lib/libssl.a dep/lib/libcrypto.a dep/lib/libarchive.a dep/lib/libzstd.a dep/lib/libspeexdsp.a dep/lib/libsamplerate.a dep/lib/librtmidi.a dep/lib/librtaudio.a
	LDFLAGS += -Wl,--no-whole-archive
	LDFLAGS += -lpthread -lGL -ldl -lX11 -lasound -ljack -lpulse -lpulse-simple
endif

ifdef ARCH_MAC
	SED := sed -i ''
	TARGET := libRack.dylib

	SOURCES += $(wildcard src/*.m src/*/*.m)
	SOURCES += $(wildcard src/*.mm src/*/*.mm)
	SOURCES += dep/osdialog/osdialog_mac.m
	LDFLAGS += -lpthread -ldl
	LDFLAGS += -framework SystemConfiguration -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework CoreAudio -framework CoreMIDI -framework AVFoundation
	LDFLAGS += -Wl,-all_load
	LDFLAGS += dep/lib/libGLEW.a dep/lib/libglfw3.a dep/lib/libjansson.a dep/lib/libcurl.a dep/lib/libssl.a dep/lib/libcrypto.a -Wl,-load_hidden,dep/lib/libarchive.a -Wl,-load_hidden,dep/lib/libzstd.a dep/lib/libspeexdsp.a dep/lib/libsamplerate.a -Wl,-load_hidden,dep/lib/librtmidi.a -Wl,-load_hidden,dep/lib/librtaudio.a
endif

ifdef ARCH_WIN
	SED := sed -i
	TARGET := libRack.dll

	SOURCES += dep/osdialog/osdialog_win.c
	LDFLAGS += -municode
	LDFLAGS += -Wl,--export-all-symbols
	LDFLAGS += -Wl,--out-implib,$(TARGET).a
	LDFLAGS += -Wl,-Bstatic -Wl,--whole-archive
	LDFLAGS += dep/lib/libglew32.a dep/lib/libglfw3.a dep/lib/libjansson.a dep/lib/libspeexdsp.a dep/lib/libsamplerate.a dep/lib/libarchive.a dep/lib/libzstd.a dep/lib/libcurl.a dep/lib/libssl.a dep/lib/libcrypto.a dep/lib/librtaudio.a dep/lib/librtmidi.a
	LDFLAGS += -Wl,-Bdynamic -Wl,--no-whole-archive
	LDFLAGS += -lpthread -lopengl32 -lgdi32 -lws2_32 -lcomdlg32 -lole32 -ldsound -lwinmm -lksuser -lshlwapi -lmfplat -lmfuuid -lwmcodecdspuuid -ldbghelp -lcrypt32 -lbcrypt
endif

# Some libraries aren't needed by plugins and might conflict with DAWs that load libRack, so make their symbols local to libRack instead of global (default).
# --exclude-libs is unavailable on Apple ld
ifndef ARCH_MAC
	LDFLAGS += -Wl,--exclude-libs,libzstd.a
	LDFLAGS += -Wl,--exclude-libs,libarchive.a
	LDFLAGS += -Wl,--exclude-libs,librtmidi.a
	LDFLAGS += -Wl,--exclude-libs,librtaudio.a
endif

include compile.mk

# Standalone adapter

STANDALONE_SOURCES += adapters/standalone.cpp

ifdef ARCH_LIN
	STANDALONE_TARGET := Rack
	STANDALONE_LDFLAGS += -static-libstdc++ -static-libgcc
	STANDALONE_LDFLAGS += -Wl,-rpath=.
endif
ifdef ARCH_MAC
	STANDALONE_TARGET := Rack
	STANDALONE_LDFLAGS += -stdlib=libc++
endif
ifdef ARCH_WIN
	STANDALONE_TARGET := Rack.exe
	STANDALONE_LDFLAGS += -mwindows
	# 1MiB stack size to match MSVC
	STANDALONE_LDFLAGS += -Wl,--stack,0x100000
	STANDALONE_OBJECTS += build/Rack.res
endif

STANDALONE_OBJECTS += $(TARGET)

$(STANDALONE_TARGET): $(STANDALONE_SOURCES) $(STANDALONE_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(STANDALONE_LDFLAGS)

# Convenience targets

all: $(TARGET) $(STANDALONE_TARGET)

dep:
	$(MAKE) -C dep

cleandep:
	$(MAKE) -C dep clean

run: $(STANDALONE_TARGET)
	./$< -d

runr: $(STANDALONE_TARGET)
	./$<

debug: $(STANDALONE_TARGET)
ifdef ARCH_MAC
	lldb -- ./$< -d
endif
ifdef ARCH_WIN
	gdb --args ./$< -d
endif
ifdef ARCH_LIN
	gdb --args ./$< -d
endif

perf: $(STANDALONE_TARGET)
	# Requires perf
	perf record --call-graph dwarf -o perf.data ./$< -d
	# Analyze with hotspot (https://github.com/KDAB/hotspot) for example
	hotspot perf.data
	rm perf.data

valgrind: $(STANDALONE_TARGET)
	# --gen-suppressions=yes
	# --leak-check=full
	valgrind --suppressions=valgrind.supp ./$< -d

clean:
	rm -rfv build dist $(TARGET) $(STANDALONE_TARGET) *.a

# Windows resources
build/%.res: %.rc
ifdef ARCH_WIN
	windres $^ -O coff -o $@
endif


# Plugin helper
plugins:
ifdef CMD
	for f in plugins/*; do (cd "$$f" && $(CMD)); done
else
	for f in plugins/*; do $(MAKE) -C "$$f"; done
endif


# The following targets are not supported for public use

DIST_NAME = Rack$(EDITION)-$(VERSION)-$(ARCH_NAME)
ifdef ARCH_MAC
	DIST_BUNDLE := VCV Rack $(VERSION_MAJOR) $(EDITION).app
else
	DIST_DIR := Rack$(VERSION_MAJOR)$(EDITION)
endif
DIST_MD := $(wildcard *.md)
DIST_HTML := $(patsubst %.md, build/%.html, $(DIST_MD))
DIST_RES := res cacert.pem Core.json template.vcv LICENSE-GPLv3.txt $(DIST_HTML) translations
DIST_SDK_DIR := Rack-SDK
DIST_SDK = Rack-SDK-$(VERSION)-$(ARCH_NAME).zip
FUNDAMENTAL_VERSION ?= 2.6.1
FUNDAMENTAL_FILENAME := Fundamental-$(FUNDAMENTAL_VERSION)-$(ARCH_NAME).vcvplugin


dist: $(TARGET) $(STANDALONE_TARGET) $(DIST_HTML)
	mkdir -p dist
	# Download Fundamental package if not already downloaded
	[ -f "$(FUNDAMENTAL_FILENAME)" ] || curl -o "$(FUNDAMENTAL_FILENAME)" "https://api.vcvrack.com/download?slug=Fundamental&version=$(FUNDAMENTAL_VERSION)&arch=$(ARCH_NAME)"
ifdef ARCH_LIN
	mkdir -p dist/"$(DIST_DIR)"
	cp $(TARGET) dist/"$(DIST_DIR)"/
	cp $(STANDALONE_TARGET) dist/"$(DIST_DIR)"/
	$(STRIP) -s dist/"$(DIST_DIR)"/$(TARGET)
	$(STRIP) -s dist/"$(DIST_DIR)"/$(STANDALONE_TARGET)
	# Manually check that no nonstandard shared libraries are linked
	ldd dist/"$(DIST_DIR)"/$(TARGET)
	ldd dist/"$(DIST_DIR)"/$(STANDALONE_TARGET)
	# Copy resources
	cp -R $(DIST_RES) dist/"$(DIST_DIR)"/
	cp "$(FUNDAMENTAL_FILENAME)" dist/"$(DIST_DIR)"/
endif
ifdef ARCH_MAC
	mkdir -p dist/"$(DIST_BUNDLE)"
	mkdir -p dist/"$(DIST_BUNDLE)"/Contents
	mkdir -p dist/"$(DIST_BUNDLE)"/Contents/Resources
	mkdir -p dist/"$(DIST_BUNDLE)"/Contents/MacOS
	cp $(TARGET) dist/"$(DIST_BUNDLE)"/Contents/Resources/
	cp $(STANDALONE_TARGET) dist/"$(DIST_BUNDLE)"/Contents/MacOS/
	$(STRIP) -S dist/"$(DIST_BUNDLE)"/Contents/Resources/$(TARGET)
	$(STRIP) -S dist/"$(DIST_BUNDLE)"/Contents/MacOS/$(STANDALONE_TARGET)
	install_name_tool -change $(TARGET) @executable_path/../Resources/$(TARGET) dist/"$(DIST_BUNDLE)"/Contents/MacOS/$(STANDALONE_TARGET)
	# Manually check that no nonstandard shared libraries are linked
	otool -L dist/"$(DIST_BUNDLE)"/Contents/Resources/$(TARGET)
	otool -L dist/"$(DIST_BUNDLE)"/Contents/MacOS/$(STANDALONE_TARGET)
	# Copy resources
	cp Info.plist dist/"$(DIST_BUNDLE)"/Contents/
	$(SED) 's/{VERSION}/$(VERSION)/g' dist/"$(DIST_BUNDLE)"/Contents/Info.plist
	cp -R icon.icns dist/"$(DIST_BUNDLE)"/Contents/Resources/
	cp -R $(DIST_RES) dist/"$(DIST_BUNDLE)"/Contents/Resources/
	cp "$(FUNDAMENTAL_FILENAME)" dist/"$(DIST_BUNDLE)"/Contents/Resources/
endif
ifdef ARCH_WIN
	mkdir -p dist/"$(DIST_DIR)"
	cp $(TARGET) dist/"$(DIST_DIR)"/
	cp $(STANDALONE_TARGET) dist/"$(DIST_DIR)"/
	$(STRIP) -s dist/"$(DIST_DIR)"/$(TARGET)
	$(STRIP) -s dist/"$(DIST_DIR)"/$(STANDALONE_TARGET)
	# Copy resources
	cp -R $(DIST_RES) dist/"$(DIST_DIR)"/
	cp /mingw64/bin/libwinpthread-1.dll dist/"$(DIST_DIR)"/
	cp /mingw64/bin/libstdc++-6.dll dist/"$(DIST_DIR)"/
	cp /mingw64/bin/libgcc_s_seh-1.dll dist/"$(DIST_DIR)"/
	cp "$(FUNDAMENTAL_FILENAME)" dist/"$(DIST_DIR)"/
endif


sdk: $(DIST_HTML)
	mkdir -p dist/$(DIST_SDK_DIR)
	cp -R include *.mk helper.py $(DIST_HTML) dist/$(DIST_SDK_DIR)/
	mkdir -p dist/$(DIST_SDK_DIR)/dep
	cp -R dep/include dist/$(DIST_SDK_DIR)/dep/
ifdef ARCH_LIN
	cp $(TARGET) dist/$(DIST_SDK_DIR)/
	$(STRIP) -s dist/$(DIST_SDK_DIR)/$(TARGET)
endif
ifdef ARCH_MAC
	cp $(TARGET) dist/$(DIST_SDK_DIR)/
	$(STRIP) -S dist/$(DIST_SDK_DIR)/$(TARGET)
endif
ifdef ARCH_WIN
	cp $(TARGET).a dist/$(DIST_SDK_DIR)/
endif
	# SDK
	cd dist && zip -q -9 -r $(DIST_SDK) $(DIST_SDK_DIR)


package:
ifdef ARCH_LIN
	# Make ZIP
	cd dist && zip -q -9 -r $(DIST_NAME).zip "$(DIST_DIR)"
endif
ifdef ARCH_MAC
	# Clean up and sign bundle
	xattr -cr dist/"$(DIST_BUNDLE)"
	codesign --verbose --sign "Developer ID Application: Andrew Belt (V8SW9J626X)" --options runtime --entitlements Entitlements.plist --timestamp --deep dist/"$(DIST_BUNDLE)"/Contents/Resources/$(TARGET) dist/"$(DIST_BUNDLE)"
	codesign --verify --deep --strict --verbose=2 dist/"$(DIST_BUNDLE)"
	# Make standalone PKG
	mkdir -p dist/Component
	cp -R dist/"$(DIST_BUNDLE)" dist/Component/
	pkgbuild --identifier com.vcvrack.rack2 --component-plist Component.plist --root dist/Component --install-location /Applications dist/Component.pkg
	# Make PKG
	productbuild --distribution Distribution.xml --package-path dist dist/$(DIST_NAME).pkg
	productsign --sign "Developer ID Installer: Andrew Belt (V8SW9J626X)" dist/$(DIST_NAME).pkg dist/$(DIST_NAME)-signed.pkg
	mv dist/$(DIST_NAME)-signed.pkg dist/$(DIST_NAME).pkg
endif
ifdef ARCH_WIN
	# Make NSIS installer
	# pacman -S mingw-w64-x86_64-nsis
	makensis -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION=$(VERSION) "-XOutFile dist/$(DIST_NAME).exe" installer.nsi
endif


lipo:
ifndef OTHER_RACK_DIR
	$(error OTHER_RACK_DIR not defined)
endif
ifdef ARCH_MAC
	# App bundle
	lipo -create -output dist/"$(DIST_BUNDLE)"/Contents/Resources/$(TARGET) dist/"$(DIST_BUNDLE)"/Contents/Resources/$(TARGET) $(OTHER_RACK_DIR)/dist/"$(DIST_BUNDLE)"/Contents/Resources/$(TARGET)
	lipo -create -output dist/"$(DIST_BUNDLE)"/Contents/MacOS/$(STANDALONE_TARGET) dist/"$(DIST_BUNDLE)"/Contents/MacOS/$(STANDALONE_TARGET) $(OTHER_RACK_DIR)/dist/"$(DIST_BUNDLE)"/Contents/MacOS/$(STANDALONE_TARGET)
	# Fundamental package
	cp $(OTHER_RACK_DIR)/dist/"$(DIST_BUNDLE)"/Contents/Resources/Fundamental-*.vcvplugin dist/"$(DIST_BUNDLE)"/Contents/Resources/
endif


notarize:
ifdef ARCH_MAC
	# Submit installer package to Apple
	xcrun notarytool submit --keychain-profile "VCV" --wait dist/$(DIST_NAME).pkg
	# Mark app as notarized
	xcrun stapler staple dist/$(DIST_NAME).pkg
	# Check notarization
	stapler validate dist/$(DIST_NAME).pkg
endif


install: uninstall
ifdef ARCH_MAC
	sudo installer -pkg dist/$(DIST_NAME).pkg -target /
endif


uninstall:
ifdef ARCH_MAC
	sudo rm -rf /Applications/"$(DIST_BUNDLE)"
endif


cleandist:
	rm -rfv dist


.DEFAULT_GOAL := all
.PHONY: all dep run debug clean plugins dist sdk package lipo notarize
