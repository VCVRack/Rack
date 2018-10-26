#
# Common makefile include for tks-source and plugins
#


# n=32bit build ("x86", jit)
# y=64bit build ("amd64", no jit)
ifeq ($(BUILD_64),)
BUILD_64=y
endif

ifeq ($(BUILD_ARM),)
BUILD_ARM=n
endif

MK=linux

#
# Enable optimizations 
#
RELEASE=y

#
# Enable debug symbols.
#  Strip executable if NOT set to 'y'
#
DEBUG=n



#
# Cross compiler setup
#
#  (note) CROSS_COMPILE and CROSS_ROOT vars should be set in the shell
#          so that this file does not need to be modified for different
#          targets
#
#  (note) CROSS_TARGET can be set to 'OMAP3' (Open Pandora)
#          this will disable the AUTOLOAD_TKOPENGL hack in "tks-source/TKS_CachedScript.cpp"
#

# Host compiler (default)
#CROSS_COMPILE=

# Code Sourcery ARMv7
#CROSS_COMPILE=arm-none-linux-gnueabi-

# Montavista ARMv5
#CROSS_COMPILE=arm_v5t_le-


# Location of target root FS (on dev. host)
#CROSS_ROOT=

#CROSS_ROOT=/bsp/pandora-dev/arm-2011.09



#
# Where to install tks
#   (creates 
#         $(TKS_SITE_PREFIX)/plugins
#         $(TKS_SITE_PREFIX)/libraries
#         $(TKS_SITE_PREFIX)/applications
#         $(TKS_SITE_PREFIX)/modules 
#         directories,
#    copies tks.sh to $(TKS_PREFIX)/bin/tks and
#    tks.bin to $(TKS_PREFIX)/bin/tks.bin
#    )
#

# Target installation paths for executable and libraries/plugins/modules
#  (also used to build "tks.sh" startup script, see tks-source/install.tks)
TKS_TARGET_PREFIX=/usr
TKS_TARGET_SITE_PREFIX=$(TKS_TARGET_PREFIX)/lib/tks

# Installation paths used by development host
TKS_PREFIX=$(CROSS_ROOT)$(TKS_TARGET_PREFIX)
TKS_SITE_PREFIX=$(CROSS_ROOT)$(TKS_TARGET_SITE_PREFIX)


#
# Tool setup
#
AR        = $(CROSS_COMPILE)ar
CPP       = $(CROSS_COMPILE)g++
CC        = $(CROSS_COMPILE)gcc
AS        = $(CROSS_COMPILE)as
STRIP     = $(CROSS_COMPILE)strip
CP        = cp -f
FIND      = find
INSTALL   = ginstall
MAKE      = make
RM        = rm -f
SED       = sed
TKS       = tks
ZIP       = zip
UPX       = upx
MD5SUM    = md5sum


# 
# Number of parallel targets to make
# 
NUMJOBS=`grep -c "model name" /proc/cpuinfo`
#NUMJOBS=4


#
# Target architecture 
#
ifeq ($(BUILD_ARM),y)
ifeq ($(BUILD_64),y)
ARCH=ARM64
else
ARCH=ARM32
endif
else
ifeq ($(BUILD_64),y)
ARCH=X64
else
ARCH=X86
endif
endif


#
# C compiler flags
#
CFLAGS= -Wall $(EXTRA_CFLAGS)


#
# C++ compiler flags
#
CPPFLAGS= -Wall $(EXTRA_CPPFLAGS)


#
# Target architecture flags
#
ARCHFLAGS=
ifeq ($(BUILD_ARM),y)
ifeq ($(BUILD_64),y)
ARCHFLAGS+= -DARCH_ARM64
else
ARCHLAGS+= -DARCH_ARM32
endif
else
ifeq ($(BUILD_64),y)
ARCHFLAGS+= -DARCH_X64
else
ARCHFLAGS+= -DARCH_X86
endif
endif

CFLAGS+= $(ARCHFLAGS)
CPPFLAGS+= $(ARCHFLAGS)


#
# Assembly flags
#
AFLAGS=

ifeq ($(CROSS_TARGET),OMAP3)
AFLAGS += -mlittle-endian -march=armv7-a -mcpu=cortex-a8 -mfpu=neon
endif


#
# Linker flags
#
LDFLAGS= $(EXTRA_LDFLAGS)


#
# Extra includes
#
#EXTRA_INCLUDES=
#EXTRA_INCLUDES= -I/home/bsp/omap35x/zlib-1.2.3
EXTRA_INCLUDES= -I$(CROSS_ROOT)/usr/include



#
# Extra library paths
#
#EXTRA_LIBS=
#EXTRA_LIBS= -L/home/bsp/omap35x/zlib-1.2.3
EXTRA_LIBS= -L$(CROSS_ROOT)/usr/lib


#
# Optimization flags
#
OPTFLAGS=

ifeq ($(RELEASE),y)

ifeq ($(CROSS_TARGET),OMAP3)
CFLAGS  += -pipe -march=armv7-a -mcpu=cortex-a8 -mtune=cortex-a8 -mfpu=neon
CPPFLAGS+= -pipe -march=armv7-a -mcpu=cortex-a8 -mtune=cortex-a8 -mfpu=neon
OPTFLAGS += -O2
else
OPTFLAGS += -O3
endif

endif


#
# Debug flags
#
DBGFLAGS=

ifeq ($(DEBUG),y)
DBGFLAGS += -g
#DBGFLAGS= -g -pg
#DBGFLAGS= -ggdb3
endif


#
# Target dependent flags
#
ifeq ($(CROSS_TARGET),OMAP3)
LDFLAGS+= -Wl,-R./ 
CFLAGS   += -DOMAP3
CPPFLAGS += -DOMAP3
endif


#
# Do not define if libgcc is missing hypotf() (old versions only)
# Set to 'y' if it has.
#
HAVE_HYPOTF=y



#
# Which flags to use for plugins
#
# "hack" for VSVR build (pffft lib requires SSE)
#OPTFLAGS+= -mtune=core2
#OPTFLAGS+= -march=haswell
OPTFLAGS+= $(EXTRA_OPTFLAGS)

#OPTFLAGS_PLUGIN = $(OPTFLAGS_SIZE)
OPTFLAGS_PLUGIN = $(OPTFLAGS)


#
# Nothing to change after this line-----------------------------
#
CFLAGS+= $(EXTRA_INCLUDES) $(DBGFLAGS)
CPPFLAGS+= $(EXTRA_INCLUDES) $(DBGFLAGS)
LDFLAGS+= $(EXTRA_LIBS)

ifeq ($(HAVE_HYPOTF),y)
CFLAGS += -DHAVE_HYPOTF
CPPFLAGS += -DHAVE_HYPOTF
endif
