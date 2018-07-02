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

"""----------------------------------------------------------------------------
LFO and envelope increments.
----------------------------------------------------------------------------"""

lookup_tables = []
lookup_tables_32 = []

sample_rate = 31089
excursion = 1 << 32
num_values = 257

# Create lookup table for envelope times (x^0.25).
max_time = 12.0  # seconds
min_time = 0.0005
gamma = 0.25
min_increment = excursion / (max_time * sample_rate)
max_increment = excursion / (min_time * sample_rate)

rates = numpy.linspace(numpy.power(max_increment, -gamma),
                       numpy.power(min_increment, -gamma), num_values)

values = numpy.power(rates, -1/gamma).astype(int)
lookup_tables_32.append(
    ('env_increments', values)
)



"""----------------------------------------------------------------------------
Envelope curves
-----------------------------------------------------------------------------"""

env_linear = numpy.arange(0, 257.0) / 256.0
env_linear[-1] = env_linear[-2]
env_quartic = env_linear ** 3.32
env_expo = 1.0 - numpy.exp(-4 * env_linear)

lookup_tables.append(('env_linear', env_linear / env_linear.max() * 65535.0))
lookup_tables.append(('env_expo', env_expo / env_expo.max() * 65535.0))
lookup_tables.append(('env_quartic', env_quartic / env_quartic.max() * 65535.0))
lookup_tables.append(('square_root', (env_linear ** 0.5) * 65535.0))



"""----------------------------------------------------------------------------
SVF coefficients
----------------------------------------------------------------------------"""

cutoff = 440.0 * 2 ** ((numpy.arange(0, 257) - 69) / 12.0)
f = cutoff / sample_rate
f[f > 1 / 8.0] = 1 / 8.0
f = 2 * numpy.sin(numpy.pi * f)
resonance = numpy.arange(0, 257) / 257.0
damp = numpy.minimum(2 * (1 - resonance ** 0.25),
       numpy.minimum(2, 2 / f - f * 0.5))

lookup_tables.append(
    ('svf_cutoff', f * 32767.0)
)

lookup_tables.append(
    ('svf_damp', damp * 32767.0)
)



"""----------------------------------------------------------------------------
Vactrol attack/decay time
----------------------------------------------------------------------------"""

vactrol_time = 0.001 * 10 ** (numpy.arange(0, 128 * 5) / 128.0)
vactrol_time[0] = 0.0001
vactrol_time[1] = 0.0002
vactrol_time[2] = 0.0005
vactrol_time[3] = 0.001
filter_coefficients = 1.0 - numpy.exp(-1 / (vactrol_time * sample_rate))
lookup_tables_32.append(
    ('lp_coefficients', excursion / 2 * filter_coefficients)
)



"""----------------------------------------------------------------------------
2164 gain
----------------------------------------------------------------------------"""

gains = (numpy.arange(0, 257) / 256.0) * 3.3
gains =10 ** (-1.5 * gains)
lookup_tables.append(
    ('2164_gain', gains * 32767.0)
)



"""----------------------------------------------------------------------------
Compressor tables
----------------------------------------------------------------------------"""

t = (numpy.arange(0, 257) / 256.0)
lookup_tables_32 += [
    ('exp2', 65536.0 * 2 ** t),
    ('log2', 65536.0 * numpy.log2(256 + 256 * t))]

lookup_tables += [
    ('compressor_ratio', 256 / (24 * t * t + 1)),
    ('soft_knee', (t ** 3.0) * 65535.0)
]



"""----------------------------------------------------------------------------
Rate control
----------------------------------------------------------------------------"""

t = numpy.arange(0, 257) / 256.0
t /= (20 / 100.0 / 3.3)
f = 2.0 ** t
f /= f[-1]
f *= 0.02 * (1 << 24)

lookup_tables_32.append(('lorenz_rate', f))
