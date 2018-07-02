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
# Custom LFO waveshapes.

import numpy


def scale(x, min=0, max=255, center=True):
  mx = x.max()
  mn = x.min()
  x = (x - mn) / (mx - mn)
  x = numpy.round(x * (max - min) + min)
  target_type = numpy.uint8
  x[x < numpy.iinfo(target_type).min] = numpy.iinfo(target_type).min
  x[x > numpy.iinfo(target_type).max] = numpy.iinfo(target_type).max
  return x.astype(target_type)


custom_lfos = []

t = numpy.arange(0, 256) / 256.0
ramp = t
triangle = 2 * t * (t < 0.5) + (2.0 - 2 * t) * (t >= 0.5)
square = (t < 0.5)
bipolar_triangle = 2 * triangle - 1.0
sine = numpy.sin(2 * numpy.pi * t)

# Ramps (5)
custom_lfos.append(ramp ** 4)
custom_lfos.append(ramp)
custom_lfos.append(triangle)
custom_lfos.append(ramp[::-1])
custom_lfos.append(ramp[::-1] ** 4)

# Bump
window = (1.0 - numpy.cos(4 * numpy.pi * t)) / 2.0
custom_lfos.append(numpy.maximum(sine, 0.0) * window)

custom_lfos.append(square.astype(float))

sine_2 = numpy.sin(2 * numpy.pi * t) + 0.7 * numpy.sin(4 * numpy.pi * t)
sine_3 = numpy.sin(2 * numpy.pi * t) + 0.7 * numpy.sin(6 * numpy.pi * t)
sine_5 = numpy.sin(2 * numpy.pi * t) + 0.7 * numpy.sin(10 * numpy.pi * t)

custom_lfos.append(sine ** 3)
custom_lfos.append(sine)
custom_lfos.append(sine_2)
custom_lfos.append(sine_3)
custom_lfos.append(sine_5)
triangle_3 = numpy.cumsum(numpy.sign(numpy.diff(sine_3)))
custom_lfos.append(numpy.array([0] + list(triangle_3)))

for fold_amount in [1.0, 0.5]:
  rotate = (numpy.arange(0, 256) + 128) % 256
  fold = (1 + fold_amount) * bipolar_triangle[rotate]
  fold[fold > 1.0] = 2.0 - fold[fold > 1.0]
  fold[fold < -1.0] = -2.0 - fold[fold < -1.0]
  custom_lfos.append(-fold)

spike = 2 ** (4 * triangle) - 1
custom_lfos.append(spike)
numpy.random.seed(999)
custom_lfos.append(numpy.random.random((256, 1)).ravel())
custom_lfos.append(sine)

lfo_waveforms = numpy.zeros((257 * len(custom_lfos),), dtype=numpy.uint8)
import pylab
for i, values in enumerate(custom_lfos):
  values = scale(values)
  lfo_waveforms[i * 257: i * 257 + 256] = values
  lfo_waveforms[i * 257 + 256] = values[0]

waveforms = [('lfo_waveforms', lfo_waveforms)]
