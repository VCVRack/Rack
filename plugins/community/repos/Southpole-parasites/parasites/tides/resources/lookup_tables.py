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

sample_rate = 48000.0
lookup_tables = []
lookup_tables_32 = []

excursion = float(1 << 32)

a4_midi = 69
a4_pitch = 440.0
notes = numpy.arange(0 * 128.0, 12 * 128.0 + 16, 16) / 128.0
pitches = a4_pitch * 2 ** ((notes - a4_midi) / 12)
increments = excursion / sample_rate * pitches

lookup_tables_32.append(
    ('increments', increments.astype(int)))


"""----------------------------------------------------------------------------
Filter coefficients
----------------------------------------------------------------------------"""

cutoff = 440.0 * 2 ** ((numpy.arange(-256, 257) - 69) / 12.0)
f = cutoff / sample_rate
f[f > 0.5] = 0.5
f = 2 * numpy.pi * f
f = 1 - numpy.exp(-numpy.arccosh(2 - numpy.cos(f)))
lookup_tables_32.append(
    ('cutoff', numpy.maximum(1, f * 32767.0 * 65536.0))
)


"""----------------------------------------------------------------------------
Attenuverter curve
----------------------------------------------------------------------------"""

ATTENUVERTER_CURVE_SIZE = 256

x = numpy.arange(0, ATTENUVERTER_CURVE_SIZE + 1) / float(ATTENUVERTER_CURVE_SIZE)
x[-1] = x[-2]
x = x * 1.05 - 0.05
x = (x + numpy.abs(x)) / 2.0
x_cosine = (1.0 - numpy.cos((x ** 1.5) * numpy.pi)) / 2.0
x_power = x ** 2.0
attenuverter_curve = (x_cosine + 0.5 * x_power) / 1.5
lookup_tables.append(
    ('attenuverter_curve', numpy.round(65535 * attenuverter_curve)))



"""----------------------------------------------------------------------------
Slope compression table (give more resolution in the edges)
----------------------------------------------------------------------------"""

SLOPE_COMPRESSION_SIZE = 256

x = numpy.arange(0, SLOPE_COMPRESSION_SIZE + 1) / (SLOPE_COMPRESSION_SIZE / 2.0)
x -= 1.0
sine = numpy.sin(x * numpy.pi / 2)
lookup_tables.append(('slope_compression', numpy.round(32767.5 * (sine + 1.0))))
