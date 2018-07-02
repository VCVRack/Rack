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

WAVESHAPER_SIZE = 1024

x = numpy.arange(0, WAVESHAPER_SIZE + 1) / float(WAVESHAPER_SIZE)

linear = x
sin = (1.0 - numpy.cos(numpy.pi * x)) / 2.0
inverse_sin = numpy.arccos(1 - 2 * x) / numpy.pi
expo = 1.0 - numpy.exp(-3 * x)
expo_max = expo.max()
expo /= expo_max

expo_flipped = (1.0 - numpy.exp(-3 * (1 - x))) / expo_max
log = numpy.log(1.0 - x * expo_max) / -3.0
log_flipped = numpy.log(1.0 - (1 - x) * expo_max) / -3.0

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

