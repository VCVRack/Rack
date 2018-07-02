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


def pole_pair_to_f_fq(pole_pair):
  fq = 1 - pole_pair.prod()
  f = -(2 - fq - (pole_pair.sum())) ** 0.5
  return f.real, fq.real



def modified_chamberlin(f, fq, x, mode='bp'):
  lp = 0.0
  bp = 0.0
  y = numpy.zeros(x.shape)
  x_ = 0.0
  coefficient = 1.0 if mode == 'bp' else 0.0
  for i in xrange(len(y)):
    lp += f * bp
    bp += -fq * bp -f * lp + (x[i] + x_ * coefficient)
    x_ = x[i]
    if mode =='bp':
      y[i] = fq * bp
    elif mode == 'lp':
      y[i] = f * lp
    elif mode == 'hp':
      y[i] = x_ - lp * f - bp * fq
  return y



SAMPLE_RATE = 96000
IR_SIZE = 2048


sample_rates = [SAMPLE_RATE / 12] * 13
sample_rates += [SAMPLE_RATE / 3] * 6
sample_rates += [SAMPLE_RATE] * 1
num_bands = len(sample_rates)

interval = 2 ** (1 / 3.0)
first_frequency = 110 / interval
frequencies = first_frequency * (interval ** numpy.arange(0, num_bands))

filters = []

responses = {}
reconstruction = {}

generate_figures = __name__ == '__main__'

for index, (frequency, sr) in enumerate(zip(frequencies, sample_rates)):
  if not sr in reconstruction:
    reconstruction[sr] = [0.0, 0.0]
    responses[sr] = []
  
  frequency = frequency / (sr * 0.5)
  
  if index == 0:
    w = frequency
    z, p, k = scipy.signal.cheby1(4, 0.5, w, 'lowpass', output='zpk')
    svf_mode = 'lp'
    gain = 1.0
  elif index == num_bands - 1:
    w = frequency
    z, p, k = scipy.signal.cheby1(4, 0.25, w, 'highpass', output='zpk')
    svf_mode = 'hp'
    gain = 21 * frequency
  else:
    w = [frequency / (interval ** 0.5), frequency * (interval ** 0.5)]
    z, p, k = scipy.signal.butter(2, w, 'bandpass', output='zpk')
    svf_mode = 'bp'
    gain = 0.25
  
  # Filter using direct form
  out = numpy.eye(IR_SIZE, 1).ravel()
  b, a = scipy.signal.zpk2tf(z, p, k)
  out = scipy.signal.lfilter(b, a, out)
  out = scipy.signal.lfilter(b, a, out)
  reconstruction[sr][0] += out
  responses[sr] += [out]
  
  # Filter using modified Chamberlin filter
  out = numpy.eye(IR_SIZE, 1).ravel() * gain
  
  coefficients = [0, 0, 0]
  for i in xrange(2):
    f, fq = pole_pair_to_f_fq(p[i*2:i*2 + 2])
    out = modified_chamberlin(f, fq, out, svf_mode)
    out = modified_chamberlin(f, fq, out, svf_mode)
    coefficients += [f, fq]
  
  delay = (numpy.arange(len(out)) * out * out).sum() / (out * out).sum()
  
  # Completely empirical fixes to the delay to maximize the flatness of the
  # total impulse response.
  if index == num_bands - 1:
    delay += 4
  
  coefficients[0] = SAMPLE_RATE / sr
  coefficients[1] = numpy.floor(delay)
  coefficients[2] = gain
  
  filters += [('%3.0f_%d' % (frequency * 0.5 * sr, sr), coefficients)]
  
  reconstruction[sr][1] += out



if generate_figures:
  pylab.figure(figsize=(20,8))
  n = len(responses.keys())
  for row, sr in enumerate(sorted(responses.keys())):
    f = numpy.arange(IR_SIZE / 2 + 1) / float(IR_SIZE) * sr
    for column, plots in enumerate([reconstruction[sr], responses[sr]]):
      pylab.subplot(2, n, column * n + row + 1)
      for r in plots:
        sy = numpy.log10(numpy.abs(numpy.fft.rfft(r)) + 1e-20) * 20.0
        pylab.semilogx(f, sy)
      pylab.xlim(80, sr / 2)
      pylab.ylim(-36, 12)
      pylab.xlabel('Frequency (Hz)')
      pylab.ylabel('Gain (dB)')
      if len(plots) == 2:
        pylab.ylim(-4, 3)
        #pylab.legend(['Direct form', 'Chamberlin'])
  pylab.savefig('filter_bank.pdf')
  # pylab.show()
  pylab.close()
  