RACK_DIR ?= ..
.NOTPARALLEL:


# Target paths

include $(RACK_DIR)/arch.mk

ifdef ARCH_LIN
	glew = lib/libGLEW.a
	glfw = lib/libglfw3.a
	jansson = lib/libjansson.a
	libspeexdsp = lib/libspeexdsp.a
	libcurl = lib/libcurl.a
	libzip = lib/libzip.a
	zlib = lib/libz.a
	rtmidi = lib/librtmidi.a
	rtaudio = lib/librtaudio.a
	openssl = lib/libssl.a
endif

ifdef ARCH_MAC
	glew = lib/libGLEW.a
	glfw = lib/libglfw3.a
	jansson = lib/libjansson.a
	libspeexdsp = lib/libspeexdsp.a
	libcurl = lib/libcurl.a
	libzip = lib/libzip.a
	zlib = lib/libz.a
	rtmidi = lib/librtmidi.a
	rtaudio = lib/librtaudio.a
	openssl = lib/libssl.a
endif

ifdef ARCH_WIN
	glew = lib/libglew32.a
	glfw = lib/libglfw3.a
	jansson = lib/libjansson.a
	libspeexdsp = lib/libspeexdsp.a
	libcurl = lib/libcurl.a
	libzip = lib/libzip.a
	zlib = lib/libz.a
	rtmidi = lib/librtmidi.a
	rtaudio = lib/librtaudio.a
	openssl = lib/libssl.a
endif

nanovg = include/nanovg.h
nanosvg = include/nanosvg.h
oui-blendish = include/blendish.h
osdialog = include/osdialog.h
pffft = include/pffft.h

DEPS += $(glew) $(glfw) $(jansson) $(libspeexdsp) $(libcurl) $(libzip) $(rtmidi) $(rtaudio) $(nanovg) $(nanosvg) $(oui-blendish) $(osdialog) $(pffft)
include $(RACK_DIR)/dep.mk


# Targets

$(glew):
	$(WGET) "https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.tgz"
	$(UNTAR) glew-2.1.0.tgz
	cd glew-2.1.0 && mkdir -p build
	cd glew-2.1.0/build && $(CMAKE) -DCMAKE_INSTALL_LIBDIR=lib ./cmake
	$(MAKE) -C glew-2.1.0/build
	$(MAKE) -C glew-2.1.0/build install

$(glfw):
	cd glfw && $(CMAKE) . \
		-DGLFW_COCOA_CHDIR_RESOURCES=OFF -DGLFW_COCOA_MENUBAR=ON -DGLFW_COCOA_RETINA_FRAMEBUFFER=ON
	$(MAKE) -C glfw
	$(MAKE) -C glfw install

$(jansson):
	$(WGET) "http://www.digip.org/jansson/releases/jansson-2.10.tar.gz"
	$(UNTAR) jansson-2.10.tar.gz
	cd jansson-2.10 && $(CONFIGURE)
	$(MAKE) -C jansson-2.10
	$(MAKE) -C jansson-2.10 install

$(libspeexdsp):
	$(WGET) "https://vcvrack.com/downloads/dep/speexdsp-SpeexDSP-1.2rc3.tgz"
	$(UNTAR) speexdsp-SpeexDSP-1.2rc3.tgz
	cd speexdsp-SpeexDSP-1.2rc3 && $(CONFIGURE)
	$(MAKE) -C speexdsp-SpeexDSP-1.2rc3
	$(MAKE) -C speexdsp-SpeexDSP-1.2rc3 install

$(openssl):
	$(WGET) "https://www.openssl.org/source/openssl-1.1.0h.tar.gz"
	$(UNTAR) openssl-1.1.0h.tar.gz
	@# ./config ignores CFLAGS, so hack it in with CC
	cd openssl-1.1.0h && CC="$(CC) $(CFLAGS)" ./config --prefix="$(realpath $(DEP_LOCAL))"
	$(MAKE) -C openssl-1.1.0h
	$(MAKE) -C openssl-1.1.0h install_sw

$(libcurl): $(openssl)
	$(WGET) "https://curl.haxx.se/download/curl-7.59.0.tar.gz"
	$(UNTAR) curl-7.59.0.tar.gz
	cd curl-7.59.0 && PKG_CONFIG_PATH= $(CONFIGURE) \
		--disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual --disable-shared --disable-symbol-hiding \
		--without-zlib --without-libpsl --without-libmetalink --without-libssh2 --without-librtmp --without-winidn --without-libidn2 --without-nghttp2 --without-brotli \
		--without-ca-bundle --with-ca-fallback --with-ssl=$(realpath $(DEP_LOCAL))
	$(MAKE) -C curl-7.59.0
	$(MAKE) -C curl-7.59.0 install

$(libzip): $(zlib)
	$(WGET) "https://nih.at/libzip/libzip-1.2.0.tar.gz"
	$(UNTAR) libzip-1.2.0.tar.gz
	cd libzip-1.2.0 && $(CONFIGURE)
	$(MAKE) -C libzip-1.2.0
	$(MAKE) -C libzip-1.2.0 install

$(zlib):
	$(WGET) "https://www.zlib.net/zlib-1.2.11.tar.gz"
	$(UNTAR) zlib-1.2.11.tar.gz
ifdef ARCH_WIN
	$(MAKE) -C zlib-1.2.11 -f win32/Makefile.gcc
	$(MAKE) -C zlib-1.2.11 -f win32/Makefile.gcc BINARY_PATH="$(realpath $(DEP_LOCAL))/bin" INCLUDE_PATH="$(realpath $(DEP_LOCAL))/include" LIBRARY_PATH="$(realpath $(DEP_LOCAL))/lib" install
else
	cd zlib-1.2.11 && $(CONFIGURE)
	$(MAKE) -C zlib-1.2.11
	$(MAKE) -C zlib-1.2.11 install
endif

$(rtmidi):
	$(WGET) "https://vcvrack.com/downloads/dep/rtmidi.tgz"
	$(UNTAR) rtmidi.tgz
	cd rtmidi && $(CONFIGURE)
	$(MAKE) -C rtmidi
	$(MAKE) -C rtmidi install

ifdef ARCH_MAC
RTAUDIO_FLAGS += -DAUDIO_OSX_CORE=ON
endif
ifdef ARCH_WIN
RTAUDIO_FLAGS += -DAUDIO_WINDOWS_DS=ON -DAUDIO_WINDOWS_WASAPI=ON -DAUDIO_WINDOWS_ASIO=ON
endif
ifdef ARCH_LIN
RTAUDIO_FLAGS += -DAUDIO_LINUX_ALSA=ON -DAUDIO_UNIX_JACK=ON
endif

$(rtaudio):
	cd rtaudio && mkdir -p build
	cd rtaudio/build && $(CMAKE) $(RTAUDIO_FLAGS) ..
	$(MAKE) -C rtaudio/build
	$(MAKE) -C rtaudio/build install
	@# For some reason, it doesn't install the static library
	cp rtaudio/build/librtaudio_static.a lib/librtaudio.a

$(nanovg): $(wildcard nanovg/src/*.h)
	cp $^ include/

$(nanosvg): $(wildcard nanosvg/src/*.h)
	cp $^ include/

$(oui-blendish): $(wildcard oui-blendish/*.h)
	cp $^ include/

$(osdialog): $(wildcard osdialog/*.h)
	cp $^ include/

$(pffft):
	$(WGET) "https://bitbucket.org/jpommier/pffft/get/29e4f76ac53b.zip"
	$(UNZIP) 29e4f76ac53b.zip
	cp jpommier-pffft-29e4f76ac53b/*.h include/

clean:
	git clean -fdx
	git submodule foreach git clean -fdx
