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

import scipy.signal
import numpy
import pylab

lookup_tables = []
int16_lookup_tables = []

"""----------------------------------------------------------------------------
Cosine table.
----------------------------------------------------------------------------"""

size = 1024
t = numpy.arange(0, size + size / 4 + 1) / float(size) * numpy.pi * 2
lookup_tables.append(('sin', numpy.sin(t)))



"""----------------------------------------------------------------------------
Grain window.
----------------------------------------------------------------------------"""

size = 4096
t = numpy.arange(0, size + 1) / float(size)
lookup_tables.append(('window', 1.0 - (numpy.cos(t * numpy.pi) + 1) / 2))



"""----------------------------------------------------------------------------
XFade table
----------------------------------------------------------------------------"""

size = 17
t = numpy.arange(0, size) / float(size-1)
t = 1.04 * t - 0.02
t[t < 0] = 0
t[t >= 1] = 1
t *= numpy.pi / 2
lookup_tables.append(('xfade_in', numpy.sin(t) * 2 ** -0.5))
lookup_tables.append(('xfade_out', numpy.cos(t) * 2 ** -0.5))



"""----------------------------------------------------------------------------
Sine window.
----------------------------------------------------------------------------"""

def sum_window(window, steps):
  n = window.shape[0]
  start = 0
  stride = n / steps
  s = 0
  for i in xrange(steps):
    s = s + window[start:start+stride] ** 2
    start += stride
  return s


window_size = 4096
t = numpy.arange(0.0, window_size) / window_size

# Perfect reconstruction for overlap of 2
sine = numpy.sin(numpy.pi * t)

# Perfect reconstruction for overlap of 4
raised = (0.5 * numpy.cos(numpy.pi * t * 2) + 0.5) * numpy.sqrt(4.0 / 3.0)

# Needs tweaks to provide good reconstruction
power = (1.0 - (2 * t - 1.0) ** 2.0) ** 1.25
compensation = sum_window(power, 2) ** 0.5
compensation = numpy.array(list(compensation) * 2)
power /= compensation

lookup_tables.append(('sine_window_4096', power))



"""----------------------------------------------------------------------------
Linear to dB, for display
----------------------------------------------------------------------------"""

db = numpy.arange(0, 257)
db[0] = 1
db[db > 255] = 255
db = numpy.log2(db / 16.0) * 32768 / 4
int16_lookup_tables += [('db', db)]



"""----------------------------------------------------------------------------
LPG cutoff
----------------------------------------------------------------------------"""

TABLE_SIZE = 256

cutoff = numpy.arange(0.0, TABLE_SIZE + 1) / TABLE_SIZE
lookup_tables.append(('cutoff', 0.49 * 2 ** (-6 * (1 - cutoff))))



"""----------------------------------------------------------------------------
Grain size table
----------------------------------------------------------------------------"""

size = numpy.arange(0.0, TABLE_SIZE + 1) / TABLE_SIZE * 4
lookup_tables.append(('grain_size', numpy.floor(1024 * (2 ** size))))



"""----------------------------------------------------------------------------
Quantizer for pitch.
----------------------------------------------------------------------------"""

PITCH_TABLE_SIZE = 1025
pitch = numpy.zeros((PITCH_TABLE_SIZE, ))
notches = [-24, -12, -7, -4, -3, -1, -0.1, 0, 0, 0.1, 1, 3, 4, 7, 12, 24]
n = len(notches) - 1
for i in xrange(n):
  start_index = int(float(i) / n * PITCH_TABLE_SIZE)
  end_index = int(float(i + 1) / n * PITCH_TABLE_SIZE)
  length = end_index - start_index
  x = numpy.arange(0.0, length) / (length - 1)
  raised_cosine = 0.5 - 0.5 * numpy.cos(x * numpy.pi)
  xfade = 0.8 * raised_cosine + 0.2 * x
  pitch[start_index:end_index] = notches[i] + (notches[i + 1] - notches[i]) * xfade

lookup_tables.append(('quantized_pitch', pitch))
