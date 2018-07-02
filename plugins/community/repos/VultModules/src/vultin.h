/*

The MIT License (MIT)

Copyright (c) 2015 Leonardo Laguna Ruiz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


NOTE: The code for the fixed-point operations is based on the project:
      https://code.google.com/p/libfixmath/

*/

#ifndef VULTIN_H
#define VULTIN_H

#include <stdint.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define static_inline static __inline
#else
#define static_inline static inline
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t fix16_t;

// Type conversion
static_inline float fix_to_float(fix16_t a)
{
   return (float)a / 0x00010000;
}
static_inline fix16_t float_to_fix(float a)
{
   float temp = a * 0x00010000;
   return (fix16_t)temp;
}

static_inline float int_to_float(int a)
{
   return (float)a;
}

static_inline int float_to_int(float a)
{
   return (int)a;
}

static_inline fix16_t int_to_fix(int a)
{
   return a * 0x00010000;
}

static_inline int fix_to_int(fix16_t a)
{
   return (a >> 16);
}

static_inline int int_clip(int v, int minv, int maxv)
{
   return v > maxv ? maxv : (v < minv ? minv : v);
}

// Basic operations for fixed point numbers
static_inline fix16_t fix_add(fix16_t x, fix16_t y)
{
   return x + y;
}

static_inline fix16_t fix_sub(fix16_t x, fix16_t y)
{
   return x - y;
}

static_inline fix16_t fix_mul(fix16_t x, fix16_t y)
{
   int64_t res = (int64_t)x * y;
   return (fix16_t)(res >> 16);
}

static_inline fix16_t fix_div(fix16_t a, fix16_t b)
{
   if (b == 0)
      return 0;
   fix16_t result = (((int64_t)a) << 16) / ((int64_t)b);
   return result;
}

static_inline fix16_t fix_minus(fix16_t x)
{
   return -x;
}

static_inline fix16_t fix_abs(fix16_t x)
{
   return x < 0 ? (-x) : x;
}

static_inline fix16_t fix_min(fix16_t a, fix16_t b)
{
   return a < b ? a : b;
}

static_inline fix16_t fix_max(fix16_t a, fix16_t b)
{
   return a > b ? a : b;
}

static_inline fix16_t fix_clip(fix16_t v, fix16_t minv, fix16_t maxv)
{
   return v > maxv ? maxv : (v < minv ? minv : v);
}

static_inline fix16_t fix_floor(fix16_t x)
{
   return (x & 0xFFFF0000);
}

static_inline fix16_t fix_not(fix16_t x)
{
   return ~x;
}

static_inline float float_eps()
{
   return 1e-18f;
}

static_inline fix16_t fix_eps()
{
   return 1;
}

static_inline float float_pi()
{
   return 3.1415926535897932384f;
}

static_inline fix16_t fix_pi()
{
   return 205887;
}

fix16_t fix_exp(fix16_t inValue);

fix16_t fix_sin(fix16_t inAngle);

fix16_t fix_cos(fix16_t inAngle);

fix16_t fix_tan(fix16_t inAngle);

fix16_t fix_sinh(fix16_t inAngle);

fix16_t fix_cosh(fix16_t inAngle);

fix16_t fix_tanh(fix16_t inAngle);

fix16_t fix_sqrt(fix16_t inValue);

/* Floating point operations */

static_inline float float_clip(float value, float low, float high)
{
   return value < low ? low : (value > high ? high : value);
}

/* Array get and set */
static_inline void float_set(float a[], int i, float value) { a[i] = value; }
static_inline float float_get(float a[], int i) { return a[i]; }
static_inline void fix_set(fix16_t a[], int i, fix16_t value) { a[i] = value; }
static_inline fix16_t fix_get(fix16_t a[], int i) { return a[i]; }
static_inline void int_set(int a[], int i, int value) { a[i] = value; }
static_inline int int_get(int a[], int i) { return a[i]; }
static_inline void bool_set(uint8_t a[], int i, uint8_t value) { a[i] = value; }
static_inline uint8_t bool_get(uint8_t a[], int i) { return a[i]; }

/* Array initialization */
void float_init_array(int size, float value, float *data);
void int_init_array(int size, int value, int *data);
void bool_init_array(int size, uint8_t value, uint8_t *data);
void fix_init_array(int size, fix16_t value, fix16_t *data);

/* Array copy */
void float_copy_array(int size, float *dest, float *src);
void int_copy_array(int size, int *dest, int *src);
void bool_copy_array(int size, uint8_t *dest, uint8_t *src);
void fix_copy_array(int size, fix16_t *dest, fix16_t *src);

static_inline uint8_t bool_not(uint8_t x)
{
   return !x;
}

/* Tables */
static_inline fix16_t *fix_wrap_array(const fix16_t x[]) { return (fix16_t *)x; };
static_inline float *float_wrap_array(const float x[]) { return (float *)x; };

/* Random numbers */
float float_random();
fix16_t fix_random();
int irandom();

/* Print values */
void float_print(float value);
void fix_print(fix16_t value);
void int_print(int value);
void string_print(char *value);
void bool_print(uint8_t value);

#ifdef __cplusplus
}
#endif

#endif // VULTIN_H
