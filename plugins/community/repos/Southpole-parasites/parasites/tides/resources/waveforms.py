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

waveforms = []

"""----------------------------------------------------------------------------
Sine wave
----------------------------------------------------------------------------"""
WAVETABLE_SIZE=1024

x = numpy.arange(0, WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE)
x[-1] = x[0]
sine = numpy.sin(2 * numpy.pi * x)
waveforms.append(('sine1024', (32767 * sine).astype(int)))

WAVETABLE_SIZE=128

x = numpy.arange(0, WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE)
x[-1] = x[0]
sine = numpy.sin(2 * numpy.pi * x)
waveforms.append(('sine128', (32767 * sine).astype(int)))

WAVETABLE_SIZE=64

x = numpy.arange(0, WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE)
x[-1] = x[0]
sine = numpy.sin(2 * numpy.pi * x)
waveforms.append(('sine64', (32767 * sine).astype(int)))

WAVETABLE_SIZE=16

x = numpy.arange(0, WAVETABLE_SIZE + 1) / float(WAVETABLE_SIZE)
x[-1] = x[0]
sine = numpy.sin(2 * numpy.pi * x)
waveforms.append(('sine16', (32767 * sine).astype(int)))

"""----------------------------------------------------------------------------
Band-limited waveforms
----------------------------------------------------------------------------"""

SAMPLE_RATE = 48000.0
WAVETABLE_SIZE = 1024

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


# Band limited waveforms.
num_zones = 20
bl_parabola_tables = []

wrap = numpy.arange(WAVETABLE_SIZE + 1) + WAVETABLE_SIZE / 2
wrap = numpy.fmod(wrap, WAVETABLE_SIZE)
    
quadrature = numpy.arange(WAVETABLE_SIZE + 1) + WAVETABLE_SIZE / 4
quadrature = numpy.fmod(quadrature, WAVETABLE_SIZE)
    
fill = numpy.arange(WAVETABLE_SIZE + 1)
fill = numpy.fmod(fill, WAVETABLE_SIZE)

ref_f0_energy = None

for zone in range(num_zones):
  f0 = 440.0 * 2.0 ** ((8 + 8 * zone - 69) / 12.0)
  f0 = min(f0, SAMPLE_RATE / 2.0)
  period = SAMPLE_RATE / f0
  m = 2 * numpy.floor(period / 2) + 1.0
  i = numpy.arange(-WAVETABLE_SIZE / 2, WAVETABLE_SIZE / 2) / \
      float(WAVETABLE_SIZE)

  pulse = numpy.sin(numpy.pi * i * m) / (m * numpy.sin(numpy.pi * i) + 1e-9)
  pulse[WAVETABLE_SIZE / 2] = 1.0
  pulse = pulse[fill]
  
  square = numpy.cumsum(pulse - pulse[wrap])
  triangle = -numpy.cumsum(square[::-1] - square.mean()) / WAVETABLE_SIZE
  saw = -numpy.cumsum(pulse[wrap] - pulse.mean())
  parabola = numpy.cumsum(saw - saw.mean())
  scaled_parabola = scale(parabola[quadrature])
  f0_energy = numpy.abs(numpy.fft.rfft(scaled_parabola)[1])
  if ref_f0_energy is None:
    ref_f0_energy = f0_energy
  scaled_parabola = scaled_parabola / f0_energy * ref_f0_energy
  scaled_parabola *= min(1.0, 32767 / scaled_parabola.max())
  bl_parabola_tables.append(
      ('bandlimited_parabola_%d' % zone, scaled_parabola))

waveforms.extend(bl_parabola_tables)



"""----------------------------------------------------------------------------
Waveshaper for audio rate
----------------------------------------------------------------------------"""

WAVESHAPER_SIZE = 1024

x = numpy.arange(0, WAVESHAPER_SIZE + 1) / float(WAVESHAPER_SIZE)

linear = x
sin = (1.0 - numpy.cos(numpy.pi * x)) / 2.0
tan = numpy.arctan(8 * numpy.cos(numpy.pi * x))
scale = tan.max()
tan = (1.0 - tan / scale) / 2.0
inverse_sin = numpy.arccos(1 - 2 * x) / numpy.pi
inverse_tan = numpy.arccos(numpy.tan(scale * (1.0 - 2.0 * x)) / 8.0) / numpy.pi

def audio_rate_flip(x):
  x = numpy.array(list(-x[WAVESHAPER_SIZE:0:-1]) + list(x))
  return numpy.round((x * 32767.0)).astype(int)

audio_rate_tables = []
audio_rate_tables.append(('inverse_tan_audio', audio_rate_flip(inverse_tan)))
audio_rate_tables.append(('inverse_sin_audio', audio_rate_flip(inverse_sin)))
audio_rate_tables.append(('linear_audio', audio_rate_flip(linear)))
audio_rate_tables.append(('sin_audio', audio_rate_flip(sin)))
audio_rate_tables.append(('tan_audio', audio_rate_flip(tan)))
waveforms.extend(audio_rate_tables)



"""----------------------------------------------------------------------------
Waveshaper for control rate
----------------------------------------------------------------------------"""

WAVESHAPER_SIZE = 512

x = numpy.arange(0, WAVESHAPER_SIZE + 1) / float(WAVESHAPER_SIZE)

linear = x
sin = (1.0 - numpy.cos(numpy.pi * x)) / 2.0
inverse_sin = numpy.arccos(1 - 2 * x) / numpy.pi
inverse_sin = (((inverse_sin*2-1) ** 3)+1)*0.5 # for more contrast
expo = 1.0 - numpy.exp(-3 * x)
expo_max = expo.max()
expo = 1.0 - (1.0 - expo) ** 2  # for more contrast
expo /= expo.max()

expo_flipped = 1.0 - numpy.exp(-3 * (1 - x))
expo_flipped = 1.0 - (1.0 - expo_flipped) ** 2  # for more contrast
expo_flipped /= expo_flipped.max()
log = numpy.log(1.0 - x * expo_max) / -3.0
log -= log.min()
log /= log.max()
log = log ** 2                  # for more contrast
log_flipped = numpy.log(1.0 - (1 - x) * expo_max) / -3.0
log_flipped -= log_flipped.min()
log_flipped /= log_flipped.max()
log_flipped = log_flipped ** 2  # for more contrast

def control_rate_flip(x, y):
  x = numpy.array(list(x) + list(y[1:]))
  return numpy.round((x * 32767.0)).astype(int)

control_rate_tables = []
control_rate_tables.append(
    ('reversed_control', control_rate_flip(log, 1.0 - log)))
control_rate_tables.append(
    ('spiky_exp_control', control_rate_flip(log, log_flipped)))
control_rate_tables.append(
    ('spiky_control', control_rate_flip(inverse_sin, 1.0 - inverse_sin)))
control_rate_tables.append(
    ('linear_control', control_rate_flip(linear, 1.0 - linear)))
control_rate_tables.append(
    ('bump_control', control_rate_flip(sin, 1.0 - sin)))
control_rate_tables.append(
    ('bump_exp_control', control_rate_flip(expo, expo_flipped)))
control_rate_tables.append(
    ('normal_control', control_rate_flip(expo, 1.0 - expo)))
waveforms.extend(control_rate_tables)



"""----------------------------------------------------------------------------
Post waveshaper
----------------------------------------------------------------------------"""

WAVESHAPER_SIZE = 1024

x = numpy.arange(0, WAVESHAPER_SIZE + 1) / (WAVESHAPER_SIZE / 2.0) - 1.0
x[-1] = x[-2]
sine = numpy.sin(8 * numpy.pi * x)
window = numpy.exp(-x * x * 4) ** 2
bipolar_fold = sine * window + numpy.arctan(3 * x) * (1 - window)
bipolar_fold /= numpy.abs(bipolar_fold).max()
waveforms.append(('bipolar_fold', numpy.round(32767 * bipolar_fold)))

x = numpy.arange(0, WAVESHAPER_SIZE + 1) / float(WAVESHAPER_SIZE)
x[-1] = x[-2]
sine = numpy.sin(8 * numpy.pi * x)
window = numpy.exp(-x * x * 4) ** 2
unipolar_fold = (0.5 * sine + 2 * x) * window + numpy.arctan(4 * x) * (1 - window)
unipolar_fold /= numpy.abs(unipolar_fold).max()
waveforms.append(('unipolar_fold', numpy.round(32767 * unipolar_fold)))
