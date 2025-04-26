RACK_DIR ?= ..
.NOTPARALLEL:


# Target paths

include $(RACK_DIR)/arch.mk

ifdef ARCH_LIN
	glew = lib/libGLEW.a
	glfw = lib/libglfw3.a
	jansson = lib/libjansson.a
	openssl = lib/libcrypto.a
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
	openssl = lib/libcrypto.a
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
	openssl = lib/libcrypto.a
	libcurl = lib/libcurl.a
	zstd = lib/libzstd.a
	libarchive = lib/libarchive.a
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

glew-2.2.0:
	$(WGET) "https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz"
	$(SHA256) glew-2.2.0.tgz d4fc82893cfb00109578d0a1a2337fb8ca335b3ceccf97b97e5cc7f08e4353e1
	$(UNTAR) glew-2.2.0.tgz
	rm glew-2.2.0.tgz

$(glew): | glew-2.2.0
	cd glew-2.2.0 && mkdir -p build
	# Increase policy version to support building with Cmake 4
	cd glew-2.2.0/build && $(CMAKE) -DCMAKE_POLICY_VERSION_MINIMUM=3.5 cmake
	$(MAKE) -C glew-2.2.0/build
	$(MAKE) -C glew-2.2.0/build install

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

openssl-3.3.2:
	$(WGET) "https://github.com/openssl/openssl/releases/download/openssl-3.3.2/openssl-3.3.2.tar.gz"
	$(SHA256) openssl-3.3.2.tar.gz 2e8a40b01979afe8be0bbfb3de5dc1c6709fedb46d6c89c10da114ab5fc3d281
	$(UNTAR) openssl-3.3.2.tar.gz
	rm openssl-3.3.2.tar.gz

$(openssl): | openssl-3.3.2
	# Set no-pinshared so OpenSSL doesn't call GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, ...) on initialization on Windows. This prevents libRack from being unloaded on Windows with FreeLibrary().
	# Don't use no-shared because libcurl's configure uses the shared library to find OpenSSL.
	cd openssl-3.3.2 && ./Configure --prefix="$(DEP_PATH)" --libdir=lib no-zlib no-capieng no-pinshared no-apps no-tests no-docs no-ui-console
	$(MAKE) -C openssl-3.3.2
	$(MAKE) -C openssl-3.3.2 install_sw

curl-8.10.0:
	$(WGET) "https://github.com/curl/curl/releases/download/curl-8_10_0/curl-8.10.0.tar.gz" || $(WGET) "https://curl.se/download/curl-8.10.0.tar.gz"
	$(SHA256) curl-8.10.0.tar.gz 58c9dcf73493ae9d181fd334b3b3987ff73124621565187ade237bff1064a716
	$(UNTAR) curl-8.10.0.tar.gz
	rm curl-8.10.0.tar.gz

CURL_FLAGS += --disable-symbol-hiding --enable-shared=no
CURL_FLAGS += --enable-http --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --enable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-mqtt --disable-manual --disable-docs --disable-libcurl-option --enable-threaded-resolver --enable-pthreads --enable-verbose --disable-sspi --disable-aws --disable-ntlm --disable-tls-srp --disable-unix-sockets --disable-mime --disable-bindlocal --disable-netrc --disable-progress-meter --disable-alt-svc --disable-hsts
CURL_FLAGS += --with-openssl="$(DEP_PATH)"
CURL_FLAGS += --without-hyper --without-zlib --without-brotli --without-zstd --without-ca-fallback --without-ca-embed --without-libpsl --without-libgsasl --without-librtmp --without-winidn --without-apple-idn --without-libidn2 --without-nghttp2 --without-ngtcp2 --without-openssl-quic --without-nghttp3 --without-quiche --without-msh3 --without-libuv

$(libcurl): | $(openssl) curl-8.10.0
	cd curl-8.10.0 && PKG_CONFIG_PATH= $(CONFIGURE) $(CURL_FLAGS)
	$(MAKE) -C curl-8.10.0
	$(MAKE) -C curl-8.10.0 install

zstd-1.5.6:
	$(WGET) "https://github.com/facebook/zstd/releases/download/v1.5.6/zstd-1.5.6.tar.gz"
	$(SHA256) zstd-1.5.6.tar.gz 8c29e06cf42aacc1eafc4077ae2ec6c6fcb96a626157e0593d5e82a34fd403c1
	# HACK This tar archive creates a symlink "untar" to "tar" before tar exists. This is okay on UNIX but not on Windows where symlinks are copies of files. So create a fake "tar" so the unarchiving works.
	mkdir -p zstd-1.5.6/tests/cli-tests/bin
	touch zstd-1.5.6/tests/cli-tests/bin/zstd
	$(UNTAR) zstd-1.5.6.tar.gz
	rm zstd-1.5.6.tar.gz

$(zstd): | zstd-1.5.6
	cd zstd-1.5.6/build/cmake && $(CMAKE) -DZSTD_BUILD_PROGRAMS=OFF -DZSTD_BUILD_SHARED=OFF -DZSTD_BUILD_STATIC=ON -DZSTD_BUILD_TESTS=OFF -DZSTD_MULTITHREAD_SUPPORT=OFF .
	$(MAKE) -C zstd-1.5.6/build/cmake
	$(MAKE) -C zstd-1.5.6/build/cmake install

libarchive-3.7.7:
	$(WGET) "https://github.com/libarchive/libarchive/releases/download/v3.7.7/libarchive-3.7.7.tar.gz" || $(WGET) "https://libarchive.org/downloads/libarchive-3.7.7.tar.gz"
	$(SHA256) libarchive-3.7.7.tar.gz 4cc540a3e9a1eebdefa1045d2e4184831100667e6d7d5b315bb1cbc951f8ddff
	$(UNTAR) libarchive-3.7.7.tar.gz
	rm libarchive-3.7.7.tar.gz

$(libarchive): | $(zstd) libarchive-3.7.7
	#cd libarchive-3.7.7 && $(CONFIGURE) --enable-shared=no --enable-static=yes --disable-bsdtar --disable-bsdcat --disable-bsdcpio --disable-posix-regex-lib --disable-xattr --disable-acl --without-zlib --without-bz2lib  --without-libb2 --without-iconv --without-lz4 --without-lzma --without-cng --without-openssl --without-xml2 --without-expat
	cd libarchive-3.7.7 && mkdir -p build
	# Increase policy version to support building with Cmake 4
	cd libarchive-3.7.7/build && $(CMAKE) -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF -DENABLE_ACL=OFF -DENABLE_BZip2=OFF -DENABLE_CAT=OFF -DENABLE_CNG=OFF -DENABLE_COVERAGE=OFF -DENABLE_CPIO=OFF -DENABLE_EXPAT=OFF -DENABLE_ICONV=OFF -DENABLE_LIBB2=OFF -DENABLE_LIBXML2=OFF -DENABLE_LZ4=OFF -DENABLE_LZMA=OFF -DENABLE_LZO=OFF -DENABLE_MBEDTLS=OFF -DENABLE_NETTLE=OFF -DENABLE_OPENSSL=OFF -DENABLE_TAR=OFF -DENABLE_TEST=OFF -DENABLE_UNZIP=OFF -DENABLE_WERROR=OFF -DENABLE_XATTR=OFF -DENABLE_ZLIB=OFF -DENABLE_ZSTD=ON -DWINDOWS_VERSION="WIN7" -DZSTD_INCLUDE_DIR:PATH=$(DEP_PATH)/include -DZSTD_LIBRARY=$(DEP_PATH)/lib/libzstd.a ..
	$(MAKE) -C libarchive-3.7.7/build
	$(MAKE) -C libarchive-3.7.7/build install

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

RTAUDIO_FLAGS += -DBUILD_SHARED_LIBS=OFF
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

# RtAudio on Windows and Mac uses these macros to enable UTF-8 device names
$(rtaudio): export CXXFLAGS += -DUNICODE -D_UNICODE
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

src: glew-2.2.0 glfw jansson-2.12 libsamplerate-0.1.9 openssl-3.3.2 curl-8.10.0 zstd-1.5.6 libarchive-3.7.7 rtaudio nanovg nanosvg oui-blendish osdialog

clean:
	git clean -fdx
	git submodule foreach git clean -fdx
