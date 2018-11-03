
export VSVR_BASE_DIR=`pwd`

# y=64 bit build
# n=32 bit build
export BUILD_64=y

# y=cross compile for ARM target (untested in VSVR build)
export BUILD_ARM=n


# CPU architecture
CPU_ARCH="-msse2"

if [ "${USER}" = "bsp" ]; then
CPU_ARCH="-march=haswell"
fi

if [ "${USER}" = "cameron" ]; then
CPU_ARCH="-march=skylake-avx512"
fi

if [ "${USER}" = "dave" ]; then
CPU_ARCH="-march=athlon-fx"
fi

# Extra compiler flags (C and C++)
#EXTRA_FLAGS=""
#EXTRA_FLAGS=-DUSE_LOG_PRINTF
EXTRA_FLAGS="-DUSE_BEGIN_REDRAW_FXN -I${VSVR_BASE_DIR}/dep/lglw"

#  (note) see dep/lglw/lglw_linux.c for an important note re: LGLW_CONTEXT_ALLOW_USE_AFTER_FREE
#EXTRA_FLAGS="${EXTRA_FLAGS} -DLGLW_CONTEXT_ALLOW_USE_AFTER_FREE"

if [ "${USER}" = "cameron" ]; then
EXTRA_FLAGS="${EXTRA_FLAGS} -DLGLW_CONTEXT_ALLOW_USE_AFTER_FREE"
fi

# Extra C compiler flags
export EXTRA_CFLAGS="${CPU_ARCH} ${EXTRA_FLAGS}"

# Extra C++ compiler flags
export EXTRA_CPPFLAGS="${CPU_ARCH} ${EXTRA_FLAGS}"

# Extra optimization flags (C/C++)
export EXTRA_OPTFLAGS=

# Extra linker flags
export EXTRA_LDFLAGS=-march=${CPU_ARCH}

# Point this to the directory which contains the 'aeffect.h' and 'aeffectx.h' files
export VST2_SDK_DIR=/mnt/dev/vstsdk2.4/pluginterfaces/vst2.x/

# n = build the plugin w/o 3rd party modules (only useful for debugging purposes)
# y = statically link 3rd party modules
export RACK_STATIC_MODULES=y
#export RACK_STATIC_MODULES=n
