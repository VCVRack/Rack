include $(RACK_DIR)/arch.mk

LOCAL ?= .
FLAGS += -g -O3 -march=nocona

ifeq ($(ARCH), mac)
	FLAGS += -mmacosx-version-min=10.7 -stdlib=libc++
	LDFLAGS += -mmacosx-version-min=10.7 -stdlib=libc++
endif

CFLAGS += $(FLAGS)
CXXFLAGS += $(FLAGS)
export CFLAGS
export CXXFLAGS
export LDFLAGS

# Commands
WGET := curl -OL
UNTAR := tar xf
UNZIP := unzip
MAKE := make
CONFIGURE := ./configure --prefix="$(realpath $(LOCAL))"
ifeq ($(ARCH), win)
	CMAKE := cmake -G 'MSYS Makefiles'
else
	CMAKE := cmake
endif
