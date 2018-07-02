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
# Lookup table definitions.

import numpy

lookup_tables = []
lookup_tables_signed = []
lookup_tables_32 = []

sample_rate = 96000
excursion = 65536 * 65536.0

# Create table for pitch.
a4_midi = 69
a4_pitch = 440.0
highest_octave = 128
notes = numpy.arange(
    highest_octave * 128.0,
    (highest_octave + 12) * 128.0 + 16,
    16)
pitches = a4_pitch * 2 ** ((notes - a4_midi * 128) / (128 * 12))
increments = excursion / sample_rate * pitches
delays = sample_rate / pitches * 65536 * 4096

lookup_tables_32.append(
    ('oscillator_increments', increments.astype(int)))

lookup_tables_32.append(
    ('oscillator_delays', delays.astype(int)))



"""----------------------------------------------------------------------------
Resonator coefficients
----------------------------------------------------------------------------"""

cutoff = 440.0 * 2 ** ((numpy.arange(0, 129) - 69) / 12.0)
f = cutoff / (sample_rate / 2)
max_resonance = 0.99985
f[f > 0.25] = 0.25
bandpass_coeff_1 = -2 * numpy.cos(2 * numpy.pi * f)
bandpass_coeff_gain = []

for f in list(f):
  sample = 1.0
  y1 = 0.0
  y2 = 0.0
  n = numpy.arange(2000)
  response = numpy.sin((n + 1) * 2 * numpy.pi * f) / numpy.sin(2 * numpy.pi * f)
  response *= max_resonance ** n
  response /= (2 * f) ** 0.5
  gain = numpy.abs(response).max()
  bandpass_coeff_gain.append(gain)

bandpass_coeff_gain = numpy.maximum(1,
    numpy.minimum(
        256,
          16384.0 / numpy.array(bandpass_coeff_gain)))

lookup_tables.append(
    ('resonator_coefficient', -bandpass_coeff_1 * 32768.0)
)

lookup_tables.append(
    ('resonator_scale', bandpass_coeff_gain)
)


"""----------------------------------------------------------------------------
SVF coefficients
----------------------------------------------------------------------------"""

cutoff = 440.0 * 2 ** ((numpy.arange(0, 257) - 69) / 12.0)
f = cutoff / sample_rate
f[f > 1 / 8.0] = 1 / 8.0
f = 2 * numpy.sin(numpy.pi * f)
resonance = numpy.arange(0, 257) / 260.0
damp = numpy.minimum(2 * (1 - resonance ** 0.25),
       numpy.minimum(2, 2 / f - f * 0.5))

lookup_tables.append(
    ('svf_cutoff', f * 32767.0)
)

lookup_tables.append(
    ('svf_damp', damp * 32767.0)
)

lookup_tables.append(
    ('svf_scale', ((damp / 2) ** 0.5) * 32767.0)
)


"""----------------------------------------------------------------------------
Envelope for granular synthesis
----------------------------------------------------------------------------"""

granular_envelope = list(numpy.hanning(257) * 32767.0)
granular_envelope += [0] * 256
lookup_tables.append(
    ('granular_envelope', granular_envelope)
)

granular_envelope_rate = 2 ** (numpy.arange(0, 257) / 64.0) * (1 << 14)
lookup_tables.append(
    ('granular_envelope_rate', granular_envelope_rate / 8)
)


"""----------------------------------------------------------------------------
Bowing envelope and friction curve
----------------------------------------------------------------------------"""

attack = numpy.linspace(0, 1, int(sample_rate * 0.025 / 4)) * 0.2 * 32768
decay = numpy.linspace(1, 0.8, int(sample_rate * 0.005 / 4)) * 0.2 * 32768
bowing_envelope = list(attack) + list(decay)
# Add a guard to factor the border check out of the sample block loop
bowing_envelope += [decay[-1]] * 32
lookup_tables.append(
    ('bowing_envelope', bowing_envelope)
)

delta = numpy.arange(0, 257) / 64.0
friction = 1 / ((numpy.abs(delta) + 0.75) ** 4)
friction = numpy.minimum(friction, 1.0)
lookup_tables.append(
    ('bowing_friction', friction * 32768.0)
)

attack = numpy.linspace(0, 1, int(sample_rate * 0.005 / 4)) * 1.3 * 16384
decay = numpy.linspace(1, 0.8, int(sample_rate * 0.01 / 4)) * 1.3 * 16384
blowing_envelope = list(attack) + list(decay)
# Add a guard to factor the border check out of the sample block loop
blowing_envelope += [decay[-1]] * 32
lookup_tables.append(
    ('blowing_envelope', blowing_envelope)
)

delta = numpy.arange(0, 257) / 128.0
jet = delta ** 3 - delta
jet = numpy.minimum(jet, 1.0)
lookup_tables_signed.append(
    ('blowing_jet', jet * 32767.0)
)

flute_body_filter = 4096 * numpy.minimum(
    0.7, 0.4 * 2 ** ((numpy.arange(0, 128) - 69) / 12.0))
lookup_tables.append(
    ('flute_body_filter', flute_body_filter)
)


"""----------------------------------------------------------------------------
Quantizer for FM frequencies.
----------------------------------------------------------------------------"""

fm_frequency_ratios = [ 0.125, 0.25, 0.5, 0.5 * 2 ** (16 / 1200.0),
  numpy.sqrt(2) / 2, numpy.pi / 4, 1.0, 1.0 * 2 ** (16 / 1200.0), numpy.sqrt(2),
  numpy.pi / 2, 7.0 / 4, 2, 2 * 2 ** (16 / 1200.0), 9.0 / 4, 11.0 / 4,
  2 * numpy.sqrt(2), 3, numpy.pi, numpy.sqrt(3) * 2, 4, numpy.sqrt(2) * 3,
  numpy.pi * 3 / 2, 5, numpy.sqrt(2) * 4, 8]

scale = []
for ratio in fm_frequency_ratios:
  ratio = 256 * 12 * numpy.log2(ratio) + 16384
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
Simulates VCO detuning
----------------------------------------------------------------------------"""

modified_pitch = []

for i in xrange(257):
  frequency = 440 * 2 ** ((i / 2.0 - 69) / 12.0)
  
  # Simulates an offset current in the integrator.
  frequency -= 0.6
  
  # Simulates the integrator cap reset time.
  time = 1 / frequency
  time += 9e-6
  frequency = 1 / time
  
  midi_pitch = 128 * (69 + 12 * numpy.log2(frequency / 440.0))
  midi_pitch = max(midi_pitch, 0)
  modified_pitch.append(midi_pitch)

modified_pitch = numpy.array(modified_pitch)
modified_pitch += (60 << 7) - modified_pitch[120]

lookup_tables.append(
    ('vco_detune', modified_pitch)
)


"""----------------------------------------------------------------------------
Bell envelopes for VOSIM and FOF
----------------------------------------------------------------------------"""

def bell(size, ratio):
  n = size / ratio
  first_half = numpy.hanning(n * 2)[:n] * 65535
  r = size - n
  second_half = numpy.hanning(r * 2)[r:] * 65535
  bell = list(first_half) + list(second_half) + [0]
  return bell

lookup_tables.append(('bell', bell(256, 16)))


"""----------------------------------------------------------------------------
Envelope increments.
----------------------------------------------------------------------------"""

sample_rate = 48000
control_rate = sample_rate / 24.0
max_time = 12.0  # seconds
min_time = 3.0 / control_rate  # seconds
gamma = 0.175
min_increment = excursion / (max_time * control_rate)
max_increment = excursion / (min_time * control_rate)
rates = numpy.linspace(numpy.power(max_increment, -gamma),
                       numpy.power(min_increment, -gamma), 128)
values = numpy.power(rates, -1/gamma).astype(int)
lookup_tables_32.append(
    ('env_portamento_increments', values)
)


"""----------------------------------------------------------------------------
Envelope curves
-----------------------------------------------------------------------------"""

env_linear = numpy.arange(0, 257.0) / 256.0
env_expo = 1.0 - numpy.exp(-4 * env_linear)

lookup_tables.append(('env_expo', env_expo / env_expo.max() * 65535.0))
