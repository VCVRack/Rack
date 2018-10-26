
export VSVR_BASE_DIR=`pwd`

# y=64 bit build
# n=32 bit build
export BUILD_64=y

# y=cross compile for ARM target (untested in VSVR build)
export BUILD_ARM=n


# CPU architecture
#CPU_ARCH=haswell
CPU_ARCH=athlon-fx

if [ "${USER}" = "bsp" ]; then
CPU_ARCH=haswell
fi

if [ "${USER}" = "cameron" ]; then
CPU_ARCH=haswell
fi

# Extra C compiler flags
export EXTRA_CFLAGS=-march=${CPU_ARCH}

# Extra C++ compiler flags
export EXTRA_CPPFLAGS=-march=${CPU_ARCH}

# Extra optimization flags (C/C++)
export EXTRA_OPTFLAGS=

# Extra linker flags
export EXTRA_LDFLAGS=-march=${CPU_ARCH}

# this must point to the directory which contains the 'aeffect.h' and 'aeffectx.h' files
export VST2_SDK_DIR=/mnt/dev/vstsdk2.4/pluginterfaces/vst2.x/

# n = build the plugin w/o 3rd party modules (only useful for debugging purposes)
# y = statically link 3rd party modules
#export RACK_STATIC_MODULES=y
export RACK_STATIC_MODULES=n
