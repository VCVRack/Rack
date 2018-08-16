#!/usr/bin/python2.5
#
# Copyright 2015 Olivier Gillet.
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
int16_lookup_tables = []
uint32_lookup_tables = []

SAMPLE_RATE = 48000.0



"""----------------------------------------------------------------------------
Sine table
----------------------------------------------------------------------------"""

WAVETABLE_SIZE = 4096
t = numpy.arange(0.0, WAVETABLE_SIZE + WAVETABLE_SIZE / 4 + 1) / WAVETABLE_SIZE
x = numpy.sin(2 * numpy.pi * t)
lookup_tables += [('sine', x)]



"""----------------------------------------------------------------------------
Exponentials covering several decades in 256 steps, with safeguard
----------------------------------------------------------------------------"""

x = numpy.arange(0, 257) / 256.0
lookup_tables += [('4_decades', 10 ** (4 * x))]



"""----------------------------------------------------------------------------
Delay compensation factor for SVF
----------------------------------------------------------------------------"""

ratio = 2.0 ** (numpy.arange(0, 257) / 12.0)
svf_shift = 2.0 * numpy.arctan(1.0 / ratio) / (2.0 * numpy.pi)
lookup_tables += [('svf_shift', svf_shift)]



"""----------------------------------------------------------------------------
Stiffness table.
----------------------------------------------------------------------------"""

structure = numpy.arange(0, 257) / 256.0
stiffness = structure + 0
for i, g in enumerate(structure):
  if g < 0.25:
    g = 0.25 - g
    stiffness[i] = -g * 0.25
  elif g < 0.3:
    stiffness[i] = 0.0
  elif g < 0.9:
    g -= 0.3
    g /= 0.6
    stiffness[i] = 0.01 * 10 ** (g * 2.005) - 0.01
  else:
    g -= 0.9
    g /= 0.1
    g *= g
    stiffness[i] = 1.5 - numpy.cos(g * numpy.pi) / 2.0

stiffness[-1] = 2.0
stiffness[-2] = 2.0
lookup_tables += [('stiffness', stiffness)]



"""----------------------------------------------------------------------------
Quantizer for FM frequencies.
----------------------------------------------------------------------------"""

fm_frequency_ratios = [ 0.5, 0.5 * 2 ** (16 / 1200.0),
  numpy.sqrt(2) / 2, numpy.pi / 4, 1.0, 1.0 * 2 ** (16 / 1200.0), numpy.sqrt(2),
  numpy.pi / 2, 7.0 / 4, 2, 2 * 2 ** (16 / 1200.0), 9.0 / 4, 11.0 / 4,
  2 * numpy.sqrt(2), 3, numpy.pi, numpy.sqrt(3) * 2, 4, numpy.sqrt(2) * 3,
  numpy.pi * 3 / 2, 5, numpy.sqrt(2) * 4, 8]

scale = []
for ratio in fm_frequency_ratios:
  ratio = 12 * numpy.log2(ratio)
  scale.extend([ratio, ratio, ratio])

target_size = int(2 ** numpy.ceil(numpy.log2(len(scale))))
while len(scale) < target_size:
  gap = numpy.argmax(numpy.diff(scale))
  scale = scale[:gap + 1] + [(scale[gap] + scale[gap + 1]) / 2] + \
      scale[gap + 1:]

scale.append(scale[-1])

lookup_tables.append(
    ('fm_frequency_quantizer', scale)
)
