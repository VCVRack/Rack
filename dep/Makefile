RACK_DIR ?= ..
.NOTPARALLEL:


# Target paths

include $(RACK_DIR)/arch.mk

ifdef ARCH_LIN
	glew = lib/libGLEW.a
	glfw = lib/libglfw3.a
	jansson = lib/libjansson.a
	openssl = lib/libssl.a
	libcurl = lib/libcurl.a
	libzip = lib/libzip.a
	zlib = lib/libz.a
	libspeexdsp = lib/libspeexdsp.a
	libsamplerate = lib/libsamplerate.a
	rtmidi = lib/librtmidi.a
	rtaudio = lib/librtaudio.a
endif

ifdef ARCH_MAC
	glew = lib/libGLEW.a
	glfw = lib/libglfw3.a
	jansson = lib/libjansson.a
	openssl = lib/libssl.a
	libcurl = lib/libcurl.a
	libzip = lib/libzip.a
	zlib = lib/libz.a
	libspeexdsp = lib/libspeexdsp.a
	libsamplerate = lib/libsamplerate.a
	rtmidi = lib/librtmidi.a
	rtaudio = lib/librtaudio.a
endif

ifdef ARCH_WIN
	glew = lib/libglew32.a
	glfw = lib/libglfw3.a
	jansson = lib/libjansson.a
	openssl = lib/libssl.a
	libcurl = lib/libcurl.a
	libzip = lib/libzip.a
	zlib = lib/libz.a
	libspeexdsp = lib/libspeexdsp.a
	libsamplerate = lib/libsamplerate.a
	rtmidi = lib/librtmidi.a
	rtaudio = lib/librtaudio.a
endif

nanovg = include/nanovg.h
nanosvg = include/nanosvg.h
oui-blendish = include/blendish.h
osdialog = include/osdialog.h
pffft = include/pffft.h

DEPS += $(glew)
DEPS += $(glfw)
DEPS += $(jansson)
DEPS += $(libcurl)
DEPS += $(libzip)
DEPS += $(libspeexdsp)
DEPS += $(libsamplerate)
DEPS += $(rtmidi)
DEPS += $(rtaudio)
DEPS += $(nanovg)
DEPS += $(nanosvg)
DEPS += $(oui-blendish)
DEPS += $(osdialog)
DEPS += $(pffft)


DEP_LOCAL := .
include $(RACK_DIR)/dep.mk


# Targets

glew-2.1.0:
	$(WGET) "https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.tgz"
	$(SHA256) glew-2.1.0.tgz 04de91e7e6763039bc11940095cd9c7f880baba82196a7765f727ac05a993c95
	$(UNTAR) glew-2.1.0.tgz
	rm glew-2.1.0.tgz

$(glew): glew-2.1.0
	cd glew-2.1.0 && mkdir -p build
	cd glew-2.1.0/build && $(CMAKE) ./cmake
	$(MAKE) -C glew-2.1.0/build
	$(MAKE) -C glew-2.1.0/build install

$(glfw): glfw
	cd glfw && mkdir -p build
	cd glfw/build && $(CMAKE) .. \
		-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	$(MAKE) -C glfw/build
	$(MAKE) -C glfw/build install

jansson-2.12:
	$(WGET) "http://www.digip.org/jansson/releases/jansson-2.12.tar.gz"
	$(SHA256) jansson-2.12.tar.gz 5f8dec765048efac5d919aded51b26a32a05397ea207aa769ff6b53c7027d2c9
	$(UNTAR) jansson-2.12.tar.gz
	rm jansson-2.12.tar.gz

$(jansson): jansson-2.12
	cd jansson-2.12 && $(CONFIGURE)
	$(MAKE) -C jansson-2.12
	$(MAKE) -C jansson-2.12 install

openssl-1.1.1d:
	$(WGET) "https://www.openssl.org/source/openssl-1.1.1d.tar.gz"
	$(SHA256) openssl-1.1.1d.tar.gz 1e3a91bc1f9dfce01af26026f856e064eab4c8ee0a8f457b5ae30b40b8b711f2
	$(UNTAR) openssl-1.1.1d.tar.gz
	rm openssl-1.1.1d.tar.gz

$(openssl): openssl-1.1.1d
	@# ./config ignores CFLAGS, so hack it in with CC
	cd openssl-1.1.1d && CC="$(CC) $(CFLAGS)" ./config --prefix="$(DEP_PATH)"
	$(MAKE) -C openssl-1.1.1d
	$(MAKE) -C openssl-1.1.1d install_sw

curl-7.66.0:
	$(WGET) "https://curl.haxx.se/download/curl-7.66.0.tar.gz"
	$(SHA256) curl-7.66.0.tar.gz d0393da38ac74ffac67313072d7fe75b1fa1010eb5987f63f349b024a36b7ffb
	$(UNTAR) curl-7.66.0.tar.gz
	rm curl-7.66.0.tar.gz

CURL_FLAGS += --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --enable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual --disable-shared --disable-symbol-hiding
CURL_FLAGS += --without-zlib --without-libpsl --without-libmetalink --without-libssh2 --without-librtmp --without-winidn --without-libidn2 --without-nghttp2 --without-brotli
CURL_FLAGS += --with-ssl="$(DEP_PATH)"

$(libcurl): $(openssl) curl-7.66.0
	cd curl-7.66.0 && PKG_CONFIG_PATH= $(CONFIGURE) $(CURL_FLAGS)
	$(MAKE) -C curl-7.66.0
	$(MAKE) -C curl-7.66.0 install

libzip-1.5.2:
	$(WGET) "https://libzip.org/download/libzip-1.5.2.tar.gz"
	$(SHA256) libzip-1.5.2.tar.gz be694a4abb2ffe5ec02074146757c8b56084dbcebf329123c84b205417435e15
	$(UNTAR) libzip-1.5.2.tar.gz
	rm libzip-1.5.2.tar.gz

$(libzip): $(zlib) libzip-1.5.2
	cd libzip-1.5.2 && mkdir -p build
	cd libzip-1.5.2/build && $(CMAKE) .. -DCMAKE_FIND_ROOT_PATH="$(DEP_PATH)" -DENABLE_COMMONCRYPTO=OFF -DENABLE_GNUTLS=OFF -DENABLE_MBEDTLS=OFF -DENABLE_OPENSSL=OFF -DENABLE_WINDOWS_CRYPTO=OFF -DENABLE_BZIP2=OFF -DBUILD_TOOLS=OFF -DBUILD_REGRESS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOC=OFF -DBUILD_SHARED_LIBS=OFF
	$(MAKE) -C libzip-1.5.2/build
	$(MAKE) -C libzip-1.5.2/build install

zlib-1.2.11:
	$(WGET) "https://www.zlib.net/zlib-1.2.11.tar.gz"
	$(SHA256) zlib-1.2.11.tar.gz c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1
	$(UNTAR) zlib-1.2.11.tar.gz
	rm zlib-1.2.11.tar.gz

$(zlib): zlib-1.2.11
ifdef ARCH_WIN
	$(MAKE) -C zlib-1.2.11 -f win32/Makefile.gcc
	$(MAKE) -C zlib-1.2.11 -f win32/Makefile.gcc BINARY_PATH="$(DEP_PATH)/bin" INCLUDE_PATH="$(DEP_PATH)/include" LIBRARY_PATH="$(DEP_PATH)/lib" install
else
	# Don't use $(CONFIGURE) because this is a handwritten configure script
	cd zlib-1.2.11 && ./configure --prefix="$(DEP_PATH)"
	$(MAKE) -C zlib-1.2.11
	$(MAKE) -C zlib-1.2.11 install
endif

speexdsp-SpeexDSP-1.2rc3:
	$(WGET) "https://vcvrack.com/downloads/dep/speexdsp-SpeexDSP-1.2rc3.tgz"
	$(SHA256) speexdsp-SpeexDSP-1.2rc3.tgz c8dded1454747f65956f981c95e7f89a06abdaa2a53e8aeaa66bab2a3d59cebd
	$(UNTAR) speexdsp-SpeexDSP-1.2rc3.tgz
	rm speexdsp-SpeexDSP-1.2rc3.tgz

$(libspeexdsp): speexdsp-SpeexDSP-1.2rc3
	cd speexdsp-SpeexDSP-1.2rc3 && $(CONFIGURE)
	$(MAKE) -C speexdsp-SpeexDSP-1.2rc3
	$(MAKE) -C speexdsp-SpeexDSP-1.2rc3 install

libsamplerate-0.1.9:
	$(WGET) "http://www.mega-nerd.com/SRC/libsamplerate-0.1.9.tar.gz"
	$(SHA256) libsamplerate-0.1.9.tar.gz 0a7eb168e2f21353fb6d84da152e4512126f7dc48ccb0be80578c565413444c1
	$(UNTAR) libsamplerate-0.1.9.tar.gz
	rm libsamplerate-0.1.9.tar.gz

$(libsamplerate): libsamplerate-0.1.9
	cd libsamplerate-0.1.9 && $(CONFIGURE) --disable-fftw --disable-sndfile
	$(MAKE) -C libsamplerate-0.1.9
	$(MAKE) -C libsamplerate-0.1.9 install

rtmidi-4.0.0:
	$(WGET) "http://www.music.mcgill.ca/~gary/rtmidi/release/rtmidi-4.0.0.tar.gz"
	$(SHA256) rtmidi-4.0.0.tar.gz 370cfe710f43fbeba8d2b8c8bc310f314338c519c2cf2865e2d2737b251526cd
	$(UNTAR) rtmidi-4.0.0.tar.gz
	rm rtmidi-4.0.0.tar.gz

RTMIDI_FLAGS += --enable-shared=no
ifdef ARCH_LIN
RTMIDI_FLAGS += --with-jack --with-alsa
endif
ifdef ARCH_MAC
RTMIDI_FLAGS += --with-core --without-jack
endif
ifdef ARCH_WIN
RTMIDI_FLAGS += --with-winmm
endif

$(rtmidi): rtmidi-4.0.0
	cd rtmidi-4.0.0 && $(CONFIGURE) $(RTMIDI_FLAGS)
	$(MAKE) -C rtmidi-4.0.0
	$(MAKE) -C rtmidi-4.0.0 install

RTAUDIO_FLAGS += -DRTAUDIO_BUILD_STATIC_LIBS=ON
ifdef ARCH_LIN
RTAUDIO_FLAGS += -DRTAUDIO_API_ALSA=ON -DRTAUDIO_API_JACK=ON -DRTAUDIO_API_PULSE=OFF -DRTAUDIO_API_OSS=OFF
endif
ifdef ARCH_MAC
RTAUDIO_FLAGS += -DRTAUDIO_API_CORE=ON -DRTAUDIO_API_PULSE=OFF -DRTAUDIO_API_JACK=OFF
endif
ifdef ARCH_WIN
RTAUDIO_FLAGS += -DRTAUDIO_API_DS=ON -DRTAUDIO_API_WASAPI=ON -DRTAUDIO_API_ASIO=ON
endif

$(rtaudio): rtaudio
	cd rtaudio && mkdir -p build
	cd rtaudio/build && $(CMAKE) $(RTAUDIO_FLAGS) ..
	$(MAKE) -C rtaudio/build
	$(MAKE) -C rtaudio/build install

$(nanovg): $(wildcard nanovg/src/*.h) nanovg/example/stb_image_write.h
	cp $^ include/

$(nanosvg): $(wildcard nanosvg/src/*.h)
	cp $^ include/

$(oui-blendish): $(wildcard oui-blendish/*.h)
	cp $^ include/

$(osdialog): $(wildcard osdialog/*.h)
	cp $^ include/

jpommier-pffft-29e4f76ac53b:
	$(WGET) "https://bitbucket.org/jpommier/pffft/get/29e4f76ac53b.zip"
	$(SHA256) 29e4f76ac53b.zip bb10afba127904a0c6c553fa445082729b7d72373511bda1b12a5be0e03f318a
	$(UNZIP) 29e4f76ac53b.zip
	rm 29e4f76ac53b.zip

$(pffft): jpommier-pffft-29e4f76ac53b
	cp jpommier-pffft-29e4f76ac53b/*.h include/

# Helpers

src: glew-2.1.0 glfw jansson-2.12 speexdsp-SpeexDSP-1.2rc3 openssl-1.1.1d curl-7.66.0 libzip-1.5.2 zlib-1.2.11 rtmidi-4.0.0 rtaudio nanovg nanosvg oui-blendish osdialog jpommier-pffft-29e4f76ac53b

clean:
	git clean -fdx
	git submodule foreach git clean -fdx
