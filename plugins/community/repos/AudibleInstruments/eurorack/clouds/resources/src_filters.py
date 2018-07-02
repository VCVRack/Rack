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
import scipy.signal

SAMPLE_RATE = 32000
CRITICAL_FREQUENCY = 12000.0
IR_LENGTH = 8192

filters = []

ratios = [2]
lengths = [31, 45, 63, 91]

configurations = []
for oversampled in [False]:
  for ratio in ratios:
    for length in lengths:
      configurations += [(oversampled, ratio, length)]

generate_figures = False
for oversampled, ratio, length in configurations:
  transition = 0.15 * 0.25 ** (length / 100.0) if oversampled else 0.4
  ir = scipy.signal.remez(
      length,
      [0, transition / ratio, 0.499 / ratio, 0.5], [1, 0])
  ir /= sum(ir)
  gain = 20 * numpy.log10(numpy.abs(numpy.fft.rfft(ir, IR_LENGTH)))
  f = numpy.arange(IR_LENGTH / 2 + 1) / float(IR_LENGTH) * SAMPLE_RATE * ratio
  bin_index = CRITICAL_FREQUENCY / ratio / SAMPLE_RATE * IR_LENGTH
  critical_gain = gain[int(round(bin_index))]

  name = 'filter_%s_%d_%d' % ('2x' if oversampled else '1x', ratio, length)
  if generate_figures:
    pylab.figure(figsize=(8, 12))
    pylab.subplot(211)
    pylab.plot(f, gain)
    pylab.xlim([0.0, SAMPLE_RATE])
    pylab.ylim([-75.0, 3.0])
    pylab.xlabel('Frequency (Hz)')
    pylab.ylabel('Gain (dB)')
    caption = 'Resampling filter, N=%d, Ratio=%d. Gain at %d Hz = %.2fdB'
    pylab.title(caption % (length, ratio, CRITICAL_FREQUENCY, critical_gain))
  
    pylab.subplot(212)
    pylab.plot(f, gain)
    pylab.xlim([0.0, 20000.0])
    pylab.ylim([-3.0, 1.0])
    pylab.xlabel('Frequency (Hz)')
    pylab.ylabel('Gain (dB)')
  
    pylab.savefig(name + '.pdf')
    pylab.close()
  filters += [(name, ir)]
