#!/usr/bin/python2.5
#
# Copyright 2016 Olivier Gillet.
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

import scipy.signal
import numpy
import pylab

lookup_tables = []
lookup_tables_i16 = []
lookup_tables_i8 = []


"""----------------------------------------------------------------------------
Sine table
----------------------------------------------------------------------------"""

WAVETABLE_SIZE = 1024
t = numpy.arange(0.0, WAVETABLE_SIZE + WAVETABLE_SIZE / 4 + 1) / WAVETABLE_SIZE
x = numpy.sin(2 * numpy.pi * t)
lookup_tables += [('sine', x)]



"""----------------------------------------------------------------------------
Quantizer for FM frequencies.
----------------------------------------------------------------------------"""

fm_frequency_ratios = [ 0.5, 0.5 * 2 ** (16 / 1200.0),
  numpy.sqrt(2) / 2, numpy.pi / 4, 1.0, 1.0 * 2 ** (16 / 1200.0), numpy.sqrt(2),
  numpy.pi / 2, 7.0 / 4, 2, 2 * 2 ** (16 / 1200.0), 9.0 / 4, 11.0 / 4,
  2 * numpy.sqrt(2), 3, numpy.pi, numpy.sqrt(3) * 2, 4, numpy.sqrt(2) * 3,
  numpy.pi * 3 / 2, 5, numpy.sqrt(2) * 4, 8]

scale = []
for ratio in fm_frequency_ratios:
  ratio = 12 * numpy.log2(ratio)
  scale.extend([ratio, ratio, ratio])

target_size = int(2 ** numpy.ceil(numpy.log2(len(scale))))
while len(scale) < target_size:
  gap = numpy.argmax(numpy.diff(scale))
  scale = scale[:gap + 1] + [(scale[gap] + scale[gap + 1]) / 2] + \
      scale[gap + 1:]

scale.append(scale[-1])

lookup_tables.append(
    ('fm_frequency_quantizer', scale)
)



"""----------------------------------------------------------------------------
Waveshaper tables stolen from Tides
----------------------------------------------------------------------------"""

WAVESHAPER_SIZE = 128

x = numpy.arange(0, WAVESHAPER_SIZE + 1) / float(WAVESHAPER_SIZE)

linear = x
tan = numpy.arctan(8 * numpy.cos(numpy.pi * x))
scale = tan.max()
fade_crop = numpy.minimum(1.0, 4.0 - 4.0 * x)
bump = (1.0 - numpy.cos(numpy.pi * x * 1.5)) * (1.0 - numpy.cos(numpy.pi * fade_crop)) / 4.5
double_bump = numpy.sin(numpy.pi * x * 1.5)
inverse_sin = numpy.arccos(1 - 2 * x) / numpy.pi
inverse_tan = numpy.arccos(numpy.tan(scale * (1.0 - 2.0 * x)) / 8.0) / numpy.pi

def flip(x):
  x = numpy.array(list(-x[WAVESHAPER_SIZE:0:-1]) + list(x))
  return numpy.round((x * 32767.0)).astype(int)

lookup_tables_i16.append(('ws_inverse_tan', flip(inverse_tan)))
lookup_tables_i16.append(('ws_inverse_sin', flip(inverse_sin)))
lookup_tables_i16.append(('ws_linear', flip(linear)))
lookup_tables_i16.append(('ws_bump', flip(bump)))
lookup_tables_i16.append(('ws_double_bump', flip(double_bump)))
lookup_tables_i16.append(('ws_double_bump_sentinel', flip(double_bump)))

WAVESHAPER_SIZE = 512

x = numpy.arange(0, WAVESHAPER_SIZE + 4) / (WAVESHAPER_SIZE / 2.0) - 1.0
x[-1] = x[-2]
sine = numpy.sin(8 * numpy.pi * x)
window = numpy.exp(-x * x * 4) ** 2
bipolar_fold = sine * window + numpy.arctan(3 * x) * (1 - window)
bipolar_fold /= numpy.abs(bipolar_fold).max()
lookup_tables.append(('fold', bipolar_fold))


def deadband(v_in, r_in, r_load, r_fb=150.0e3, v_sat=10.47):
  v_out = -r_fb / r_in * v_in
  v_out[v_out >= v_sat] = v_sat
  v_out[v_out < -v_sat] = -v_sat
  return (v_in * r_fb * r_load + v_out * r_in * r_load) / (r_fb * r_load + r_in * r_load + r_in * r_fb)


def diode(x):
  return 0.7 * x / (0.3 + numpy.abs(x))

v_in = x * 12.0
stage_1 = deadband(v_in, 10e3, 100e3)
stage_2 = deadband(v_in, 49.9e3, 44.2e3)
stage_3 = deadband(v_in, 91e3, 18e3)
stage_4 = deadband(v_in, 30e3, 71.4e3)
stage_5 = deadband(v_in, 68e3, 33.0e3)

stage_45 = -33.0 / 71.4 * stage_4 -33.0 / 33.0 * stage_5 - 33.0 / 240.0 * v_in
v_out = diode(-150.0 / 100.0 * stage_1 -150.0 / 44.2 * stage_2 - 150.0 / 18.0 * stage_3 - 150.0 / 33.0 * stage_45)

lookup_tables.append(('fold_2', v_out / v_out.max()))


"""----------------------------------------------------------------------------
Stiffness table (partial detuning) for modal synthesis and additive synthesis.
----------------------------------------------------------------------------"""

structure = numpy.arange(0, 65) / 64.0
stiffness = structure + 0
for i, g in enumerate(structure):
  if g < 0.25:
    g = 0.25 - g
    stiffness[i] = -g * 0.25
  elif g < 0.3:
    stiffness[i] = 0.0
  elif g < 0.9:
    g -= 0.3
    g /= 0.6
    stiffness[i] = 0.01 * 10 ** (g * 2.005) - 0.01
  else:
    g -= 0.9
    g /= 0.1
    g *= g
    stiffness[i] = 1.5 - numpy.cos(g * numpy.pi) / 2.0

stiffness[-1] = 2.0
stiffness[-2] = 2.0
lookup_tables += [('stiffness', stiffness)]



"""----------------------------------------------------------------------------
Delay compensation factor for SVF
----------------------------------------------------------------------------"""

ratio = 2.0 ** (numpy.arange(0, 257) / 12.0)
svf_shift = 1.0 - 2.0 * numpy.arctan(1.0 / ratio) / (2.0 * numpy.pi)
lookup_tables += [('svf_shift', svf_shift)]



"""----------------------------------------------------------------------------
Excitation signal for LPC speech synthesis.
32x interpolated (band-limited interpolation), phase adjusted for maximum
compacity in the time-domain
----------------------------------------------------------------------------"""

excitation_pulse = numpy.array([
    42, -44,  50, -78,  18,  37,  20,   2,
   -31, -59,   2,  95,  90,   5,  15,  38,
    -4, -91, -91, -42, -35, -36,  -4,  37,
    43,  34,  33,  15,  -1,  -8, -18, -19,
   -17,  -9, -10,  -6,   0,   3,   2,   1])


def minimum_phase_reconstruction(signal):
  fft_size = len(signal)
  Xf = numpy.fft.fft(signal, fft_size)
  real_cepstrum = numpy.fft.ifft(numpy.log(1e-50 + numpy.abs(Xf))).real
  real_cepstrum[1:fft_size / 2] *= 2
  real_cepstrum[fft_size / 2 + 1:] = 0
  min_phi = numpy.fft.ifft(numpy.exp(numpy.fft.fft(real_cepstrum))).real
  return min_phi

n = len(excitation_pulse)
target_length = 20
ratio = 32

pulse_upsampled = numpy.fft.irfft(numpy.fft.rfft(excitation_pulse), ratio * n)

pulse_upsampled = minimum_phase_reconstruction(
  pulse_upsampled * numpy.kaiser(n * ratio, 1))[:ratio * target_length]
pulse_upsampled -= pulse_upsampled[0]
pulse_upsampled[-ratio * 4:] *= numpy.linspace(1, 0, ratio * 4)
pulse_upsampled /= pulse_upsampled.max()

lookup_tables_i8 += [('lpc_excitation_pulse', numpy.round(pulse_upsampled * 127))]
