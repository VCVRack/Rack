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

import numpy as np
import pylab
import scipy.signal

lookup_tables = []

"""----------------------------------------------------------------------------
Sine table.
----------------------------------------------------------------------------"""

size = 1024
t = np.arange(0, size + size / 4 + 1) / float(size) * np.pi * 2
lookup_tables.append(('sin', np.sin(t)))

"""----------------------------------------------------------------------------
Arcsine table.
----------------------------------------------------------------------------"""

size = 256
t = np.arange(-size/2, size/2+1) / float(size)*2
lookup_tables.append(('arcsin', np.arcsin(t)*2/np.pi))

"""----------------------------------------------------------------------------
XFade table
----------------------------------------------------------------------------"""

size = 257
t = np.arange(0, size) / float(size)
t = 1.04 * t - 0.02
t[t < 0] = 0
t[t >= 1] = 1
t *= np.pi / 2
lookup_tables.append(('xfade_in', np.sin(t) * (2 ** -0.5)))
lookup_tables.append(('xfade_out', np.cos(t) * (2 ** -0.5)))



"""----------------------------------------------------------------------------
Wavefolder LUT.
----------------------------------------------------------------------------"""

WAVETABLE_SIZE = 4096
t = np.arange(0.0, WAVETABLE_SIZE + 1) / WAVETABLE_SIZE

# Waveshape based on Tides
# x = 2 * t - 1
# x = x * 8.0
# x = x / (1 + np.abs(x))
# sine = np.sin(8 * np.pi * x)
# window = np.exp(-x * x * 4) ** 2
# bipolar_fold = sine * window + np.arctan(3 * x) * (1 - window)
# bipolar_fold /= np.abs(bipolar_fold).max()
# bipolar_fold *= 0.8

x = 2 * t - 1
central_bump = np.exp(-x * x * 4) ** 2
x = x * central_bump + (1 - central_bump) * ((t + t ** 1.3) - 1)
x = x * 8.0
x = x / (1 + np.abs(x))

timbre = np.fromfile('warps/resources/timbre.raw', dtype=np.float32)
bipolar_fold = []
for value in x:
  value = (value + 1) * 0.5 * 4095
  value_integral = int(value)
  value_fractional = value - value_integral
  a = timbre[value_integral]
  b = timbre[value_integral + 1]
  bipolar_fold.append(a + (b - a) * value_fractional)
bipolar_fold = np.array(bipolar_fold) * 2.2
lookup_tables += [('bipolar_fold', bipolar_fold)]



"""----------------------------------------------------------------------------
MIDI to normalized frequency table.
----------------------------------------------------------------------------"""

SAMPLE_RATE = 96000

TABLE_SIZE = 256

midi_note = np.arange(0, TABLE_SIZE) - 48
frequency = 440 * 2 ** ((midi_note - 69) / 12.0)
max_frequency = min(12000, SAMPLE_RATE / 2)
frequency[frequency >= max_frequency] = max_frequency
frequency /= SAMPLE_RATE

semitone = 2 ** (np.arange(0, TABLE_SIZE) / 256.0 / 12.0)

lookup_tables.append(('midi_to_f_high', frequency))
lookup_tables.append(('midi_to_f_low', semitone))



"""----------------------------------------------------------------------------
Potentiometer compensation.
----------------------------------------------------------------------------"""

source = [0, 0.006, 0.014, 0.16, 0.33, 0.5, 0.67, 0.84, 0.96, 0.994, 1.00]
dest = [0, 0.1, 0.125, 0.25, 0.375, 0.5, 0.625, 0.750, 0.875, 0.9, 1.00]
n = len(source) - 1
pot_curve = []
for i in xrange(513):
  frac = i / 512.0
  n = 0
  while (frac > source[n + 1]):
    n += 1
  position = (frac - source[n]) / (source[n + 1] - source[n])
  x = dest[n] + position * (dest[n + 1] - dest[n])
  pot_curve += [x]

lookup_tables.append(('pot_curve', pot_curve))



"""----------------------------------------------------------------------------
Allpass filter network for phase shifter.
----------------------------------------------------------------------------"""

BANDWIDTH = 0.495
ORDER = 17
PASSBAND = [10.0, 20000.0]
FFT_SIZE = 65536



def iir_lpf_poles(cutoff, order, warp=True):
  wp = cutoff * np.pi
  ws = np.pi - wp

  # C. Britton Rorabaugh, "Digital Filter Designer's Handbook", p. 94
  k = np.tan(0.5 * wp) / np.tan(0.5 * ws) if warp else wp / ws
  u = 0.5 * (1 - (1 - k ** 2) ** 0.25) / (1 + (1 - k ** 2) ** 0.25)
  q = u + 2 * u ** 5 + 15 * u ** 9 + 150 * u ** 13

  poles = []
  for i in xrange((order - 1) / 2):
    w = (i + 1) * np.pi / order
    num = np.sum(((-1) ** m) * q ** (m * (m + 1)) * np.sin((2 * m + 1) * w) for m in xrange(0, 7))
    den = np.sum(((-1) ** m) * q ** (m * m) * np.cos(2 * m * w) for m in xrange(1, 7))
    l = 2 * q ** 0.25 * num / (1 + 2 * den)
    b = ((1 - k * l ** 2) * (1 - l ** 2 / k)) ** 0.5
    c = 2 * b / (1 + l ** 2)
    poles += [(2 - c) / (2 + c)]
  return poles



def warped_lp_to_ap(lp_poles, band):
  w = np.pi * band
  beta = (np.tan(w[0]) * np.tan(w[1])) ** 0.5
  b = (beta - 1) / (beta + 1)
  ap_poles = []
  # Create array of AP poles and do frequency warping.
  for pole in lp_poles:
    pole = np.sqrt(pole)
    conjugates = [-pole, pole] if pole != 0.0 else [pole]
    ap_poles += [(-c + b) / (-c * b + 1) for c in conjugates]
  return -np.sort(ap_poles)



def phase_shift(x, y):
  xf = np.fft.rfft(x)
  yf = np.fft.rfft(y)
  f = np.arange(0, FFT_SIZE / 2 + 1) / float(FFT_SIZE) * SAMPLE_RATE
  error = np.mod(np.angle(yf / xf) + 2 * np.pi, 2 * np.pi)
  return f, error / np.pi * 180



def iq_decomposition(ap_poles, x=np.eye(FFT_SIZE, 1).ravel()):
  x = [x, x + 0]
  for i, b in enumerate(ap_poles):
    x[i & 1] = scipy.signal.lfilter([-b, 1], [1, -b], x[i & 1])
  return x[0], x[1]



# Coefficients taken from Sean Costello's "hilbert" Csound opcode.
UGSC_FREQS = [0.3609, 1.2524, 2.7412, 5.5671, 11.1573, 22.3423, 44.7581,
              89.6271, 179.6242, 364.7914, 798.4578, 2770.1114]
UGSC_FREQS = np.array(UGSC_FREQS) * 15 * np.pi / SAMPLE_RATE
UGSC_POLES = [(1 - alpha) / (1 + alpha) for alpha in UGSC_FREQS]

# Coefficients computed from the all-pass decomposition of an elliptic halfband
# filter.
AP_POLES = warped_lp_to_ap(
    iir_lpf_poles(BANDWIDTH, ORDER) + [0.0],
    np.array(PASSBAND) / SAMPLE_RATE)

if __name__ == '__main__':
  f, error_1 = phase_shift(*iq_decomposition(UGSC_POLES))
  _, error_2 = phase_shift(*iq_decomposition(AP_POLES))
  pylab.semilogx(f, error_1, 'r')
  pylab.semilogx(f, error_2, 'g')
  pylab.xlim([2, 48000])
  pylab.xlabel('Frequency (Hz)')
  pylab.ylim([90 * 0.99, 90 * 1.01])
  pylab.ylabel('Phase shift (degrees)')
  pylab.show()

lookup_tables.append(('ap_poles', AP_POLES))
