#!/usr/bin/python2.5
#
# Copyright 2015 Olivier Gillet.
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
import scipy.stats

lookup_tables = []
distributions = []

"""----------------------------------------------------------------------------
Raised cosine
----------------------------------------------------------------------------"""

x = numpy.arange(0, 257) / 256.0
c = 1.0 - (0.5 * numpy.cos(x * numpy.pi) + 0.5)
lookup_tables += [('raised_cosine', c)]

x = numpy.arange(0, 257) / 256.0
c = numpy.sin(x * numpy.pi * 2)
lookup_tables += [('sine', c)]



"""----------------------------------------------------------------------------
Logit table
----------------------------------------------------------------------------"""

x = numpy.arange(0, 257) / 256.0
log_odds = x * 20.0 - 10.0
odds = 2 ** log_odds
p = odds / (1 + odds)
lookup_tables += [('logit', p)]



"""----------------------------------------------------------------------------
Inverse CDF of Beta distribution for various combinations of alpha/beta.
Used as a LUT for inverse transform sampling.
----------------------------------------------------------------------------"""

N_nu = 9
N_mu = 5

def squash(x):
  return x / (1 + x ** 2) ** 0.5

nu_values = 2 ** numpy.array([9, 5, 3, 2.5, 2, 1.5, 1, 0.5, -1])
mu_values = numpy.linspace(0, 0.5, N_mu)
mu_values[0] = 0.05
plot = False

if plot:
  import pylab
  VOLTAGE_RANGE = 8

for i, mu in enumerate(mu_values):
  row = []
  for j, nu in enumerate(nu_values):
    error = numpy.exp(-(numpy.log2(nu) - 1) ** 2 / 20.0)
    corrected_mu = 0.5 * (2 * mu) ** (1 / (1 + 3.0 * error))
    alpha, beta = corrected_mu * nu, (1 - corrected_mu) * nu
    if plot:
      x = numpy.arange(-VOLTAGE_RANGE, VOLTAGE_RANGE, 0.1)
      p = scipy.stats.beta.pdf(0.5 * (x / VOLTAGE_RANGE + 1.0), alpha, beta)
      pylab.subplot(N_mu, N_nu, i * N_nu + j + 1)
      pylab.plot(x, p)
    
    body = numpy.arange(0, 129) / 128.0
    head = body / 20.0
    tail = body / 20.0 + 0.95
    values = numpy.hstack((body, head, tail))
    ppf = scipy.stats.beta.ppf(values, alpha, beta)
    row += [('icdf_%d_%d' % (i, j), ppf)]
    if j == N_nu - 1:
      row += [('icdf_%d_%d_guard' % (i, j), ppf)]
  
  distributions += row
  if i == N_mu - 1:
    distributions += [(name + '_guard', values) for (name, values) in row]

if plot:
  pylab.show()
