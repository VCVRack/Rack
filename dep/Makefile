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
	libomp = lib/libomp.a
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
ifdef ARCH_MAC
	DEPS += $(libomp)
endif

include $(RACK_DIR)/dep.mk


# Targets

$(glew):
	$(WGET) "https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.tgz"
	$(SHA256) glew-2.1.0.tgz 04de91e7e6763039bc11940095cd9c7f880baba82196a7765f727ac05a993c95
	$(UNTAR) glew-2.1.0.tgz
	cd glew-2.1.0 && mkdir -p build
	cd glew-2.1.0/build && $(CMAKE) ./cmake
	$(MAKE) -C glew-2.1.0/build
	$(MAKE) -C glew-2.1.0/build install

$(glfw):
	cd glfw && mkdir -p build
	cd glfw/build && $(CMAKE) .. \
		-DGLFW_COCOA_CHDIR_RESOURCES=OFF -DGLFW_COCOA_MENUBAR=ON -DGLFW_COCOA_RETINA_FRAMEBUFFER=ON
	$(MAKE) -C glfw/build
	$(MAKE) -C glfw/build install

$(jansson):
	$(WGET) "http://www.digip.org/jansson/releases/jansson-2.12.tar.gz"
	$(SHA256) jansson-2.12.tar.gz 5f8dec765048efac5d919aded51b26a32a05397ea207aa769ff6b53c7027d2c9
	$(UNTAR) jansson-2.12.tar.gz
	cd jansson-2.12 && $(CONFIGURE)
	$(MAKE) -C jansson-2.12
	$(MAKE) -C jansson-2.12 install

$(libspeexdsp):
	$(WGET) "https://vcvrack.com/downloads/dep/speexdsp-SpeexDSP-1.2rc3.tgz"
	$(SHA256) speexdsp-SpeexDSP-1.2rc3.tgz c8dded1454747f65956f981c95e7f89a06abdaa2a53e8aeaa66bab2a3d59cebd
	$(UNTAR) speexdsp-SpeexDSP-1.2rc3.tgz
	cd speexdsp-SpeexDSP-1.2rc3 && $(CONFIGURE)
	$(MAKE) -C speexdsp-SpeexDSP-1.2rc3
	$(MAKE) -C speexdsp-SpeexDSP-1.2rc3 install

$(openssl):
	$(WGET) "https://www.openssl.org/source/openssl-1.1.1a.tar.gz"
	$(SHA256) openssl-1.1.1a.tar.gz fc20130f8b7cbd2fb918b2f14e2f429e109c31ddd0fb38fc5d71d9ffed3f9f41
	$(UNTAR) openssl-1.1.1a.tar.gz
	@# ./config ignores CFLAGS, so hack it in with CC
	cd openssl-1.1.1a && CC="$(CC) $(CFLAGS)" ./config --prefix="$(realpath $(DEP_LOCAL))"
	$(MAKE) -C openssl-1.1.1a
	$(MAKE) -C openssl-1.1.1a install_sw

$(libcurl): $(openssl)
	$(WGET) "https://curl.haxx.se/download/curl-7.63.0.tar.gz"
	$(SHA256) curl-7.63.0.tar.gz d483b89062832e211c887d7cf1b65c902d591b48c11fe7d174af781681580b41
	$(UNTAR) curl-7.63.0.tar.gz
	cd curl-7.63.0 && PKG_CONFIG_PATH= $(CONFIGURE) \
		--disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual --disable-shared --disable-symbol-hiding \
		--without-zlib --without-libpsl --without-libmetalink --without-libssh2 --without-librtmp --without-winidn --without-libidn2 --without-nghttp2 --without-brotli \
		--without-ca-bundle --with-ca-fallback --with-ssl=$(realpath $(DEP_LOCAL))
	$(MAKE) -C curl-7.63.0
	$(MAKE) -C curl-7.63.0 install

$(libzip): $(zlib)
	$(WGET) "https://libzip.org/download/libzip-1.5.1.tar.xz"
	$(SHA256) libzip-1.5.1.tar.xz 04ea35b6956c7b3453f1ed3f3fe40e3ddae1f43931089124579e8384e79ed372
	$(UNTAR) libzip-1.5.1.tar.xz
	cd libzip-1.5.1 && mkdir -p build
	cd libzip-1.5.1/build && $(CMAKE) .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=ON -DCMAKE_FIND_ROOT_PATH="$(realpath $(DEP_LOCAL))"
	$(MAKE) -C libzip-1.5.1/build
	$(MAKE) -C libzip-1.5.1/build install

$(zlib):
	$(WGET) "https://www.zlib.net/zlib-1.2.11.tar.gz"
	$(SHA256) zlib-1.2.11.tar.gz c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1
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
	$(SHA256) rtmidi.tgz b66ddc090eec2f3d05bdbe863760be9dce77cd525c3ca7c3a47cbabe035ebde1
	$(UNTAR) rtmidi.tgz
	cd rtmidi && $(CONFIGURE)
	$(MAKE) -C rtmidi
	$(MAKE) -C rtmidi install

ifdef ARCH_MAC
RTAUDIO_FLAGS += -DRTAUDIO_API_CORE=ON -DRTAUDIO_API_PULSE=OFF
-DRTAUDIO_API_JACK=OFF
endif
ifdef ARCH_WIN
RTAUDIO_FLAGS += -DRTAUDIO_API_DS=ON -DRTAUDIO_API_WASAPI=ON -DRTAUDIO_API_ASIO=ON
endif
ifdef ARCH_LIN
RTAUDIO_FLAGS += -DRTAUDIO_API_ALSA=ON -DRTAUDIO_API_JACK=ON -DRTAUDIO_API_PULSE=OFF -DRTAUDIO_API_OSS=OFF
endif

$(rtaudio):
	cd rtaudio && mkdir -p build
	cd rtaudio/build && $(CMAKE) $(RTAUDIO_FLAGS) ..
	$(MAKE) -C rtaudio/build
	$(MAKE) -C rtaudio/build install
	cp lib/librtaudio_static.a lib/librtaudio.a

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
	$(SHA256) 29e4f76ac53b.zip bb10afba127904a0c6c553fa445082729b7d72373511bda1b12a5be0e03f318a
	$(UNZIP) 29e4f76ac53b.zip
	cp jpommier-pffft-29e4f76ac53b/*.h include/

$(libomp):
	$(WGET) "https://releases.llvm.org/7.0.1/openmp-7.0.1.src.tar.xz"
	$(SHA256) openmp-7.0.1.src.tar.xz bf16b78a678da67d68405214ec7ee59d86a15f599855806192a75dcfca9b0d0c
	$(UNTAR) openmp-7.0.1.src.tar.xz
	cd openmp-7.0.1.src && mkdir -p build
	cd openmp-7.0.1.src/build && $(CMAKE) .. -DLIBOMP_ENABLE_SHARED=OFF
	$(MAKE) -C openmp-7.0.1.src/build
	$(MAKE) -C openmp-7.0.1.src/build install

clean:
	git clean -fdx
	git submodule foreach git clean -fdx
