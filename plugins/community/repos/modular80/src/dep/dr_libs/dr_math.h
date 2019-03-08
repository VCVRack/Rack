// Public Domain. See "unlicense" statement at the end of this file.

// NOTE: This is still very much work in progress and is only being updated as I need it. You don't want to be using this library
//       in its current state.

// QUICK NOTES
// - This library does not use SSE for its basic types (vec4, etc.). Rationale: 1) It keeps things simple; 2) SSE is not always
//   faster than the FPU(s) on modern CPUs; 3) The library can always implement functions that work on __m128 variables directly
//   in the future if the need arises; 4) It doesn't work well with the pass-by-value API this library uses.
// - Use DISABLE_SSE to disable SSE optimized functions.
// - Angles are always specified in radians, unless otherwise noted. Rationale: Consistency with the standard library and most
//   other math libraries.
//   - Use radians() and degrees() to convert between the two.

#ifndef dr_math_h
#define dr_math_h

#include <math.h>

#if defined(_MSC_VER)
#define DR_MATHCALL static __forceinline
#else
#define DR_MATHCALL static inline
#endif

#define DR_PI       3.14159265358979323846
#define DR_PIF      3.14159265358979323846f

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct
{
    float x;
    float y;
    float z;
} vec3;

typedef struct
{
    float x;
    float y;
} vec2;

typedef struct
{
    vec4 col[4];
} mat4;

typedef struct
{
    float x;
    float y;
    float z;
    float w; 
} quat;


// Radians to degrees.
DR_MATHCALL float dr_degrees(float radians)
{
    return radians * 57.29577951308232087685f;
}

// Degrees to radians.
DR_MATHCALL float dr_radians(float degrees)
{
    return degrees * 0.01745329251994329577f;
}



///////////////////////////////////////////////
//
// VEC4
//
///////////////////////////////////////////////

DR_MATHCALL vec4 vec4f(float x, float y, float z, float w)
{
    vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}
DR_MATHCALL vec4 vec4v(const float* v)
{
    return vec4f(v[0], v[1], v[2], v[3]);
}
DR_MATHCALL vec4 vec4_zero()
{
    return vec4f(0, 0, 0, 0);
}
DR_MATHCALL vec4 vec4_one()
{
    return vec4f(1, 1, 1, 1);
}



DR_MATHCALL vec4 vec4_add(vec4 a, vec4 b)
{
    return vec4f(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

DR_MATHCALL vec4 vec4_sub(vec4 a, vec4 b)
{
    return vec4f(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}


DR_MATHCALL vec4 vec4_mul(vec4 a, vec4 b)
{
    return vec4f(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
DR_MATHCALL vec4 vec4_mul_1f(vec4 a, float x)
{
    return vec4f(a.x * x, a.y * x, a.z * x, a.w * x);
}
DR_MATHCALL vec4 vec4_mul_mat4(vec4 v, mat4 m)
{
    const vec4 m0 = m.col[0];
    const vec4 m1 = m.col[1];
    const vec4 m2 = m.col[2];
    const vec4 m3 = m.col[3];

    return vec4f(
        m0.x*v.x + m0.y*v.y + m0.z*v.z + m0.w*v.w,
        m1.x*v.x + m1.y*v.y + m1.z*v.z + m1.w*v.w,
        m2.x*v.x + m2.y*v.y + m2.z*v.z + m2.w*v.w,
        m3.x*v.x + m3.y*v.y + m3.z*v.z + m3.w*v.w
    );
}


DR_MATHCALL vec4 vec4_div(vec4 a, vec4 b)
{
    return vec4f(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}






///////////////////////////////////////////////
//
// VEC3
//
///////////////////////////////////////////////

DR_MATHCALL vec3 vec3f(float x, float y, float z)
{
    vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}
DR_MATHCALL vec3 vec3v(const float* v)
{
    return vec3f(v[0], v[1], v[2]);
}
DR_MATHCALL vec3 vec3_zero()
{
    return vec3f(0, 0, 0);
}
DR_MATHCALL vec3 vec3_one()
{
    return vec3f(1, 1, 1);
}


DR_MATHCALL vec3 vec3_add(vec3 a, vec3 b)
{
    return vec3f(a.x + b.x, a.y + b.y, a.z + b.z);
}

DR_MATHCALL vec3 vec3_sub(vec3 a, vec3 b)
{
    return vec3f(a.x - b.x, a.y - b.y, a.z - b.z);
}


DR_MATHCALL vec3 vec3_mul(vec3 a, vec3 b)
{
    return vec3f(a.x * b.x, a.y * b.y, a.z * b.z);
}
DR_MATHCALL vec3 vec3_mul_1f(vec3 a, float x)
{
    return vec3f(a.x * x, a.y * x, a.z * x);
}


DR_MATHCALL vec3 vec3_div(vec3 a, vec3 b)
{
    return vec3f(a.x / b.x, a.y / b.y, a.z / b.z);
}


DR_MATHCALL float vec3_dot(vec3 a, vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}


DR_MATHCALL float vec3_length2(vec3 a)
{
    return vec3_dot(a, a);
}

DR_MATHCALL float vec3_length(vec3 a)
{
    return sqrtf(vec3_length2(a));
}


DR_MATHCALL vec3 vec3_normalize(vec3 a)
{
    float len = vec3_length(a);

    return vec3f(
        a.x / len,
        a.y / len,
        a.z / len
    );
}

DR_MATHCALL vec3 vec3_cross(vec3 a, vec3 b)
{
    return vec3f(
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    );
}


DR_MATHCALL vec3 vec3_triangle_normal(vec3 p1, vec3 p2, vec3 p3)
{
    vec3 u = vec3_sub(p2, p1);
    vec3 v = vec3_sub(p3, p1);
    return vec3_normalize(vec3_cross(u, v));
}



///////////////////////////////////////////////
//
// VEC2
//
///////////////////////////////////////////////

DR_MATHCALL vec2 vec2f(float x, float y)
{
    vec2 result;
    result.x = x;
    result.y = y;

    return result;
}
DR_MATHCALL vec2 vec2v(const float* v)
{
    return vec2f(v[0], v[1]);
}
DR_MATHCALL vec2 vec2_zero()
{
    return vec2f(0, 0);
}
DR_MATHCALL vec2 vec2_one()
{
    return vec2f(1, 1);
}


DR_MATHCALL vec2 vec2_add(vec2 a, vec2 b)
{
    return vec2f(a.x + b.x, a.y + b.y);
}

DR_MATHCALL vec2 vec2_sub(vec2 a, vec2 b)
{
    return vec2f(a.x - b.x, a.y - b.y);
}


DR_MATHCALL vec2 vec2_mul(vec2 a, vec2 b)
{
    return vec2f(a.x * b.x, a.y * b.y);
}
DR_MATHCALL vec2 vec2_mul_1f(vec2 a, float x)
{
    return vec2f(a.x * x, a.y * x);
}


DR_MATHCALL vec2 vec2_div(vec2 a, vec2 b)
{
    return vec2f(a.x / b.x, a.y / b.y);
}


DR_MATHCALL float vec2_dot(vec2 a, vec2 b)
{
    return a.x*b.x + a.y*b.y;
}


DR_MATHCALL float vec2_length2(vec2 a)
{
    return vec2_dot(a, a);
}

DR_MATHCALL float vec2_length(vec2 a)
{
    return sqrtf(vec2_length2(a));
}


DR_MATHCALL vec2 vec2_normalize(vec2 a)
{
    float len = vec2_length(a);

    return vec2f(
        a.x / len,
        a.y / len
    );
}


DR_MATHCALL float vec2_angle(vec2 a, vec2 b)
{
    return atanf(a.y / a.x) - atanf(b.y / b.x);
}

DR_MATHCALL vec2 vec2_rotate(vec2 a, float angleInRadians)
{
    float c = cosf(angleInRadians);
    float s = sinf(angleInRadians);

    return vec2f(
        a.x*c - a.y*s,
        a.x*s + a.y*c
    );
}


///////////////////////////////////////////////
//
// MAT4
//
///////////////////////////////////////////////

DR_MATHCALL mat4 mat4f(vec4 col0, vec4 col1, vec4 col2, vec4 col3)
{
    mat4 result;
    result.col[0] = col0;
    result.col[1] = col1;
    result.col[2] = col2;
    result.col[3] = col3;

    return result;
}

DR_MATHCALL mat4 mat4_identity()
{
    mat4 result;
    result.col[0] = vec4f(1, 0, 0, 0);
    result.col[1] = vec4f(0, 1, 0, 0);
    result.col[2] = vec4f(0, 0, 1, 0);
    result.col[3] = vec4f(0, 0, 0, 1);

    return result;
}

DR_MATHCALL mat4 mat4_ortho(float left, float right, float bottom, float top, float znear, float zfar)
{
    float rml = right - left;
    float tmb = top - bottom;
    float fmn = zfar - znear;

    float rpl = right + left;
    float tpb = top + bottom;
    float fpn = zfar + znear;

    mat4 result;
    result.col[0] = vec4f(2/rml, 0, 0,  0);
    result.col[1] = vec4f(0, 2/tmb, 0,  0);
    result.col[2] = vec4f(0, 0, -2/fmn, 0);
    result.col[3] = vec4f(-(rpl/rml), -(tpb/tmb), -(fpn/fmn), 1);

    return result;
}

DR_MATHCALL mat4 mat4_perspective(float fovy, float aspect, float znear, float zfar)
{
    float f = (float)tan(DR_PI/2 - fovy/2);

    mat4 result;
    result.col[0] = vec4f(f / aspect, 0, 0, 0);
    result.col[1] = vec4f(0, f, 0, 0);
    result.col[2] = vec4f(0, 0,     (zfar + znear) / (znear - zfar), -1);
    result.col[3] = vec4f(0, 0, (2 * zfar * znear) / (znear - zfar),  0);

    return result;
}

DR_MATHCALL mat4 mat4_vulkan_clip_correction()
{
    mat4 result;
    result.col[0] = vec4f(1,  0, 0,    0);
    result.col[1] = vec4f(0, -1, 0,    0);
    result.col[2] = vec4f(0,  0, 0.5f, 0);
    result.col[3] = vec4f(0,  0, 0.5f, 1);

    return result;
}

DR_MATHCALL mat4 mat4_translate(vec3 translation)
{
    mat4 result;
    result.col[0] = vec4f(1, 0, 0, 0);
    result.col[1] = vec4f(0, 1, 0, 0);
    result.col[2] = vec4f(0, 0, 1, 0);
    result.col[3] = vec4f(translation.x, translation.y, translation.z, 1);

    return result;
}

DR_MATHCALL mat4 mat4_rotate(float angleInRadians, vec3 axis)
{
    float c = cosf(angleInRadians);
    float s = sinf(angleInRadians);

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    float xx = x*x;
    float xy = x*y;
    float xz = x*z;
    float yy = y*y;
    float yz = y*z;
    float zz = z*z;

    float xs = x*s;
    float ys = y*s;
    float zs = z*s;

    mat4 result;
    result.col[0] = vec4f(xx * (1 - c) + c,  xy * (1 - c) - zs, xz * (1 - c) + ys, 0);
    result.col[1] = vec4f(xy * (1 - c) + zs, yy * (1 - c) + c,  yz * (1 - c) - xs, 0);
    result.col[2] = vec4f(xz * (1 - c) - ys, yz * (1 - c) + xs, zz * (1 - c) + c,  0);
    result.col[3] = vec4f(0,                 0,                 0,                 1);

    return result;
}

DR_MATHCALL mat4 mat4_scale(vec3 scale)
{
    mat4 result;
    result.col[0] = vec4f(scale.x, 0, 0, 0);
    result.col[1] = vec4f(0, scale.y, 0, 0);
    result.col[2] = vec4f(0, 0, scale.z, 0);
    result.col[3] = vec4f(0, 0, 0, 1);

    return result;
}


DR_MATHCALL mat4 mat4_mul(mat4 a, mat4 b)
{
    const vec4 a0 = a.col[0];
    const vec4 a1 = a.col[1];
    const vec4 a2 = a.col[2];
    const vec4 a3 = a.col[3];

    const vec4 b0 = b.col[0];
    const vec4 b1 = b.col[1];
    const vec4 b2 = b.col[2];
    const vec4 b3 = b.col[3];

    mat4 result;
    result.col[0] = vec4f(
        a0.x*b0.x + a1.x*b0.y + a2.x*b0.z + a3.x*b0.w,
        a0.y*b0.x + a1.y*b0.y + a2.y*b0.z + a3.y*b0.w,
        a0.z*b0.x + a1.z*b0.y + a2.z*b0.z + a3.z*b0.w,
        a0.w*b0.x + a1.w*b0.y + a2.w*b0.z + a3.w*b0.w
    );

    result.col[1] = vec4f(
        a0.x*b1.x + a1.x*b1.y + a2.x*b1.z + a3.x*b1.w,
        a0.y*b1.x + a1.y*b1.y + a2.y*b1.z + a3.y*b1.w,
        a0.z*b1.x + a1.z*b1.y + a2.z*b1.z + a3.z*b1.w,
        a0.w*b1.x + a1.w*b1.y + a2.w*b1.z + a3.w*b1.w
    );

    result.col[2] = vec4f(
        a0.x*b2.x + a1.x*b2.y + a2.x*b2.z + a3.x*b2.w,
        a0.y*b2.x + a1.y*b2.y + a2.y*b2.z + a3.y*b2.w,
        a0.z*b2.x + a1.z*b2.y + a2.z*b2.z + a3.z*b2.w,
        a0.w*b2.x + a1.w*b2.y + a2.w*b2.z + a3.w*b2.w
    );

    result.col[3] = vec4f(
        a0.x*b3.x + a1.x*b3.y + a2.x*b3.z + a3.x*b3.w,
        a0.y*b3.x + a1.y*b3.y + a2.y*b3.z + a3.y*b3.w,
        a0.z*b3.x + a1.z*b3.y + a2.z*b3.z + a3.z*b3.w,
        a0.w*b3.x + a1.w*b3.y + a2.w*b3.z + a3.w*b3.w
    );

    return result;
}

DR_MATHCALL vec4 mat4_mul_vec4(mat4 m, vec4 v)
{
    const vec4 m0 = m.col[0];
    const vec4 m1 = m.col[1];
    const vec4 m2 = m.col[2];
    const vec4 m3 = m.col[3];

    return vec4f(
        m0.x*v.x + m1.x*v.y + m2.x*v.z + m3.x*v.w,
        m0.y*v.x + m1.y*v.y + m2.y*v.z + m3.y*v.w,
        m0.z*v.x + m1.z*v.y + m2.z*v.z + m3.z*v.w,
        m0.w*v.x + m1.w*v.y + m2.w*v.z + m3.w*v.w
    );
}


///////////////////////////////////////////////
//
// QUAT
//
///////////////////////////////////////////////

DR_MATHCALL quat quatf(float x, float y, float z, float w)
{
    quat result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}
DR_MATHCALL quat quatv(const float* v)
{
    return quatf(v[0], v[1], v[2], v[3]);
}

DR_MATHCALL quat quat_identity()
{
    return quatf(0, 0, 0, 1);
}





///////////////////////////////////////////////
//
// TRANSFORM
//
///////////////////////////////////////////////

typedef struct
{
    vec3 position;
    quat rotation;
    vec3 scale;
}transform_t;

DR_MATHCALL transform_t transform_init(vec3 position, quat rotation, vec3 scale)
{
    transform_t result;
    result.position = position;
    result.rotation = rotation;
    result.scale    = scale;

    return result;
}

DR_MATHCALL transform_t transform_identity()
{
    transform_t result;
    result.position = vec3_zero();
    result.rotation = quat_identity();
    result.scale    = vec3_one();

    return result;
}


DR_MATHCALL transform_t transform_translate(transform_t transform, vec3 offset)
{
    transform_t result = transform;
    result.position = vec3_add(transform.position, offset);

    return result;
}




///////////////////////////////////////////////
//
// SSE IMPLEMENTATION
//
///////////////////////////////////////////////

// Not supporting SSE on x86/MSVC due to pass-by-value errors with aligned types.
#if (defined(_MSC_VER) && defined(_M_X64)) || defined(__SSE2__)
#define SUPPORTS_SSE
#endif

#if !defined(DISABLE_SSE) && defined(SUPPORTS_SSE)
#define ENABLE_SSE
#endif

#ifdef ENABLE_SSE
#if defined(__MINGW32__)
#include <intrin.h>
#endif
#include <emmintrin.h>
#endif





#ifdef __cplusplus
}
#endif

#endif  //dr_math_h

/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
