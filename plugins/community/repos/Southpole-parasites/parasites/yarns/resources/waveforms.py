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
# Waveform definitions.

import numpy

WAVETABLE_SIZE = 256

# Waveforms for the trigger shaper ---------------------------------------------

numpy.random.seed(666)

def scale(x):
  if x.min() > 0:
    x = (x - x.min()) / (x.max() - x.min()) * 32767.0
  else:
    abs_max = numpy.abs(x).max()
    x = x / abs_max * 32767.0
  return numpy.round(x)


t = numpy.arange(0, WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE)

exponential = numpy.exp(-4.0 * t)
ring = numpy.exp(-3.0 * t) * numpy.cos(8.0 * t * numpy.pi)
steps = numpy.sign(numpy.sin(4.0 * t * numpy.pi)) * (2 ** (-numpy.round(t * 2.0)))
noise = numpy.random.randn(WAVETABLE_SIZE + 1, 1).ravel() * ((1 - t) ** 2)

waveforms = [('exponential', scale(exponential))]
waveforms += [('ring', scale(ring))]
waveforms += [('steps', scale(steps))]
waveforms += [('noise', scale(noise))]



"""----------------------------------------------------------------------------
Band-limited waveforms
----------------------------------------------------------------------------"""

WAVETABLE_SIZE = 1024

# Sine wave.
sine = -numpy.sin(numpy.arange(WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE) * 2 * numpy.pi) * 32767

waveforms.append(('sine', scale(sine)))
