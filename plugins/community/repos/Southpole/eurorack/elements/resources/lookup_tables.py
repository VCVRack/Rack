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

lookup_tables = []
int16_lookup_tables = []
uint32_lookup_tables = []

SAMPLE_RATE = 32000.0


"""----------------------------------------------------------------------------
Sine table
----------------------------------------------------------------------------"""

WAVETABLE_SIZE = 4096
t = numpy.arange(0.0, WAVETABLE_SIZE + 1) / WAVETABLE_SIZE
t[-1] = t[0]

x = numpy.sin(2 * numpy.pi * t)
lookup_tables += [('sine', x)]



"""----------------------------------------------------------------------------
Coefficients for approximate filter, 32Hz to 16kHz ; Q = 0.5 to 500
----------------------------------------------------------------------------"""

frequency = 32 * (10 ** (2.7 * numpy.arange(0, 257) / 256))
frequency /= SAMPLE_RATE

frequency[frequency >= 0.499] = 0.499

g = numpy.tan(numpy.pi * frequency)
r = 2.0
h = 1.0 / (1.0 + r * g + g * g)
gain = (0.42 / frequency) * (4 ** (frequency * frequency))
r = 1 / (0.5 * 10 ** (3.0 * numpy.arange(0, 257) / 256))

lookup_tables += [
  ('approx_svf_gain', gain),
  ('approx_svf_g', g),
  ('approx_svf_r', r),
  ('approx_svf_h', h)]



"""----------------------------------------------------------------------------
Exponentials covering several decades in 256 steps, with safeguard
----------------------------------------------------------------------------"""

x = numpy.arange(0, 257) / 256.0
lookup_tables += [('4_decades', 10 ** (4 * x))]



"""----------------------------------------------------------------------------
3dB/V table for accent/strength control
----------------------------------------------------------------------------"""

x = numpy.arange(0, 257) / 256.0
coarse = 10 ** (1.5 * (x - 0.5))
fine = 10 ** (x * numpy.log10(coarse[1] / coarse[0]))
lookup_tables += [('accent_gain_coarse', coarse)]
lookup_tables += [('accent_gain_fine', fine)]



"""----------------------------------------------------------------------------
dB brightness table
----------------------------------------------------------------------------"""

x = numpy.arange(0, 513) / 512.0
x[0] = x[1]
x[-1] = x[-2]
brightness = (9 + numpy.log2(x)) / 9
int16_lookup_tables += [('db_led_brightness', brightness * 256.0)]



"""----------------------------------------------------------------------------
Stiffness table.
----------------------------------------------------------------------------"""

geometry = numpy.arange(0, 257) / 256.0
stiffness = geometry + 0
for i, g in enumerate(geometry):
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
Envelope increments and curves.
----------------------------------------------------------------------------"""

TABLE_SIZE = 256
t = numpy.arange(0.0, TABLE_SIZE + 2) / TABLE_SIZE
t[-1] = t[-2] = 1.0

control_rate = SAMPLE_RATE / 16.0
max_time = 8.0  # seconds
min_time = 0.0005
gamma = 0.175
min_increment = 1.0 / (max_time * control_rate)
max_increment = 1.0 / (min_time * control_rate)
a = numpy.power(max_increment, -gamma)
b = numpy.power(min_increment, -gamma)

lookup_tables.append(
    ('env_increments', numpy.power(a + (b - a) * t, -1 / gamma))
)

env_linear = t
env_quartic = t ** 3.32
env_expo = 1.0 - numpy.exp(-4 * t)

lookup_tables.append(('env_linear', env_linear / env_linear.max()))
lookup_tables.append(('env_expo', env_expo / env_expo.max()))
lookup_tables.append(('env_quartic', env_quartic / env_quartic.max()))



"""----------------------------------------------------------------------------
MIDI to normalized frequency table.
----------------------------------------------------------------------------"""

TABLE_SIZE = 256

midi_note = numpy.arange(0, TABLE_SIZE) - 48
frequency = 440 * 2 ** ((midi_note - 69) / 12.0)
max_frequency = min(12000, SAMPLE_RATE / 2)
frequency[frequency >= max_frequency] = max_frequency
frequency /= SAMPLE_RATE

semitone = 2 ** (numpy.arange(0, TABLE_SIZE) / 256.0 / 12.0)


lookup_tables.append(('midi_to_f_high', frequency))
lookup_tables.append(('midi_to_increment_high', frequency * (1 << 32)))
lookup_tables.append(('midi_to_f_low', semitone))



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
Quantizer for FM frequencies.
----------------------------------------------------------------------------"""

detune_ratios = [-24, -12, -11.95, -5.0, -0.05, 0.0, 0.05, 7.0, 12.0, 19.0, 24.0]

scale = []
for ratio in detune_ratios:
  scale.extend([ratio, ratio, ratio])

target_size = int(2 ** numpy.ceil(numpy.log2(len(scale))))
while len(scale) < target_size:
  gap = numpy.argmax(numpy.diff(scale))
  scale = scale[:gap + 1] + [(scale[gap] + scale[gap + 1]) / 2] + \
      scale[gap + 1:]

scale.append(scale[-1])

lookup_tables.append(
    ('detune_quantizer', scale)
)



"""----------------------------------------------------------------------------
Delay compensation factor for SVF
----------------------------------------------------------------------------"""

ratio = 2.0 ** (numpy.arange(0, 257) / 12.0)
svf_shift = 2.0 * numpy.arctan(1.0 / ratio) / (2.0 * numpy.pi)
lookup_tables += [('svf_shift', svf_shift)]