#!/usr/bin/python2.5
#
# Copyright 2009 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
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
#
# Self-organizing map.

import numpy
import random


class SOM(object):
  
  def __init__(self, grid_size, radius, learning_rate):
    self._grid_size = grid_size
    self._grid_x = numpy.arange(0, grid_size * grid_size) % grid_size
    self._grid_y = numpy.arange(0, grid_size * grid_size) / grid_size
    self._radius = radius
    self._learning_rate = learning_rate
    self._codewords = None

  @staticmethod
  def standardize(x, std_power=1, axis=0, regularization=0.0):
    t, n = x.shape
    mean = x.mean(axis=axis)
    std = x.std(axis=axis) ** std_power + regularization
    return (x - mean) / std
    
  def classify(self, data):
    distances = ((self._codewords - data) ** 2).sum(axis=1)
    return distances.argmin(), distances
    
  def train(self, data, iterations=10000, seed=42):
    n, d = data.shape
    nodes = self._grid_size
    numpy.random.seed(seed)
    random.seed(seed)
    self._codewords = numpy.random.randn(nodes * nodes, d)
    self._error_history = []
    milestone = 2
    for i in xrange(iterations):
      if i == milestone:
        print 'iteration', i, 'of', iterations, '\t', \
            round(1000.0 * i / iterations) * 0.1, '%'
        milestone <<= 1
      radius = self._radius * 2 ** (-2 * float(i) / iterations)
      learning_rate = self._learning_rate * 2 ** (-7 * float(i) / iterations)
      
      # Pick a random vector.
      x = random.choice(data)
      
      # Find best matching unit.
      bmu, distances = self.classify(x)
      self._error_history.append(distances[bmu])
      
      # Compute neighborhood update function.
      delta_x = self._grid_x[bmu] - self._grid_x
      delta_y = self._grid_y[bmu] - self._grid_y
      rbf = numpy.exp(-(delta_x ** 2 + delta_y ** 2) / (radius * radius))
      rbf = numpy.tile(rbf.reshape((nodes * nodes, 1)), (1, d))
      update = rbf * (numpy.tile(x, (nodes * nodes, 1)) - self._codewords)
      self._codewords += learning_rate * update
    return self._error_history
    
  def checkpoint(self):
    numpy.save('weights', self._codewords)

  def resume(self):
    self._codewords = numpy.load('weights.npy')

  def plot(self, x):
    import pylab

    for i in xrange(x.shape[0]):
      _, d = self.classify(x[i, :])
      d = numpy.exp(-d)
      pylab.figure()
      pylab.imshow(
          d.reshape((self._grid_size, self._grid_size)),
          interpolation='nearest')
      pylab.savefig('response_%d.pdf' % i)
