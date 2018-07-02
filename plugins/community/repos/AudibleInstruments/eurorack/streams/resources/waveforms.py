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
Vactrol saturation waveshaper
----------------------------------------------------------------------------"""

WAVESHAPER_SIZE = 1024

x = numpy.arange(0, WAVESHAPER_SIZE + 1) / float(WAVESHAPER_SIZE)
x = numpy.exp(-10 * numpy.exp(-13 * x))
# Add a slight slope
x += 0.1 * (x ** 0.05)
x = x - x.min()
x /= x.max()
waveforms = [('gompertz', 32767.0 * x)]


"""----------------------------------------------------------------------------
Linear to dB, for display
----------------------------------------------------------------------------"""

db = numpy.arange(0, 257)
db[0] = 1
db[db > 255] = 255
db = numpy.log2(db / 16.0) * 32768 / 4
waveforms += [('db', db)]
