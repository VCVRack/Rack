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
	zstd = lib/libzstd.a
	libarchive = lib/libarchive.a
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
	zstd = lib/libzstd.a
	libarchive = lib/libarchive.a
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
	zstd = lib/libzstd.a
	libarchive = lib/libarchive_static.a
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
fuzzysearchdatabase = include/FuzzySearchDatabase.hpp
ghcfilesystem = include/ghc/filesystem.hpp
tinyexpr = include/tinyexpr.h
simde = include/simde

DEPS += $(glew)
DEPS += $(glfw)
DEPS += $(jansson)
DEPS += $(libcurl)
DEPS += $(libarchive)
DEPS += $(libspeexdsp)
DEPS += $(libsamplerate)
DEPS += $(rtmidi)
DEPS += $(rtaudio)
DEPS += $(nanovg)
DEPS += $(nanosvg)
DEPS += $(oui-blendish)
DEPS += $(osdialog)
DEPS += $(pffft)
DEPS += $(fuzzysearchdatabase)
DEPS += $(ghcfilesystem)
DEPS += $(tinyexpr)
DEPS += $(simde)


DEP_LOCAL := .
include $(RACK_DIR)/dep.mk


# Targets
# These targets are all order-only "|" because we usually don't care if a library was built before or after other libraries.

glew-2.1.0:
	$(WGET) "https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.tgz"
	$(SHA256) glew-2.1.0.tgz 04de91e7e6763039bc11940095cd9c7f880baba82196a7765f727ac05a993c95
	$(UNTAR) glew-2.1.0.tgz
	rm glew-2.1.0.tgz

$(glew): | glew-2.1.0
	cd glew-2.1.0 && mkdir -p build
	cd glew-2.1.0/build && $(CMAKE) ./cmake
	$(MAKE) -C glew-2.1.0/build
	$(MAKE) -C glew-2.1.0/build install

$(glfw): | glfw
	cd glfw && mkdir -p build
	cd glfw/build && $(CMAKE) .. \
		-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	$(MAKE) -C glfw/build
	$(MAKE) -C glfw/build install

jansson-2.12:
	$(WGET) "https://github.com/akheron/jansson/releases/download/v2.12/jansson-2.12.tar.gz" || $(WGET) "http://www.digip.org/jansson/releases/jansson-2.12.tar.gz"
	$(SHA256) jansson-2.12.tar.gz 5f8dec765048efac5d919aded51b26a32a05397ea207aa769ff6b53c7027d2c9
	$(UNTAR) jansson-2.12.tar.gz
	rm jansson-2.12.tar.gz

$(jansson): | jansson-2.12
	cd jansson-2.12 && $(CONFIGURE)
	$(MAKE) -C jansson-2.12
	$(MAKE) -C jansson-2.12 install

openssl-1.1.1k:
	$(WGET) "https://www.openssl.org/source/openssl-1.1.1k.tar.gz"
	$(SHA256) openssl-1.1.1k.tar.gz 892a0875b9872acd04a9fde79b1f943075d5ea162415de3047c327df33fbaee5
	$(UNTAR) openssl-1.1.1k.tar.gz
	rm openssl-1.1.1k.tar.gz

$(openssl): | openssl-1.1.1k
	# The CFLAGS environment variable is ignored, so pass CFLAGS to their custom ./config script, which passes them to the compiler.
	# Set no-pinshared so OpenSSL doesn't call GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, ...) on initialization on Windows. This prevents libRack from being unloaded on Windows with FreeLibrary().
	cd openssl-1.1.1k && ./config --prefix="$(DEP_PATH)" no-shared no-hw threads no-zlib no-capieng no-pinshared $(CFLAGS)
	$(MAKE) -C openssl-1.1.1k
	$(MAKE) -C openssl-1.1.1k install_sw

curl-7.79.1:
	$(WGET) "https://github.com/curl/curl/releases/download/curl-7_79_1/curl-7.79.1.tar.gz" || $(WGET) "https://curl.se/download/curl-7.79.1.tar.gz"
	$(SHA256) curl-7.79.1.tar.gz 370b11201349816287fb0ccc995e420277fbfcaf76206e309b3f60f0eda090c2
	$(UNTAR) curl-7.79.1.tar.gz
	rm curl-7.79.1.tar.gz

CURL_FLAGS += --disable-symbol-hiding --enable-shared=no
CURL_FLAGS += --enable-http --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --enable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-mqtt --disable-manual --enable-threaded-resolver --enable-pthreads --enable-verbose --disable-sspi --disable-ntlm --disable-ntlm-wb --disable-tls-srp --disable-unix-sockets --disable-doh --disable-mime --disable-dateparse --disable-netrc --disable-progress-meter --disable-alt-svc
CURL_FLAGS += --without-hyper --without-zlib --without-brotli --without-zstd --without-libpsl --without-libgsasl --without-libssh2 --without-librtmp --without-winidn --without-libidn2 --without-nghttp2 --without-ngtcp2 --without-nghttp3 --without-quiche
CURL_FLAGS += --with-ssl="$(DEP_PATH)"

$(libcurl): | $(openssl) curl-7.79.1
	cd curl-7.79.1 && PKG_CONFIG_PATH= $(CONFIGURE) $(CURL_FLAGS)
	$(MAKE) -C curl-7.79.1
	$(MAKE) -C curl-7.79.1 install

zstd-1.4.5:
	$(WGET) "https://github.com/facebook/zstd/releases/download/v1.4.5/zstd-1.4.5.tar.gz"
	$(SHA256) zstd-1.4.5.tar.gz 98e91c7c6bf162bf90e4e70fdbc41a8188b9fa8de5ad840c401198014406ce9e
	$(UNTAR) zstd-1.4.5.tar.gz
	rm zstd-1.4.5.tar.gz

$(zstd): | zstd-1.4.5
	cd zstd-1.4.5/build/cmake && $(CMAKE) -DZSTD_BUILD_PROGRAMS=ON -DZSTD_BUILD_SHARED=ON -DZSTD_BUILD_STATIC=ON -DZSTD_MULTITHREAD_SUPPORT=OFF .
	$(MAKE) -C zstd-1.4.5/build/cmake
	$(MAKE) -C zstd-1.4.5/build/cmake install

libarchive-3.4.3:
	$(WGET) "https://github.com/libarchive/libarchive/releases/download/v3.4.3/libarchive-3.4.3.tar.gz" || $(WGET) "https://libarchive.org/downloads/libarchive-3.4.3.tar.gz"
	$(SHA256) libarchive-3.4.3.tar.gz ee1e749213c108cb60d53147f18c31a73d6717d7e3d2481c157e1b34c881ea39
	$(UNTAR) libarchive-3.4.3.tar.gz
	rm libarchive-3.4.3.tar.gz

$(libarchive): | $(zstd) libarchive-3.4.3
	#cd libarchive-3.4.3 && $(CONFIGURE) --enable-shared=no --enable-static=yes --disable-bsdtar --disable-bsdcat --disable-bsdcpio --disable-posix-regex-lib --disable-xattr --disable-acl --without-zlib --without-bz2lib  --without-libb2 --without-iconv --without-lz4 --without-lzma --without-cng --without-openssl --without-xml2 --without-expat
	cd libarchive-3.4.3 && mkdir -p build
	cd libarchive-3.4.3/build && $(CMAKE) -DENABLE_ACL:BOOL=OFF -DENABLE_BZip2:BOOL=OFF -DENABLE_CAT:BOOL=OFF -DENABLE_CNG:BOOL=OFF -DENABLE_COVERAGE:BOOL=OFF -DENABLE_CPIO:BOOL=OFF -DENABLE_EXPAT:BOOL=OFF -DENABLE_ICONV:BOOL=OFF -DENABLE_LIBB2:BOOL=OFF -DENABLE_LIBXML2:BOOL=OFF -DENABLE_LZ4:BOOL=OFF -DENABLE_LZMA:BOOL=OFF -DENABLE_LZO:BOOL=OFF -DENABLE_MBEDTLS:BOOL=OFF -DENABLE_NETTLE:BOOL=OFF -DENABLE_OPENSSL:BOOL=OFF -DENABLE_TAR:BOOL=OFF -DENABLE_TEST:BOOL=OFF -DENABLE_WERROR:BOOL=OFF -DENABLE_XATTR:BOOL=OFF -DENABLE_ZLIB:BOOL=OFF -DENABLE_ZSTD:BOOL=ON -DZSTD_INCLUDE_DIR:PATH=$(DEP_PATH)/include -DBUILD_TESTING:BOOL=OFF ..
	$(MAKE) -C libarchive-3.4.3/build
	$(MAKE) -C libarchive-3.4.3/build install

$(libspeexdsp): | speexdsp
	cd speexdsp && ./autogen.sh
	cd speexdsp && $(CONFIGURE)
	$(MAKE) -C speexdsp
	$(MAKE) -C speexdsp install

libsamplerate-0.1.9:
	$(WGET) "https://github.com/libsndfile/libsamplerate/releases/download/0.1.9/libsamplerate-0.1.9.tar.gz" || $(WGET) "http://www.mega-nerd.com/SRC/libsamplerate-0.1.9.tar.gz"
	$(SHA256) libsamplerate-0.1.9.tar.gz 0a7eb168e2f21353fb6d84da152e4512126f7dc48ccb0be80578c565413444c1
	$(UNTAR) libsamplerate-0.1.9.tar.gz
	rm libsamplerate-0.1.9.tar.gz

$(libsamplerate): | libsamplerate-0.1.9
	cd libsamplerate-0.1.9 && $(CONFIGURE) --disable-fftw --disable-sndfile
	$(MAKE) -C libsamplerate-0.1.9
	$(MAKE) -C libsamplerate-0.1.9 install

RTMIDI_FLAGS += --enable-shared=no
ifdef ARCH_LIN
RTMIDI_FLAGS += --with-alsa --with-jack
endif
ifdef ARCH_MAC
RTMIDI_FLAGS += --with-core --without-jack
endif
ifdef ARCH_WIN
RTMIDI_FLAGS += --with-winmm
endif

$(rtmidi): | rtmidi
	cd rtmidi && ./autogen.sh --no-configure
	cd rtmidi && $(CONFIGURE) $(RTMIDI_FLAGS)
	$(MAKE) -C rtmidi
	$(MAKE) -C rtmidi install

RTAUDIO_FLAGS += -DRTAUDIO_BUILD_STATIC_LIBS=ON -DRTAUDIO_BUILD_TESTING=OFF
ifdef ARCH_LIN
RTAUDIO_FLAGS += -DRTAUDIO_API_ALSA=ON -DRTAUDIO_API_JACK=ON -DRTAUDIO_API_PULSE=ON -DRTAUDIO_API_OSS=OFF
endif
ifdef ARCH_MAC
RTAUDIO_FLAGS += -DRTAUDIO_API_CORE=ON -DRTAUDIO_API_PULSE=OFF -DRTAUDIO_API_JACK=OFF
endif
ifdef ARCH_WIN
RTAUDIO_FLAGS += -DRTAUDIO_API_DS=ON -DRTAUDIO_API_WASAPI=ON
# ASIO is proprietary software owned by Steinberg, so distributing it with your own VCV Rack build would violate Rack's GPLv3 license.
# However, since VCV owns the copyright of all GPL'd code in the Rack package, we include ASIO in the Windows build for convenience to our users.
ifdef RTAUDIO_ASIO
RTAUDIO_FLAGS += -DRTAUDIO_API_ASIO=ON
else
RTAUDIO_FLAGS += -DRTAUDIO_API_ASIO=OFF
endif
endif

$(rtaudio): | rtaudio
	cd rtaudio && mkdir -p build
	cd rtaudio/build && $(CMAKE) $(RTAUDIO_FLAGS) ..
	$(MAKE) -C rtaudio/build
	$(MAKE) -C rtaudio/build install

$(nanovg): $(wildcard nanovg/src/*.h) nanovg/example/stb_image_write.h
	mkdir -p include
	cp $^ include/

$(nanosvg): $(wildcard nanosvg/src/*.h)
	mkdir -p include
	cp $^ include/

$(oui-blendish): $(wildcard oui-blendish/*.h)
	mkdir -p include
	cp $^ include/

$(osdialog): $(wildcard osdialog/*.h)
	mkdir -p include
	cp $^ include/

$(pffft): | pffft
	mkdir -p include
	cp pffft/*.h include/

$(fuzzysearchdatabase): fuzzysearchdatabase/src/FuzzySearchDatabase.hpp
	mkdir -p include
	cp $^ include/

$(ghcfilesystem): filesystem/include/ghc
	mkdir -p include
	cp -r $^ include/

$(tinyexpr): tinyexpr/tinyexpr.h
	mkdir -p include
	cp $^ include/

$(simde): simde/simde
	mkdir -p include
	cp -R $^ include/

# Helpers

src: glew-2.1.0 glfw jansson-2.12 libsamplerate-0.1.9 openssl-1.1.1k curl-7.79.1 zstd-1.4.5 libarchive-3.4.3 rtaudio nanovg nanosvg oui-blendish osdialog

clean:
	git clean -fdx
	git submodule foreach git clean -fdx
