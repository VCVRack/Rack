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

WAVETABLE_SIZE = 1024

waveforms = []

"""----------------------------------------------------------------------------
Sine wave
----------------------------------------------------------------------------"""

x = numpy.arange(0, WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE)
x[-1] = x[0]
sine = numpy.sin(2 * numpy.pi * x)
waveforms.append(('sine', (32767 * sine).astype(int)))


"""----------------------------------------------------------------------------
Waveshaper
----------------------------------------------------------------------------"""

x = numpy.arange(0, WAVETABLE_SIZE + 1) / (WAVETABLE_SIZE / 2.0) - 1.0
x[-1] = x[-2]

window = numpy.exp(-x * x * 4) ** 1.5
sine = numpy.sin(8 * numpy.pi * x)
sine_fold = sine * window + numpy.arctan(3 * x) * (1 - window)
sine_fold /= numpy.abs(sine_fold).max()

power = x ** 7
overdrive = numpy.tanh(5.0 * x)
overdrive /= numpy.abs(overdrive).max()
waveforms.append(('fold_power', numpy.round(32767 * power).astype(int)))
waveforms.append(('fold_sine', numpy.round(32767 * sine_fold).astype(int)))
waveforms.append(('overdrive', numpy.round(32767 * overdrive).astype(int)))


"""----------------------------------------------------------------------------
Surprise!
----------------------------------------------------------------------------"""

digits = file('peaks/data/digits.bin', 'rb').read()
digits = map(ord, digits)
waveforms_8 = [('digits', digits)]
