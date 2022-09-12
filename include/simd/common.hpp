#pragma once

#if defined ARCH_X64
	// Intel intrinsics header
	#include <x86intrin.h>
#elif defined ARCH_ARM64
	// Translation header for using SSE3 intrinsics on ARM64 NEON
	#include <sse2neon.h>
#endif
