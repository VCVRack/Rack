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

import numpy
import pylab

lookup_tables = []

sample_rate = 31250

"""----------------------------------------------------------------------------
Envelope times.
----------------------------------------------------------------------------"""

num_values = 4096
max_time = 16.0  # seconds
min_time = 0.001
gamma = 0.125
min_frequency = 1.0 / (max_time * sample_rate)
max_frequency = 1.0 / (min_time * sample_rate)
at_0 = numpy.power(max_frequency, -gamma)
at_1 = numpy.power(min_frequency, -gamma)
rates = numpy.linspace(0, 2.0, num_values) * (at_1 - at_0) + at_0
values = rates ** (-1/gamma)
lookup_tables.append(
    ('env_frequency', values)
)

"""----------------------------------------------------------------------------
Portamento LPF coefficient.
----------------------------------------------------------------------------"""

num_values = 512
max_time = 4.0  # seconds
min_time = 0.0001
min_frequency = 1.0 / (max_time * sample_rate)
max_frequency = 1.0 / (min_time * sample_rate)
rates = numpy.linspace(numpy.log(max_frequency),
                       numpy.log(min_frequency), num_values)
values = numpy.exp(rates)
lookup_tables.append(
    ('portamento_coefficient', values)
)



"""----------------------------------------------------------------------------
Sine table.
----------------------------------------------------------------------------"""

size = 1024
t = numpy.arange(0, size + size / 4 + 1) / float(size) * numpy.pi * 2
lookup_tables.append(('sine', numpy.sin(t)))

