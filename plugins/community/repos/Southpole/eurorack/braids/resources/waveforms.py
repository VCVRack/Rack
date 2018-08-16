#!/usr/bin/python2.5
#
# Copyright 2012 Olivier Gillet.
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


"""----------------------------------------------------------------------------
Waveforms for vowel synthesis
----------------------------------------------------------------------------"""

SAMPLE_RATE = 96000

waveforms = []
# Create amplitude modulated sine/square tables for formants.
sine_samples = []
square_samples = []
sine = numpy.sin(numpy.arange(16.0) / 16.0 * 2 * numpy.pi)
for i, x in enumerate(sine):
  gains = numpy.exp(0.184 * numpy.arange(16.0))
  gains[0] = 0
  values = gains * x * 4
  values = numpy.round(values).astype(int)
  amps = numpy.round(gains)
  if (i >= 8):
    amps = -amps
  square_samples.extend(amps.astype(int))
  sine_samples.extend(values)

waveforms.extend([
    ('formant_sine', sine_samples),
    ('formant_square', square_samples)
])


"""----------------------------------------------------------------------------
Band-limited waveforms
----------------------------------------------------------------------------"""

WAVETABLE_SIZE = 256

# The Juno-6 / Juno-60 waveforms have a brighter harmonic content, which can be
# recreated by adding to the signal a 1-pole high-pass filtered version of
# itself.
JUNINESS = 1.0


def dither(x, order=0, type=numpy.int16):
  for i in xrange(order):
    x = numpy.hstack((numpy.zeros(1,), numpy.cumsum(x)))
  x = numpy.round(x)
  for i in xrange(order):
    x = numpy.diff(x)
  if any(x < numpy.iinfo(type).min) or any(x > numpy.iinfo(type).max):
    print 'Clipping occurred!'
  x[x < numpy.iinfo(type).min] = numpy.iinfo(type).min
  x[x > numpy.iinfo(type).max] = numpy.iinfo(type).max
  return x.astype(type)


def scale(array, min=-32766, max=32766, center=True, dither_level=2):
  if center:
    array -= array.mean()
  mx = numpy.abs(array).max()
  array = (array + mx) / (2 * mx)
  array = array * (max - min) + min
  return dither(array, order=dither_level)


# Sine wave.
sine = -numpy.sin(numpy.arange(WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE) * \
    2 * numpy.pi) * 127.5 + 127.5

# Band limited waveforms.
num_zones = 15
bl_pulse_tables = []
bl_tri_tables = []

wrap = numpy.fmod(
    numpy.arange(WAVETABLE_SIZE + 1) + WAVETABLE_SIZE / 2,
    WAVETABLE_SIZE)
    
quadrature = numpy.fmod(
    numpy.arange(WAVETABLE_SIZE + 1) + WAVETABLE_SIZE / 4,
    WAVETABLE_SIZE)
    
fill = numpy.fmod(
    numpy.arange(WAVETABLE_SIZE + 1),
    WAVETABLE_SIZE)

waveforms.append(('sine', scale(sine[quadrature])))

for zone in range(num_zones):
  f0 = 440.0 * 2.0 ** ((18 + 8 * zone - 69) / 12.0)
  if zone == num_zones - 1:
    f0 = SAMPLE_RATE / 2.0 - 1
  else:
    f0 = min(f0, SAMPLE_RATE / 2.0)
  period = SAMPLE_RATE / f0
  m = 2 * numpy.floor(period / 2) + 1.0
  i = numpy.arange(-WAVETABLE_SIZE / 2, WAVETABLE_SIZE / 2) / \
      float(WAVETABLE_SIZE)
  pulse = numpy.sin(numpy.pi * i * m) / (m * numpy.sin(numpy.pi * i) + 1e-9)
  pulse[WAVETABLE_SIZE / 2] = 1.0
  pulse = pulse[fill]

  bl_pulse_tables.append(('bandlimited_comb_%d' % zone,
                          scale(pulse[quadrature])))

waveforms.extend(bl_pulse_tables)
waveforms.extend(bl_tri_tables)


"""----------------------------------------------------------------------------
Wavetables
-----------------------------------------------------------------------------"""

wavetable_data = []
waves = map(ord, file('braids/data/waves.bin', 'rb').read())
wavetable_data.append(('waves', waves))

wave_map = map(ord, file('braids/data/map.bin', 'rb').read())
wavetable_data.append(('map', wave_map))



"""----------------------------------------------------------------------------
???
-----------------------------------------------------------------------------"""

random_data = [
5, 0, 132, 0, 20, 16, 20, 81, 16, 65, 8, 17, 4, 65, 17, 5, 
0, 69, 1, 88, 17, 25, 0, 132, 144, 0, 64, 80, 0, 21, 148, 0, 
65, 17, 5, 129, 4, 1, 68, 1, 65, 5, 65, 17, 21, 4, 20, 16, 
4, 128, 80, 0, 4, 64, 20, 21, 0, 4, 1, 20, 16, 9, 17, 68, 
17, 5, 17, 4, 1, 132, 0, 65, 16, 20, 81, 145, 1, 81, 17, 21, 
16, 21, 81, 1, 20, 16, 21, 68, 16, 16, 144, 5, 0, 132, 17, 4, 
65, 68, 129, 65, 20, 81, 129, 0, 4, 20, 65, 129, 17, 5, 80, 5, 
64, 64, 1, 132, 64, 65, 17, 68, 65, 64, 17, 5, 80, 0, 4, 1, 
20, 16, 25, 64, 4, 17, 4, 20, 16, 84, 20, 128, 5, 0, 132, 0, 
65, 80, 1, 65, 24, 81, 0, 129, 80, 129, 17, 5, 64, 1, 65, 64, 
16, 64, 16, 64, 17, 5, 64, 1, 65, 17, 5, 17, 132, 5, 80, 1, 
64, 20, 1, 16, 25, 81, 0, 80, 4, 129, 16, 5, 0, 4, 20, 16, 
4, 0, 8, 16, 4, 1, 20, 81, 16, 69, 1, 8, 21, 72, 0, 80, 
4, 65, 1, 0, 80, 1, 65, 0, 64, 80, 65, 17, 4, 20, 20, 129, 
1, 4, 21, 20, 16, 132, 17, 5, 16, 8, 1, 4, 4, 64, 0, 4, 
5, 17, 21, 81, 0, 5, 128, 0, 65, 64, 17, 4, 80, 16, 68, 0, 
4, 1, 1, 89, 17, 21, 64, 80, 1, 145, 0, 68, 20, 69, 1, 88, 
17, 25, 0, 132, 16, 65, 80, 1, 81, 1, 4, 21, 16, 21, 1, 16, 
9, 21, 20, 128, 4, 0, 69, 16, 20, 16, 21, 81, 65, 8, 20, 4, 
64, 64, 1, 132, 21, 145, 1, 64, 24, 64, 16, 4, 80, 65, 1, 9, 
5, 1, 4, 65, 4, 21, 64, 1, 81, 16, 16, 144, 17, 4, 1, 4, 
5, 65, 20, 128, 80, 9, 5, 1, 4, 1, 88, 0, 64, 8, 81, 80, 
1, 81, 17, 0, 145, 17, 69, 1, 4, 4, 17, 4, 9, 21, 20, 128, 
81, 84, 17, 64, 17, 68, 16, 8, 16, 20, 81, 0, 21, 20, 128, 144, 
5, 21, 0, 4, 1, 132, 64, 129, 1, 64, 80, 1, 65, 1, 5, 1, 
1, 9, 9, 81, 64, 17, 64, 20, 68, 1, 24, 0, 4, 5, 65, 69, 
65, 1, 68, 16, 8, 81, 4, 5, 65, 64, 65, 17, 8, 64, 0, 64, 
80, 1, 68, 0, 24, 5, 85, 4, 65, 64, 80, 16, 64, 64, 17, 64, 
20, 128, 80, 65, 1, 24, 69, 21, 1, 20, 65, 4, 129, 17, 5, 65, 
1, 68, 16, 68, 1, 24, 0, 20, 81, 0, 5, 65, 1, 64, 17, 21, 
4, 1, 1, 25, 81, 20, 64, 64, 16, 65, 80, 17, 0, 145, 1, 65, 
0, 64, 20, 16, 20, 80, 64, 65, 17, 88, 0, 64, 24, 0, 4, 5, 
65, 17, 0, 145, 17, 4, 65, 4, 145, 65, 4, 65, 0, 80, 17, 5, 
80, 0, 4, 1, 1, 89, 0, 64, 4, 65, 8, 81, 80, 0, 88, 0, 
64, 64, 0, 8, 17, 133, 65, 8, 64, 80, 64, 0, 24, 1, 5, 80, 
17, 5, 8, 21, 0, 20, 81, 0, 149, 5, 0, 132, 0, 20, 16, 20, 
81, 16, 65, 24, 16, 4, 65, 17, 5, 81, 1, 20, 17, 0, 88, 0, 
64, 20, 16, 9, 5, 1, 4, 1, 8, 81, 17, 5, 65, 24, 65, 16, 
64, 80, 0, 4, 64, 4, 128, 80, 65, 1, 8, 64, 5, 5, 65, 20, 
128, 80, 25, 16, 21, 81, 0, 21, 1, 16, 9, 64, 64, 16, 64, 20, 
84, 16, 16, 144, 20, 0, 21, 16, 68, 16, 65, 9, 16, 20, 81, 16, 
8, 25, 16, 20, 81, 0, 5, 17, 4, 1, 68, 1, 0, 80, 5, 0, 
4, 65, 132, 65, 4, 5, 65, 4, 129, 5, 0, 132, 1, 20, 81, 17, 
5, 65, 17, 0, 145, 16, 5, 0, 20, 145, 16, 69, 16, 132, 20, 20, 
65, 80, 17, 68, 1, 8, 20, 8, 25, 20, 81, 0, 68, 1, 0, 80, 
16, 65, 64, 1, 65, 1, 5, 20, 20, 129, 1, 65, 17, 21, 84, 4, 
64, 21, 1, 16, 9, 64, 68, 64, 65, 17, 8, 0, 20, 81, 16, 9, 
16, 4, 5, 129, 5, 0, 68, 1, 145, 1, 65, 17, 5, 80, 16, 64, 
1, 8, 16, 4, 1, 4, 20, 16, 20, 144, 64, 9, 21, 16, 4, 65, 
17, 5, 64, 0, 88, 0, 64, 8, 65, 17, 21, 81, 81, 16, 16, 144, 
144, 0, 4, 80, 1, 20, 64, 20, 24, 16, 4, 0, 20, 81, 16, 4, 
80, 0, 24, 81, 0, 129, 16, 5, 0, 20, 81, 17, 5, 17, 4, 128, 
80, 65, 1, 24, 16, 5, 20, 0, 20, 0, 4, 1, 68, 0, 24, 0, 
4, 80, 16, 4, 64, 9, 16, 4, 65, 17, 21, 9, 25, 80, 64, 65, 
1, 24, 81, 0, 129, 16, 81, 0, 21, 80, 24, 0, 20, 81, 1, 144, 
80, 89, 0, 64, 8, 16, 4, 5, 129, 20, 20, 128, 17, 5, 16, 88, 
0, 64, 8, 65, 17, 21, 81, 81, 16, 16, 144, 4, 0, 69, 16, 20, 
16, 21, 0, 20, 81, 1, 20, 16, 25, 1, 5, 80, 64, 89, 80, 16, 
64, 1, 5, 20, 20, 65, 16, 16, 144, 5, 0, 132, 1, 64, 80, 16, 
84, 20, 20, 64, 4, 129, 5, 4, 17, 84, 17, 69, 1, 24, 0, 4, 
21, 16, 20, 80, 17, 0, 145, 16, 5, 84, 0, 128, 5, 0, 132, 1, 
4, 65, 64, 65, 1, 5, 64, 16, 16, 144, 16, 5, 0, 4, 85, 16, 
17, 65, 0, 8, 0, 4, 5, 17, 4, 17, 68, 65, 64, 65, 17, 4, 
16, 1, 24, 81, 20, 64, 64, 16, 65, 144, 16, 5, 0, 4, 4, 64, 
16, 65, 4, 65, 20, 64, 16, 16, 144, 5, 0, 4, 85, 16, 17, 65, 
0, 24, 0, 20, 16, 9, 64, 21, 81, 1, 65, 1, 5, 0, 4, 5, 
80, 0, 68, 65, 16, 16, 80, 255
]

wavetable_data.append(('code', random_data))
