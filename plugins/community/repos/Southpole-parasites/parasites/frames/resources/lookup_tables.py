#!/usr/bin/python2.5
#
# Copyright 2014 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# -----------------------------------------------------------------------------
#
# Lookup table definitions.

import numpy

lookup_tables = []
lookup_tables_32 = []

"""----------------------------------------------------------------------------
Easing
----------------------------------------------------------------------------"""

def BounceEaseIn(t, b, c, d):
  return c - BounceEaseOut(d - t, 0, c, d) + b


def BounceEaseOut(t, b, c, d):
  t /= d
  if t < 1 / 2.75:
    return c * (7.5625 * t * t) + b
  elif t < 2 / 2.75:
    t -= 1.5/2.75
    return c * (7.5625 * t * t + .75) + b
  elif t < 2.5 / 2.75:
    t -= 2.25 / 2.75
    return c * (7.5625 * t * t + .9375) + b;
  else:
    t -= 2.625 / 2.75
    return c * (7.5625 * t * t + .984375) + b;


def BounceEaseInOut(t, b, c, d):
  if (t < d / 2):
    return BounceEaseIn(t * 2, 0, c, d) * 0.5 + b
  else:
    return BounceEaseOut(t * 2 - d, 0, c, d) * 0.5 + c * 0.5 + b


x = numpy.arange(0, 1025.0) / 1024.0
steps = numpy.sign(x - 0.5) * 32767.5 + 32767.5
linear = x * 65535.0
quartic_in = (x ** 4) * 65535.0
quartic_out = (1 - (1 - x) ** 4) * 65535.0
in_out_sine = (1.0 - numpy.cos(x * numpy.pi)) / 2.0 * 65535.0
in_out_bounce = x + 0
for i in xrange(len(x)):
  in_out_bounce[i] = BounceEaseOut(x[i], 0, 65535.0, 1.0)

# lookup_tables.append(('easing_steps', steps))
# lookup_tables.append(('easing_linear', linear))
lookup_tables.append(('easing_in_quartic', quartic_in))
lookup_tables.append(('easing_out_quartic',quartic_out))
lookup_tables.append(('easing_in_out_sine', in_out_sine))
lookup_tables.append(('easing_in_out_bounce', in_out_bounce))


"""----------------------------------------------------------------------------
2164 variable-skew normalization
----------------------------------------------------------------------------"""

x = numpy.arange(0, 256.0) / 255.0
lookup_tables.append(('response_balance', numpy.round(32767 * (x ** 1.5))))

gain = numpy.linspace(1.0 / 4096, 1.0, 1025)
voltage = 65535 / 2.5 * -2.0 / 3.0 * numpy.log10(gain)
vca_linear = numpy.maximum(numpy.minimum(numpy.round(voltage), 65535), 0)
lookup_tables.append(('vca_linear', vca_linear))



"""----------------------------------------------------------------------------
Simple expo table for LED brightness adjustment
----------------------------------------------------------------------------"""

x = numpy.arange(0, 256.0) / 255.0
expo = numpy.exp(-8.0 * (1.0 - x))
lookup_tables.append(('exponential', 65535 * expo))


"""----------------------------------------------------------------------------
Phase increment lookup table for LFO mode
----------------------------------------------------------------------------"""

frequency = 110 * 2 ** (numpy.arange(0, 159.0) / 158.0 - 13)
phase_increment = frequency / 24000 * (1 << 32)
lookup_tables_32 = [('increments', numpy.round(phase_increment).astype(int))]

      
"""----------------------------------------------------------------------------
Euclidean patterns
----------------------------------------------------------------------------"""

def Flatten(l):
  if hasattr(l, 'pop'):
    for item in l:
      for j in Flatten(item):
        yield j
  else:
    yield l


def EuclideanPattern(k, n):
  pattern = [[1]] * k + [[0]] * (n - k)
  while k:
    cut = min(k, len(pattern) - k)
    k, pattern = cut, [pattern[i] + pattern[k + i] for i in xrange(cut)] + \
      pattern[cut:k] + pattern[k + cut:]
  return pattern


table = []
for num_steps in xrange(1, 33):
  for num_notes in xrange(32):
    num_notes = min(num_notes, num_steps)
    bitmask = 0
    for i, bit in enumerate(Flatten(EuclideanPattern(num_notes, num_steps))):
      if bit:
        bitmask |= (1 << i)
    table.append(bitmask)

lookup_tables_32 += [('euclidean', table)]
