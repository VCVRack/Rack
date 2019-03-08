//
//  SIMDUtilities.hpp
//  SIMDUtilities - A set of functions and tools for SIMD operations
//
//  Created by Dale Johnson on 17/07/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
//

#pragma once
#include <intrin.h>
#include <mmintrin.h>
#include <emmintrin.h>  // SSE2
#include <xmmintrin.h>
#include <pmmintrin.h>
// #include <tmmintrin.h>  // SSE3
// #include <ivec.h>//MMX
// #include <fvec.h>//SSE(also include ivec.h)
// #include <dvec.h>//SSE2(also include fvec.h)

#ifdef BUILD_64
// see c:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.12.25827\include\mmintrin.h
// see https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_set1_pi16&expand=4942
static __m64 Dmm_set1_pi16(short s) {
   __m64 r;
   r.m64_i16[0] = s;
   r.m64_i16[1] = s;
   r.m64_i16[2] = s;
   r.m64_i16[3] = s;
   return r;
}
#else
#define Dmm_set1_pi16(x) _mm_set1_pi16(x)
#endif

#define VALLEY_1F2 0.5
#define VALLEY_1F3 0.1666666667
#define VALLEY_1F4 0.04166666667
#define VALLEY_1F5 0.008333333333
#define VALLEY_1F6 0.001388888889
#define VALLEY_1F7 0.0001984126984
#define VALLEY_1F8 0.0000248015873
#define VALLEY_1F9 0.000002755731922
#define VALLEY_1F10 0.000000275573192
#define VALLEY_1F11 0.000000025052108
#define VALLEY_1F12 0.000000002087676
#define VALLEY_1F13 0.000000001605904

#ifdef _WIN32
    inline void* aligned_alloc_16(size_t __size) {
        return _aligned_malloc(__size, 16);
    }

    inline void aligned_free_16(void* ptr) {
        _aligned_free(ptr);
    }
#elif __APPLE__
    inline void* aligned_alloc_16(size_t __size) {
        return malloc(__size);
    }

    inline void aligned_free_16(void* ptr) {
        free(ptr);
    }
#elif __linux__
    inline void* aligned_alloc_16(size_t __size) {
        return aligned_alloc(16, __size);
    }

    inline void aligned_free_16(void* ptr) {
        free(ptr);
    }
#endif

inline __m128 _mm_linterp_ps(const __m128& a, const __m128& b, const __m128& frac) {
    return _mm_add_ps(a, _mm_mul_ps(frac, _mm_sub_ps(b, a)));
}

inline __m128d _mm_linterp_pd(const __m128d& a, const __m128d& b, const __m128d& frac) {
    return _mm_add_pd(a, _mm_mul_pd(frac, _mm_sub_pd(b, a)));
}

inline __m128 _mm_abs_ps(const __m128& a) {
    __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
    return _mm_and_ps(a,mask);
}

inline __m128 _mm_switch_ps(const __m128& a, const __m128& b, const __m128& cond) {
    return _mm_or_ps(_mm_andnot_ps(cond, a), _mm_and_ps(cond, b));
}

inline __m128d _mm_switch_pd(const __m128d& a, const __m128d& b, const __m128d& cond) {
    return _mm_or_pd(_mm_andnot_pd(cond, a), _mm_and_pd(cond, b));
}

inline __m128i _mm_switch_int32(const __m128i& a, const __m128i& b, const __m128i& cond) {
    return _mm_or_si128(_mm_andnot_si128(cond, a), _mm_and_si128(cond, b));
}

#ifndef _MSC_VER
inline __m64 _mm_switch_si64(const __m64& a, const __m64& b, const __m64& cond) {
    return _mm_or_si64(_mm_andnot_si64(cond, a), _mm_and_si64(cond, b));
}
#endif

#ifndef _MSC_VER
inline __m64 _mm_switch_pi16(const __m64& a, const __m64& b, const __m64& cond) {
    return _mm_or_si64(_mm_andnot_si64(cond, a), _mm_and_si64(cond, b));
}
#endif

inline __m128 _mm_clamp_ps(const __m128& a, const __m128& min, const __m128& max) {
    return _mm_min_ps(_mm_max_ps(a, min), max);
}

inline __m128d _mm_clamp_pd(const __m128d& a, const __m128d& min, const __m128d& max) {
    return _mm_min_pd(_mm_max_pd(a, min), max);
}

inline __m128i _mm_clamp_int32(const __m128i& a, const __m128i& min, const __m128i& max) {
    return _mm_switch_int32(_mm_switch_int32(a, min, _mm_cmplt_epi32(a, min)), max, _mm_cmpgt_epi32(a, max));
}

inline __m128 _mm_posRectify_ps(const __m128& a) {
    return _mm_max_ps(a, _mm_set1_ps(0.f));
}

inline __m128d _mm_posRectify_pd(const __m128d& a) {
    return _mm_max_pd(a, _mm_set1_pd(0.0));
}

inline __m128 _mm_negRectify_ps(const __m128& a) {
    return _mm_min_ps(a, _mm_set1_ps(0.f));
}

inline __m128d _mm_negRectify_pd(const __m128d& a) {
    return _mm_min_pd(a, _mm_set1_pd(0.0));
}


inline __m128 _mm_mirror_ps(const __m128& a, const __m128& f) {
    // Make switching phasor
    __m128 ones = _mm_set1_ps(1.f);
    __m128 twos = _mm_set1_ps(2.f);
    __m128 x = _mm_sub_ps(_mm_mul_ps(a, twos), ones);
    x = _mm_mul_ps(x, _mm_add_ps(_mm_mul_ps(_mm_abs_ps(f), _mm_set1_ps(9.f)), ones));
    x = _mm_mul_ps(_mm_add_ps(x, ones), _mm_set1_ps(0.5f));
    x = _mm_abs_ps(x);
    x = _mm_mul_ps(x, _mm_set1_ps(0.5f));
    __m128i xInt = _mm_cvttps_epi32(x);
    __m128 xIntF = _mm_cvtepi32_ps(xInt);
    x = _mm_sub_ps(x, xIntF);

    __m128 y = _mm_sub_ps(_mm_mul_ps(a, twos), ones);
    y = _mm_mul_ps(y, _mm_add_ps(_mm_mul_ps(_mm_abs_ps(f), _mm_set1_ps(9.f)), ones));
    y = _mm_mul_ps(_mm_add_ps(y, ones), _mm_set1_ps(0.5f));
    y = _mm_abs_ps(y);
    __m128i yInt = _mm_cvttps_epi32(y);
    __m128 yIntF = _mm_cvtepi32_ps(yInt);
    __m128 z = _mm_sub_ps(y, yIntF);

    return _mm_switch_ps(z, _mm_sub_ps(ones, z), _mm_cmpgt_ps(x, _mm_set1_ps(0.5f)));
}

inline __m128 _mm_pinch_ps(const __m128& a, const __m128& f) {
    __m128 ones = _mm_set1_ps(1.f);
    __m128 half = _mm_set1_ps(0.5f);

    __m128 x = _mm_mul_ps(_mm_sub_ps(ones, f), half);
    __m128 mask = _mm_cmplt_ps(a, x);

    __m128 denom = _mm_add_ps(_mm_and_ps(mask, x), _mm_andnot_ps(mask, _mm_sub_ps(ones, x)));
    __m128 m = _mm_div_ps(half, denom);
    __m128 c = _mm_sub_ps(half, _mm_mul_ps(m, x));

    x = _mm_add_ps(_mm_mul_ps(m, a), _mm_andnot_ps(mask, c));

    return x;
}

inline __m128 _mm_tilt_ps(const __m128& a, const __m128& f) {
    __m128 x = _mm_mul_ps(f, _mm_set1_ps(3.f));
    x = _mm_abs_ps(x);
    x = _mm_add_ps(x, _mm_set1_ps(1.f));
    x = _mm_mul_ps(a, x);
    __m128 mask = _mm_cmplt_ps(f, _mm_set1_ps(0.f));
    x = _mm_add_ps(x, _mm_and_ps(mask, _mm_mul_ps(f, _mm_set1_ps(3.f))));
    return x;
}

inline __m128 _mm_lean_ps(const __m128& a, const __m128& f) {
    __m128 ones = _mm_set1_ps(1.f);
    __m128 mask = _mm_cmplt_ps(f, _mm_set1_ps(0.f));
    __m128 x = _mm_switch_ps(a, _mm_sub_ps(ones, a), mask);
    x = _mm_mul_ps(x, x);
    x = _mm_mul_ps(x, x);
    __m128 xx = _mm_switch_ps(x, _mm_sub_ps(ones, x), mask);
    __m128 ff = _mm_switch_ps(f, _mm_mul_ps(f, _mm_set1_ps(-1.f)), mask);
    return _mm_linterp_ps(a, xx, ff);
}

inline __m128 _mm_twist_ps(const __m128& a, const __m128& f) {
    __m128 _f = _mm_add_ps(_mm_mul_ps(f, _mm_set1_ps(1.98f)), _mm_set1_ps(1.f));
    __m128 midMask = _mm_and_ps(_mm_cmpgt_ps(a ,_mm_set1_ps(0.333333f)),
                                 _mm_cmple_ps(a ,_mm_set1_ps(0.666666f)));
    __m128 highMask = _mm_cmpgt_ps(a, _mm_set1_ps(0.666666f));

    __m128 k = _mm_add_ps(_mm_mul_ps(_f, _mm_set1_ps(-0.5f)), _mm_set1_ps(1.5f));
    __m128 x1 = _mm_mul_ps(a, k);
    __m128 x2 = _mm_add_ps(_mm_mul_ps(a, _f), _mm_mul_ps(_mm_sub_ps(_f, _mm_set1_ps(1.f)), _mm_set1_ps(-0.5f)));
    __m128 out = x1;
    out = _mm_switch_ps(out, x2, midMask);
    return _mm_switch_ps(out, _mm_add_ps(x1, _mm_sub_ps(_mm_set1_ps(1.f), k)), highMask);
}

inline __m128 _mm_wrap_ps(const __m128& a, const __m128& f) {
    __m128 x = _mm_mul_ps(a, _mm_add_ps(_mm_mul_ps(_mm_abs_ps(f), _mm_set1_ps(8.f)), _mm_set1_ps(1.f)));
    __m128i xInt = _mm_cvttps_epi32(x);
    __m128 xIntF = _mm_cvtepi32_ps(xInt);
    return _mm_sub_ps(x, xIntF);
}

inline __m128 _mm_reflect_ps(const __m128& a, const __m128& f) {
    return _mm_switch_ps(a, _mm_sub_ps(_mm_set1_ps(1.f), a), _mm_cmpgt_ps(a, f));
}

inline __m128 _mm_pulse_ps(const __m128& a, const __m128& f) {
    __m128 half = _mm_set1_ps(0.5f);
    __m128 x = _mm_mul_ps(a, half);
    x = _mm_mul_ps(x, _mm_add_ps(_mm_mul_ps(_mm_abs_ps(f), _mm_set1_ps(8.f)), _mm_set1_ps(1.f)));
    __m128i xInt = _mm_cvttps_epi32(x);
    __m128 xIntF = _mm_cvtepi32_ps(xInt);
    x = _mm_sub_ps(x, xIntF);
    return _mm_switch_ps(a, _mm_set1_ps(0.f), _mm_cmpgt_ps(x, half));
}

inline __m128 _mm_step4_ps(const __m128& a, const __m128& f) {
    __m128 aScale = _mm_mul_ps(a, _mm_set1_ps(4.f));
    __m128i aInt = _mm_cvttps_epi32(aScale);
    __m128 aIntF = _mm_cvtepi32_ps(aInt);
    aIntF = _mm_div_ps(aIntF, _mm_set1_ps(4.f));
    return _mm_linterp_ps(a, aIntF, _mm_abs_ps(f));
}

inline __m128 _mm_step8_ps(const __m128& a, const __m128& f) {
    __m128 aScale = _mm_mul_ps(a, _mm_set1_ps(8.f));
    __m128i aInt = _mm_cvttps_epi32(aScale);
    __m128 aIntF = _mm_cvtepi32_ps(aInt);
    aIntF = _mm_div_ps(aIntF, _mm_set1_ps(8.f));
    return _mm_linterp_ps(a, aIntF, _mm_abs_ps(f));
}

inline __m128 _mm_step16_ps(const __m128& a, const __m128& f) {
    __m128 aScale = _mm_mul_ps(a, _mm_set1_ps(16.f));
    __m128i aInt = _mm_cvttps_epi32(aScale);
    __m128 aIntF = _mm_cvtepi32_ps(aInt);
    aIntF = _mm_div_ps(aIntF, _mm_set1_ps(16.f));
    return _mm_linterp_ps(a, aIntF, _mm_abs_ps(f));
}

inline __m128 _mm_varStep_ps(const __m128& a, const __m128& f) {
    __m128 absF = _mm_abs_ps(f);
    __m128 ff = _mm_sub_ps(_mm_set1_ps(64.f), _mm_mul_ps(absF, _mm_set1_ps(64.f)));
    __m128 aScale = _mm_mul_ps(a, ff);
    __m128i aInt = _mm_cvttps_epi32(aScale);
    __m128 aIntF = _mm_cvtepi32_ps(aInt);
    aIntF = _mm_div_ps(aIntF, ff);
    ff = _mm_mul_ps(absF, _mm_set1_ps(100.f));
    ff = _mm_clamp_ps(ff, _mm_set1_ps(0.f), _mm_set1_ps(1.f));
    return _mm_linterp_ps(a, aIntF, ff);
}

namespace valley {
    inline __m128 _mm_sine_ps(__m128 x) {
        __m128 x2 = _mm_mul_ps(x, x);
        __m128 x3 = _mm_mul_ps(x2, x);
        __m128 x5 = _mm_mul_ps(x2, x3);
        __m128 x7 = _mm_mul_ps(x2, x5);
        __m128 x9 = _mm_mul_ps(x2, x7);
        __m128 x11 = _mm_mul_ps(x2, x9);
        __m128 out = _mm_sub_ps(x, _mm_mul_ps(x3, _mm_set1_ps((float)VALLEY_1F3)));
        out = _mm_add_ps(out, _mm_mul_ps(x5, _mm_set1_ps((float)VALLEY_1F5)));
        out = _mm_sub_ps(out, _mm_mul_ps(x7, _mm_set1_ps((float)VALLEY_1F7)));
        out = _mm_add_ps(out, _mm_mul_ps(x9, _mm_set1_ps((float)VALLEY_1F9)));
        return _mm_sub_ps(out, _mm_mul_ps(x11, _mm_set1_ps((float)VALLEY_1F11)));
    }
}
