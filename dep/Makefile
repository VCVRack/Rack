
LOCAL = $(PWD)

# Arch-specifics
include ../arch.mk

ifeq ($(ARCH),lin)
	CFLAGS += -DPMALSA
endif

ifeq ($(ARCH),mac)
	export CFLAGS = -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk \
		-mmacosx-version-min=10.7
	export CXXFLAGS = $(CFLAGS)
	export LDFLAGS = $(CFLAGS)
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

# Packages
glew = glew-2.1.0
glfw = glfw-3.2.1
jansson = jansson-2.10
libsamplerate = libsamplerate-0.1.9
libcurl = curl-7.54.1
libzip = libzip-1.2.0
portmidi = portmidi
portaudio = portaudio
asio = asiosdk2.3


.NOTPARALLEL:

all: $(glew) $(glfw) $(jansson) $(libsamplerate) $(libcurl) $(libzip) $(portmidi) $(portaudio)

$(glew):
	$(WGET) https://downloads.sourceforge.net/project/glew/glew/$(patsubst glew-%,%,$@)/$@.tgz
	$(UNTAR) $@.tgz
	$(MAKE) -C $@ glew.lib
	# On Linux, default lib path is lib64, so change it back to lib.
	$(MAKE) -C $@ GLEW_DEST="$(LOCAL)" LIBDIR="$(LOCAL)/lib" install

$(glfw):
	$(WGET) https://github.com/glfw/glfw/releases/download/$(patsubst glfw-%,%,$@)/$@.zip
	$(UNZIP) $@.zip
	cd $@ && $(CMAKE) . \
		-DCMAKE_INSTALL_PREFIX="$(LOCAL)" -DBUILD_SHARED_LIBS=ON \
		-DGLFW_USE_CHDIR=OFF -DGLFW_USE_MENUBAR=ON -DGLFW_USE_RETINA=ON
	$(MAKE) -C $@
	$(MAKE) -C $@ install
ifeq ($(ARCH),win)
	# Not sure why the GLFW build system puts a .dll in the lib directory
	mv "$(LOCAL)/lib/glfw3.dll" "$(LOCAL)/bin/"
endif

$(jansson):
	$(WGET) http://www.digip.org/jansson/releases/$@.tar.gz
	$(UNTAR) $@.tar.gz
	cd $@ && ./configure --prefix="$(LOCAL)"
	$(MAKE) -C $@
	$(MAKE) -C $@ install

$(libsamplerate):
	$(WGET) http://www.mega-nerd.com/SRC/$@.tar.gz
	$(UNTAR) $@.tar.gz
	cd $@ && ./configure --prefix="$(LOCAL)"
	$(MAKE) -C $@
	$(MAKE) -C $@ install

$(libcurl):
	$(WGET) https://curl.haxx.se/download/$@.tar.gz
	$(UNTAR) $@.tar.gz
	cd $@ && ./configure --prefix="$(LOCAL)" \
		--disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual \
		--without-zlib --without-ssl --without-ca-bundle --without-ca-path --without-ca-fallback --without-libpsl --without-libmetalink --without-libssh2 --without-librtmp --without-winidn --without-libidn2 --without-nghttp2
	$(MAKE) -C $@
	$(MAKE) -C $@ install

$(libzip):
	$(WGET) https://nih.at/libzip/$@.tar.gz
	$(UNTAR) $@.tar.gz
	cd $@ && ./configure --prefix="$(LOCAL)"
	$(MAKE) -C $@
	$(MAKE) -C $@ install

$(portmidi):
	git clone https://github.com/AndrewBelt/portmidi.git $@
	cd $@ && $(CMAKE) . -DCMAKE_INSTALL_PREFIX="$(LOCAL)"
	$(MAKE) -C $@
	$(MAKE) -C $@ install

$(portaudio):
ifeq ($(ARCH),win)
	$(WGET) https://github.com/adfernandes/precompiled-portaudio-windows/raw/master/portaudio-r1891-build.zip
	$(UNZIP) portaudio-r1891-build.zip
	mv portaudio-r1891-build $@
	cp portaudio/lib/x64/ReleaseMinDependency/portaudio_x64.lib "$(LOCAL)/lib"
	cp portaudio/lib/x64/ReleaseMinDependency/portaudio_x64.dll "$(LOCAL)/bin"
	cp portaudio/include/*.h "$(LOCAL)/include"
else
	$(WGET) http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz
	$(UNTAR) pa_stable_v190600_20161030.tgz
	cd $@ && ./configure --prefix="$(LOCAL)" $(PORTAUDIO_ASIO)
	$(MAKE) -C $@
	$(MAKE) -C $@ install
endif

clean:
	git clean -fdxi
