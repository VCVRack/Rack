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
import pylab


def scale(x):
  xc = x - x.mean()
  abs_max = numpy.abs(xc).max()
  xc /= abs_max
  return xc



def fshift(x, shift):
  s = x[:-1] + 0
  s = numpy.fft.rfft(s)
  s[1:] *= shift
  xh = numpy.fft.irfft(s)
  xh -= xh.mean()
  return numpy.array(list(xh) + [xh[0]])



def make_quadrature(name, x, angle_1=0, angle_2=0.5):
  xc = fshift(x, numpy.exp(1j * angle_1 * numpy.pi))
  xh = fshift(x, numpy.exp(1j * angle_2 * numpy.pi))
  scale = max(numpy.abs(xc).max(), numpy.abs(xh).max())
  return [(name + '_i', xc / scale), (name + '_q', xh / scale)]



SAMPLE_RATE = 96000
WAVETABLE_SIZE = 1024

t = numpy.arange(WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE) * 2 * numpy.pi

sine = -numpy.sin(t)
harmonics = -numpy.sin(t) - 0.5 * numpy.sin(2 * t) + 0.5 * numpy.sin(5 * t)
buzzy = 0
for i in xrange(7):
  buzzy += numpy.sin((1 + i) * t + 1.012 * i) * numpy.sin(1.123 * i)

iq_waveforms = []
iq_waveforms += make_quadrature('sine', sine)
iq_waveforms += make_quadrature('harmonics', harmonics)
iq_waveforms += make_quadrature('buzzy', buzzy)
