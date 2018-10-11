#
# Common makefile include for tks-source and plugins
#  (requires MSYS shell, MSVC and GNU make)
#

# n=32bit build ("x86", jit)
# y=64bit build ("amd64", no jit)
BUILD_64=y

MK=msvc

# invalid path, will be overwritten later on
LIB_INSTALL_PREFIX=/f/git/VeeSeeVSTRack/dep/lib/msvc/


# y=use Windows 10 SDK
# n=use Windows 7 SDK
ifeq ($(BUILD_64),y)
HAVE_WIN10=y
else
HAVE_WIN10=n
endif

#
# Where to install tks
#   (creates 
#         $(TKS_SITE_PREFIX)/plugins
#         $(TKS_SITE_PREFIX)/libraries
#         $(TKS_SITE_PREFIX)/applications
#         $(TKS_SITE_PREFIX)/modules 
#         directories,
#    copies tks.exe to $(TKS_SITE_PREFIX)/tks.exe
#    )
#
ifeq ($(BUILD_64),y)
TKS_PREFIX=/c/Program Files/tks
else
TKS_PREFIX=/c/Program Files (x86)/tks
endif
TKS_SITE_PREFIX=$(TKS_PREFIX)

# Target installation paths for executable and libraries/plugins/modules
#  (also used to build "tks.sh" startup script, see tks-source/install.tks)
#  (these vars must NOT be prefixed by $CROSS_ROOT)
TKS_TARGET_PREFIX=$(TKS_PREFIX)
TKS_TARGET_SITE_PREFIX=$(TKS_SITE_PREFIX)


# 
# Number of parallel targets to make
# 
NUMJOBS=$(NUMBER_OF_PROCESSORS)
#NUMJOBS=4


#
# Set to 'y' to use shared "msvcrtXX.dll"
# (decreases file size considerably but requires the user to have e.g. msvcrt90.dll
#  installed)
# (if unsure, choose 'y')
#
#USE_SHARED_MSVCRT=y
USE_SHARED_MSVCRT=n


#
# Set this to != 'n' to use VC/Platform libs from WinDDK
#  (requires USE_SHARED_MSVCRT=y)
#  (This is used to get of the "dll hell" msvcrtXX.dll dependencies)
#  (this disables compiler security checks because _security_check_cookie() 
#   does not seem to be exported by this version of msvcrt)
#
#WINDDK_PATH=f:/dev/winddk_msvcrt/3790.1830

# needs Windows Driver Kit Version 7.1.0 
WINDDK_PATH=c:/WinDDK/7600.16385.1

#WINDDK_PATH=n


#
# Set to 'y' to use shared "zlib1.dll"
# (otherwise compile and statically link zlib sources)
# (if unsure, choose 'y' because zlib is also used by the tkopengl plugin)
#
ifeq ($(BUILD_64),y)
USE_SHARED_ZLIB=y
else
USE_SHARED_ZLIB=n
endif


#
# Set to 'y' to use shared libpng.dll
#  (otherwise compile and statically link libpng sources)
#  (actually you need to keep this set to 'n' on Win32
#   since the official libpng Win32 binaries are for GCC
#   and I've been too lazy to build the .dll myself ._°)
#
ifeq ($(BUILD_64),y)
USE_SHARED_LIBPNG=n
else
USE_SHARED_LIBPNG=n
endif


#
# Common source directory for 3rd party libs (libpng, zlib)
#
# Now this is a bit tricky: The directory name MUST use the
# MSDOS style drive:/path naming convention, NOT the MSYS
# /drive/path one! (required by the MSVC compiler)
#
OTHER_SRC=f:/sources


#
# Path to zlib sources (required if USE_SHARED_ZLIB != y)
#  (zlib123.zip distribution)
#  (note) [19Jan2018] superceded by tks-source/zlib-1.2.11/ in windows build
#
ZLIB_SRC=$(OTHER_SRC)/zlib123


#
# Path to zlib binaries (required if USE_SHARED_ZLIB=y)
#  (zlib123-dll.zip distribution)
#  (note) [19Jan2018] superceded by tks-source/zlib-1.2.11/ in windows build
#
ZLIB_BIN=$(OTHER_SRC)/zlib123-dll


#
# Path to libpng sources (required if USE_SHARED_LIBPNG != y)
#  (lpng1235.zip)
#  (Note: you may need to fix "fileno" to "_fileno" in pngwio.c:140
#         because of ISO-C++ compliance)
#
LIBPNG_SRC=$(OTHER_SRC)/lpng1235


#
# Set this to the Visual Studio installation directory.
# Includes/libraries will be searched in $VCTK/include and $VCTK/lib
#
#VCTK=c:/Program Files (x86)/Microsoft Visual Studio 12.0/VC
VCTK=c:/Program Files (x86)/Microsoft Visual Studio 14.0/VC


#
# Set this to the Windows Kits paths that contains the standard header and ucrt lib files
#  (installed by Visual Studio 15)
#  Note: not needed when using older version of VC
#
WINKITS_INC=$(VCTK)/../../Windows Kits/10/Include/10.0.10240.0/ucrt/
ifeq ($(BUILD_64),y)
WINKITS_LIB=$(VCTK)/../../Windows Kits/10/Lib/10.0.10240.0/ucrt/x64/
else
WINKITS_LIB=$(VCTK)/../../Windows Kits/10/Lib/10.0.10240.0/ucrt/x86/
endif


#
# Set this to the platform SDK installation directory
#
ifeq ($(HAVE_WIN10),y)
W32API_INC=C:/Program Files (x86)/Windows Kits/10/Include/10.0.16299.0/um
ifeq ($(BUILD_64),y)
W32API_LIB=C:/Program Files (x86)/Windows Kits/10/Lib/10.0.16299.0/um/x64
else
W32API_LIB=C:/Program Files (x86)/Windows Kits/10/Lib/10.0.16299.0/um/x86
endif
else
#W32API=d:/Programme/Microsoft Platform SDK
#W32API=f:/fli/tools/dev/MSVCTK2003/w32api
#W32API=c:/Programme/Microsoft Platform SDK
#W32API=c:/Program Files/Microsoft SDKs/Windows/v6.0a
W32API=c:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A

W32API_INC=$(W32API)/include
ifeq ($(BUILD_64),y)
W32API_LIB=$(W32API)/lib/x64
else
W32API_LIB=$(W32API)/lib
endif
endif


#
# Set this to the DirectX SDK installation directory
#
#DXSDK_LIB=f:/fli/tools/dev/MSVCTK2003/w32api/lib
#DXSDK_LIB=c:/Programme/Microsoft DirectX SDK (December 2005)/lib/x86
#DXSDK_LIB='c:/Programme/Microsoft DirectX SDK (August 2006)/Lib/x86'
#DXSDK_LIB=c:/Program Files/Microsoft DirectX SDK (August 2008)/Lib/x86
ifeq ($(BUILD_64),y)
DXSDK_LIB=c:/Program Files (x86)/Microsoft DirectX SDK (August 2008)/Lib/x64
else
DXSDK_LIB=c:/Program Files (x86)/Microsoft DirectX SDK (August 2008)/Lib/x86
endif
#DXSDK_LIB=C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86

#DXSDK_INC=f:/fli/tools/dev/MSVCTK2003/w32api/include
#DXSDK_INC=c:/Programme/Microsoft DirectX SDK (December 2005)/include
#DXSDK_INC='c:/Programme/Microsoft DirectX SDK (August 2006)/Include'
#DXSDK_INC=c:/Program Files/Microsoft DirectX SDK (August 2008)/Include
DXSDK_INC=c:/Program Files (x86)/Microsoft DirectX SDK (August 2008)/Include
#DXSDK_INC=C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include



#
# Tool setup
#
ifeq ($(BUILD_64),y)
CC        = "$(VCTK)/bin/amd64/cl.exe"
LD        = "$(VCTK)/bin/amd64/link.exe"
LIB       = "$(VCTK)/bin/amd64/lib.exe"
else
CC        = "$(VCTK)/bin/cl.exe"
LD        = "$(VCTK)/bin/link.exe"
LIB       = "$(VCTK)/bin/lib.exe"
endif
CP        = cp -f
MAKE      = make
FIND      = /c/msys/1.0/bin/find.exe
MKDIR     = mkdir -p
RM        = rm -f
SED       = sed
TKS       = "$(TKS_PREFIX)/tks"
ZIP       = zip
MD5SUM    = md5sum
#
# UPX binary [optional]
#  (Note: UPX303w crashes without -q option during parallel build (-j 4))
#
#UPX=/h/fli/tools/dev/upx304w/upx.exe -q
UPX=/f/fli/tools/dev/upx303w/upx.exe -q




#
# C compiler flags
#
#CFLAGS= -nologo -W3 -Zp8 -GR- -EHs-c- -D_CRT_SECURE_NO_DEPRECATE -DWIN32 -MP4
CFLAGS= -nologo -W3 -Zp8  -EHs-c- -D_CRT_SECURE_NO_DEPRECATE -DWIN32
#CFLAGS += /arch:SSE3
#CFLAGS += -arch:AVX2
CFLAGS += -wo4250
#-GR-
ifeq ($(BUILD_64),y)
CFLAGS+= -DBUILD_64
endif
CFLAGS+= -I"$(WINKITS_INC)"

# workaround for windows sdk sal_supp.h __useHeader/__on_failure macro redefinitions
CFLAGS += -D_USING_V110_SDK71_

#-MP4
#-fp:fast
####CFLAGS= -nologo -W3 -Zp8 -GR- -EHs-c- -D_CRT_SECURE_NO_DEPRECATE -DWIN32 -MT -MP4
####CFLAGS= -nologo -W3 -Zp8 -GR- -D_CRT_SECURE_NO_DEPRECATE -DWIN32 -MD -MP4


#
# C++ compiler flags
#
CPPFLAGS= $(CFLAGS)
CPPFLAGS+= -I"$(WINKITS_INC)"

#
# Default linker flags
#
LDFLAGS= -INCREMENTAL:NO -VERSION:0.9 
#LDFLAGS= -INCREMENTAL:NO -MACHINE:X86 -VERSION:0.9 -DEBUG -FIXED:NO
ifeq ($(BUILD_64),y)
LDFLAGS += -MACHINE:X64 
else
LDFLAGS += -MACHINE:X86 
endif


#
# Size optimization linker flags (used by plugins)
#
LDFLAGS_SIZE=
#LDFLAGS_SIZE= -MACHINE:X86 -VERSION:0.9 -OPT:REF -OPT:ICF=10 -LTCG -INCREMENTAL:NO -NOASSEMBLY -SUBSYSTEM:CONSOLE 
#####-FORCE 
#####-NODEFAULTLIB:LIBCMT 
#####-OPT:NOWIN98 -NODEFAULTLIB


#
# Extra includes
#
EXTRA_INCLUDES=


#
# Extra library paths
#
EXTRA_LIBS=

EXTRA_LIBS+= -LIBPATH:"$(WINKITS_LIB)"


#
# Default Optimization flags
#
OPTFLAGS= -Ox -Ot
#OPTFLAGS= -Od -D_DEBUG

#-Ox
#OPTFLAGS  = /Os /O1
#/Oy 
#/fp:strict /GS-


#
# Size optimization flags (used by plugins)
#
OPTFLAGS_SIZE= -Os -GL -GF -Gy -GA


#
# Which flags to use for plugins
#
#OPTFLAGS_PLUGIN = $(OPTFLAGS_SIZE)
OPTFLAGS_PLUGIN = $(OPTFLAGS)


#
# Debug flags
#
DBGFLAGS=


#
# Do not define if msvcrt is missing hypotf() (old versions only)
# Set to 'y' if it has.
#
#HAVE_HYPOTF=y



#
# Nothing to change after this line-----------------------------
#
CFLAGS+= $(EXTRA_INCLUDES) $(DBGFLAGS)
CPPFLAGS+= $(EXTRA_INCLUDES) $(DBGFLAGS)
LDFLAGS+= $(EXTRA_LIBS)

# hack for portaudio plugin 
#  (uses alloca resulting in unresolved __alloca_probe_16 reference)
ifeq ($(FORCE_NO_WINDDK),y)
WINDDK_PATH=n
endif


ifeq ($(USE_SHARED_MSVCRT),y)
# link runtime libs dynamically..
SHARED_MSVCRT_CFLAGS = -MD
ifneq ($(WINDDK_PATH),n)
# ..using WinDDK
ifeq ($(BUILD_64),y)
SHARED_MSVCRT_LDFLAGS =  -LIBPATH:"$(WINDDK_PATH)/lib/crt/amd64" -LIBPATH:"$(WINDDK_PATH)/lib/win7/amd64"
else
SHARED_MSVCRT_LDFLAGS =  -LIBPATH:"$(WINDDK_PATH)/lib/crt/i386" -LIBPATH:"$(WINDDK_PATH)/lib/wxp/i386"
endif # BUILD_64
SHARED_MSVCRT_LDFLAGS +=  msvcrt.lib kernel32.lib bufferoverflow.lib ntdll.lib
# (note) inc/wxp does not exist in WinDDK 7.1.0
SHARED_MSVCRT_CFLAGS += -I"$(WINDDK_PATH)/inc/crt" -I"$(WINDDK_PATH)/inc/wxp" -QIfist -DDX_NEED_FPUFIX
else
# ..using VC install + Platform SDK
SHARED_MSVCRT_LDFLAGS =    -LIBPATH:"$(W32API_LIB)" msvcrt.lib kernel32.lib
#-LIBPATH:"$(VCTK)/lib"
SHARED_MSVCRT_CFLAGS += -I"$(VCTK)/include" -I"$(W32API_INC)"
endif # WINDDK_PATH
else
# link runtime libs statically
ifeq ($(TKS_LIB),y)
ifeq ($(TKS_LIB_DEBUG),y)
SHARED_MSVCRT_CFLAGS = -MDd
else
SHARED_MSVCRT_CFLAGS = -MD
endif
else
SHARED_MSVCRT_CFLAGS = -MT
endif
SHARED_MSVCRT_CFLAGS += -I"$(VCTK)/include" -I"$(W32API_INC)"
ifeq ($(BUILD_64),y)
SHARED_MSVCRT_LDFLAGS = -LIBPATH:"$(VCTK)/lib/amd64" -LIBPATH:"$(W32API_LIB)" 
else
SHARED_MSVCRT_LDFLAGS = -LIBPATH:"$(VCTK)/lib" -LIBPATH:"$(W32API_LIB)" 
endif # BUILD_64
endif # USE_SHARED_MSVCRT

LDFLAGS += $(SHARED_MSVCRT_LDFLAGS)
LDFLAGS_SIZE += $(SHARED_MSVCRT_LDFLAGS)
CFLAGS += $(SHARED_MSVCRT_CFLAGS)
CPPFLAGS += $(SHARED_MSVCRT_CFLAGS)

# for plugins only:
LDFLAGS_SIZE += -LIBPATH:"$(WINKITS_LIB)"


#
# HYPOTF emulation (tkmath)
#
ifeq ($(HAVE_HYPOTF),y)
CFLAGS += -DHAVE_HYPOTF
CPPFLAGS += -DHAVE_HYPOTF
endif
