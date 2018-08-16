#!/usr/bin/python2.5
#
# Copyright 2012 Olivier Gillet.
#
# Author: Olivier Gillet (olivier@mutable-instruments.net)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# -----------------------------------------------------------------------------

import numpy

waveforms = []

SAMPLE_RATE = float(32000000 / 666)

WAVETABLE_SIZE = 512

def Dither(x, order=0, type=numpy.uint8):
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


def Scale(array, min=1, max=254, center=True, dither=2):
  if center:
    array -= array.mean()
  mx = numpy.abs(array).max()
  array = (array + mx) / (2 * mx)
  array = array * (max - min) + min
  return Dither(array, order=dither)


# Sine wave.
numpy.random.seed(21)
sine = -numpy.sin(numpy.arange(WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE) * 2 * numpy.pi)

# Band limited waveforms.
num_zones = 8
bl_tri_tables = []
bl_ntri_tables = []

fill = numpy.fmod(numpy.arange(WAVETABLE_SIZE + 1), WAVETABLE_SIZE)
wrap = numpy.fmod(numpy.arange(WAVETABLE_SIZE + 1) + WAVETABLE_SIZE / 2, WAVETABLE_SIZE)
quadrature = numpy.fmod(numpy.arange(WAVETABLE_SIZE + 1) + WAVETABLE_SIZE / 4, WAVETABLE_SIZE)
step = numpy.fmod(numpy.arange(WAVETABLE_SIZE + 1) + WAVETABLE_SIZE / 32, WAVETABLE_SIZE)

for zone in range(num_zones):
  f0 = 440.0 * 2.0 ** ((18 + 16 * zone - 69) / 12.0)
  period = SAMPLE_RATE / f0
  m = 2 * numpy.floor(period / 2) + 1.0
  i = numpy.arange(-WAVETABLE_SIZE / 2, WAVETABLE_SIZE / 2) / float(WAVETABLE_SIZE)
  pulse = numpy.sin(numpy.pi * i * m) / (m * numpy.sin(numpy.pi * i) + 1e-9)
  pulse[WAVETABLE_SIZE / 2] = 1.0
  pulse = pulse[fill]

  square = numpy.cumsum(pulse - pulse[wrap])
  triangle = -numpy.cumsum(square[::-1] - square.mean()) / WAVETABLE_SIZE
  
  nes_triangle = 0
  for i in xrange(32):
    nes_triangle += (1 if i < 16 else -1) * pulse
    pulse = pulse[step]
  nes_triangle = -numpy.cumsum(nes_triangle)

  triangle = triangle[quadrature]
  if zone >= num_zones - 2:
    triangle = sine
  bl_tri_tables.append(('bandlimited_triangle_%d' % zone,
                        Scale(triangle[quadrature])))

  nes_triangle = nes_triangle[quadrature]
  if zone >= num_zones - 2:
    nes_triangle = sine
  bl_ntri_tables.append(('bandlimited_nes_triangle_%d' % zone,
                        Scale(nes_triangle[quadrature])))

waveforms.extend(bl_tri_tables)
waveforms.extend(bl_ntri_tables)

