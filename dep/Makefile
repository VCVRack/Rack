LOCAL = $(shell pwd)

# Arch-specifics
include ../arch.mk

ifeq ($(ARCH),mac)
	MACOS_FLAGS = -mmacosx-version-min=10.7 -stdlib=libc++
	export CFLAGS = $(MACOS_FLAGS)
	export CXXFLAGS = $(MACOS_FLAGS)
	export CPPFLAGS = $(MACOS_FLAGS)
	export LDFLAGS = $(MACOS_FLAGS)
endif

# Commands
WGET = wget -nc
UNTAR = tar xf
UNZIP = unzip
ifeq ($(ARCH),win)
	CMAKE = cmake -G 'MSYS Makefiles'
else
	CMAKE = cmake
endif

# Libraries
ifeq ($(ARCH),lin)
	glew = lib/libGLEW.so
	glfw = lib/libglfw.so
	jansson = lib/libjansson.so
	libsamplerate = lib/libsamplerate.so
	libcurl = lib/libcurl.so
	libzip = lib/libzip.so
	rtmidi = lib/librtmidi.so
	rtaudio = lib/librtaudio.so
endif

ifeq ($(ARCH),mac)
	glew = lib/libGLEW.dylib
	glfw = lib/libglfw.dylib
	jansson = lib/libjansson.dylib
	libsamplerate = lib/libsamplerate.dylib
	libcurl = lib/libcurl.dylib
	libzip = lib/libzip.dylib
	rtmidi = lib/librtmidi.dylib
	rtaudio = lib/librtaudio.dylib
	openssl = lib/libssl.dylib
endif

ifeq ($(ARCH),win)
	glew = bin/glew32.dll
	glfw = bin/glfw3.dll
	jansson = bin/libjansson-4.dll
	libsamplerate = bin/libsamplerate-0.dll
	libcurl = bin/libcurl-4.dll
	libzip = bin/libzip-5.dll
	rtmidi = bin/librtmidi-4.dll
	rtaudio = bin/librtaudio.dll
	openssl = bin/libssl-1_1-x64.dll
endif

# Library configuration
ifdef RTAUDIO_ALL_APIS
ifeq ($(ARCH),mac)
	RTAUDIO_FLAGS = -DAUDIO_OSX_CORE=ON
endif
ifeq ($(ARCH),win)
	RTAUDIO_FLAGS = -DAUDIO_WINDOWS_DS=ON -DAUDIO_WINDOWS_WASAPI=ON -DAUDIO_WINDOWS_ASIO=ON
endif
ifeq ($(ARCH),lin)
	RTAUDIO_FLAGS = -DAUDIO_LINUX_ALSA=ON
endif
endif


.NOTPARALLEL:

all: $(glew) $(glfw) $(jansson) $(libsamplerate) $(libcurl) $(libzip) $(rtmidi) $(rtaudio)
	@echo ""
	@echo "#######################################"
	@echo "# Built all dependencies successfully #"
	@echo "#######################################"
	@echo ""

$(glew):
	$(WGET) https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.tgz
	$(UNTAR) glew-2.1.0.tgz
	$(MAKE) -C glew-2.1.0 glew.lib
	# On Linux, default lib path is lib64, so change it back to lib.
	$(MAKE) -C glew-2.1.0 GLEW_DEST="$(LOCAL)" LIBDIR="$(LOCAL)/lib" install

$(glfw):
	$(WGET) https://github.com/glfw/glfw/releases/download/3.2.1/glfw-3.2.1.zip
	$(UNZIP) glfw-3.2.1.zip
	cd glfw-3.2.1 && $(CMAKE) . \
		-DCMAKE_INSTALL_PREFIX="$(LOCAL)" -DBUILD_SHARED_LIBS=ON \
		-DGLFW_USE_CHDIR=OFF -DGLFW_USE_MENUBAR=ON -DGLFW_USE_RETINA=ON
	$(MAKE) -C glfw-3.2.1
	$(MAKE) -C glfw-3.2.1 install
ifeq ($(ARCH),win)
	# Not sure why the GLFW build system puts a .dll in the lib directory
	mv "$(LOCAL)/lib/glfw3.dll" "$(LOCAL)/bin/"
endif

$(jansson):
	$(WGET) http://www.digip.org/jansson/releases/jansson-2.10.tar.gz
	$(UNTAR) jansson-2.10.tar.gz
	cd jansson-2.10 && ./configure --prefix="$(LOCAL)"
	$(MAKE) -C jansson-2.10
	$(MAKE) -C jansson-2.10 install

$(libsamplerate):
	$(WGET) http://www.mega-nerd.com/SRC/libsamplerate-0.1.9.tar.gz
	$(UNTAR) libsamplerate-0.1.9.tar.gz
	cd libsamplerate-0.1.9 && ./configure --prefix="$(LOCAL)" --disable-fftw --disable-sndfile
	$(MAKE) -C libsamplerate-0.1.9/src
	$(MAKE) -C libsamplerate-0.1.9/src install

$(openssl):
	$(WGET) https://www.openssl.org/source/openssl-1.1.0g.tar.gz
	$(UNTAR) openssl-1.1.0g.tar.gz
	cd openssl-1.1.0g && ./config --prefix="$(LOCAL)"
	$(MAKE) -C openssl-1.1.0g
	$(MAKE) -C openssl-1.1.0g install
ifeq ($(ARCH),mac)
	otool -L lib/libssl.1.1.dylib
	install_name_tool -change $(PWD)/lib/libcrypto.1.1.dylib @loader_path/libcrypto.1.1.dylib lib/libssl.1.1.dylib
	otool -L lib/libssl.1.1.dylib
endif

$(libcurl): $(openssl)
	$(WGET) https://github.com/curl/curl/releases/download/curl-7_56_0/curl-7.56.0.tar.gz
	$(UNTAR) curl-7.56.0.tar.gz
	cd curl-7.56.0 && ./configure --prefix="$(LOCAL)" \
		--disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual \
		--without-zlib --without-libpsl --without-libmetalink --without-libssh2 --without-librtmp --without-winidn --without-libidn2 --without-nghttp2 \
		--without-ca-bundle --with-ca-fallback --with-ssl="$(LOCAL)"
	$(MAKE) -C curl-7.56.0
	$(MAKE) -C curl-7.56.0 install
ifeq ($(ARCH),mac)
	otool -L lib/libcurl.4.dylib
	install_name_tool -change $(PWD)/lib/libssl.1.1.dylib @loader_path/libssl.1.1.dylib lib/libcurl.4.dylib
	install_name_tool -change $(PWD)/lib/libcrypto.1.1.dylib @loader_path/libcrypto.1.1.dylib lib/libcurl.4.dylib
	otool -L lib/libcurl.4.dylib
endif

$(libzip):
	$(WGET) https://nih.at/libzip/libzip-1.2.0.tar.gz
	$(UNTAR) libzip-1.2.0.tar.gz
	cd libzip-1.2.0 && ./configure --prefix="$(LOCAL)"
	$(MAKE) -C libzip-1.2.0
	$(MAKE) -C libzip-1.2.0 install

$(rtmidi):
	$(WGET) http://www.music.mcgill.ca/~gary/rtmidi/release/rtmidi-3.0.0.tar.gz
	$(UNTAR) rtmidi-3.0.0.tar.gz
	cd rtmidi-3.0.0 && ./configure --prefix="$(LOCAL)"
	$(MAKE) -C rtmidi-3.0.0
	$(MAKE) -C rtmidi-3.0.0 install

$(rtaudio):
	git clone https://github.com/thestk/rtaudio.git
	cd rtaudio && mkdir -p cmakebuild
	cd rtaudio/cmakebuild && cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX="$(LOCAL)" $(RTAUDIO_FLAGS) ..
	$(MAKE) -C rtaudio/cmakebuild
	$(MAKE) -C rtaudio/cmakebuild install

clean:
	git clean -ffdxi
