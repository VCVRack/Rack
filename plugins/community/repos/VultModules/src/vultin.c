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
#include "vultin.h"
#include "stdio.h"

fix16_t fix_exp(fix16_t inValue)
{
   if (inValue == 0)
      return 0x00010000;
   if (inValue == 0x00010000)
      return 178145;
   if (inValue >= 681391)
      return 0x7FFFFFFF;
   if (inValue <= -772243)
      return 0;
   // The power-series converges much faster on positive values
   // and exp(-x) = 1/exp(x).
   int neg = (inValue < 0);
   if (neg)
      inValue = -inValue;
   fix16_t result = inValue + 0x00010000;
   fix16_t term = inValue;
   uint_fast8_t i;
   for (i = 2; i < 30; i++)
   {
      term = fix_mul(term, fix_div(inValue, int_to_fix(i)));
      result += term;
      if ((term < 500) && ((i > 15) || (term < 20)))
         break;
   }
   if (neg)
      result = fix_div(0x00010000, result);
   return result;
}

fix16_t fix_sin(fix16_t x0)
{
   fix16_t x1 = (x0 % 0x6487e /* 6.283185 */);
   uint8_t sign = (x1 > 0x3243f /* 3.141593 */);
   fix16_t x2 = (x1 % 0x3243f /* 3.141593 */);
   fix16_t x3;
   if (x2 > 0x1921f /* 1.570796 */)
      x3 = fix_add(0x3243f /* 3.141593 */, (-x2));
   else
      x3 = x2;
   fix16_t xp2 = fix_mul(x3, x3);
   fix16_t acc = fix_mul(x3, fix_add(0x10000 /* 1.000000 */, fix_mul(fix_add((0xffffd556 /* -0.166667 */), fix_mul(0x222 /* 0.008333 */, xp2)), xp2)));
   return (sign ? (-acc) : acc);
}

fix16_t fix_cos(fix16_t inAngle)
{
   return fix_sin(inAngle + (fix_pi() >> 1));
}

fix16_t fix_tan(fix16_t inAngle)
{
   return fix_div(fix_sin(inAngle), fix_cos(inAngle));
}

fix16_t fix_sinh(fix16_t inAngle)
{
   return fix_mul(fix_exp(inAngle) - fix_exp(-inAngle), 0x8000);
}

fix16_t fix_cosh(fix16_t inAngle)
{
   return fix_mul(fix_exp(inAngle) + fix_exp(-inAngle), 0x8000);
}

fix16_t fix_tanh(fix16_t inAngle)
{
   fix16_t e_x = fix_exp(inAngle);
   fix16_t m_e_x = fix_exp(-inAngle);
   return fix_div(e_x - m_e_x, e_x + m_e_x);
}

fix16_t fix_sqrt(fix16_t inValue)
{
   uint8_t neg = (inValue < 0);
   uint32_t num = (neg ? -inValue : inValue);
   uint32_t result = 0;
   uint32_t bit;
   uint8_t n;

   // Many numbers will be less than 15, so
   // this gives a good balance between time spent
   // in if vs. time spent in the while loop
   // when searching for the starting value.
   if (num & 0xFFF00000)
      bit = (uint32_t)1 << 30;
   else
      bit = (uint32_t)1 << 18;

   while (bit > num)
      bit >>= 2;

   // The main part is executed twice, in order to avoid
   // using 64 bit values in computations.
   for (n = 0; n < 2; n++)
   {
      // First we get the top 24 bits of the answer.
      while (bit)
      {
         if (num >= result + bit)
         {
            num -= result + bit;
            result = (result >> 1) + bit;
         }
         else
         {
            result = (result >> 1);
         }
         bit >>= 2;
      }

      if (n == 0)
      {
         // Then process it again to get the lowest 8 bits.
         if (num > 65535)
         {
            // The remainder 'num' is too large to be shifted left
            // by 16, so we have to add 1 to result manually and
            // adjust 'num' accordingly.
            // num = a - (result + 0.5)^2
            //   = num + result^2 - (result + 0.5)^2
            //   = num - result - 0.5
            num -= result;
            num = (num << 16) - 0x8000;
            result = (result << 16) + 0x8000;
         }
         else
         {
            num <<= 16;
            result <<= 16;
         }

         bit = 1 << 14;
      }
   }
   return (neg ? -(int32_t)result : (int32_t)result);
}

/* Array initialization */
void float_init_array(int size, float value, float *data)
{
   int i;
   for (i = 0; i < size; i++)
      data[i] = value;
}

void int_init_array(int size, int value, int *data)
{
   int i;
   for (i = 0; i < size; i++)
      data[i] = value;
}

void bool_init_array(int size, uint8_t value, uint8_t *data)
{
   int i;
   for (i = 0; i < size; i++)
      data[i] = value;
}

void fix_init_array(int size, fix16_t value, fix16_t *data)
{
   int i;
   for (i = 0; i < size; i++)
      data[i] = value;
}

void float_copy_array(int size, float *dest, float *src)
{
   int i;
   for (i = 0; i < size; i++)
      dest[i] = src[i];
}

void int_copy_array(int size, int *dest, int *src)
{
   int i;
   for (i = 0; i < size; i++)
      dest[i] = src[i];
}

void bool_copy_array(int size, uint8_t *dest, uint8_t *src)
{
   int i;
   for (i = 0; i < size; i++)
      dest[i] = src[i];
}

void fix_copy_array(int size, fix16_t *dest, fix16_t *src)
{
   int i;
   for (i = 0; i < size; i++)
      dest[i] = src[i];
}

float float_random()
{
   return (float)rand() / RAND_MAX;
}

fix16_t fix_random()
{
   float temp = ((float)rand() / RAND_MAX) * 0x00010000;
   return (fix16_t)temp;
}

int irandom()
{
   return (int)rand();
}

void float_print(float value)
{
   printf("%f\n", value);
}
void fix_print(fix16_t value)
{
   printf("%f\n", fix_to_float(value));
}
void int_print(int value)
{
   printf("%i\n", value);
}
void string_print(char *value)
{
   printf("%s\n", value);
}
void bool_print(uint8_t value)
{
   printf("%s\n", value ? "true" : "false");
}
