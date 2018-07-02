import numpy
import pylab

def dumb(f):
  return f * numpy.pi

def pade(f):
  return f * -0.90585 / (-0.28833 + f * f)

def poly3taylor(f):
  fsq = f * f
  r = 0.3333333 * numpy.pi ** 3
  r *= fsq
  r += numpy.pi
  r *= f
  return r

def poly3gradient(f, a=3.736e-01):
  fsq = f * f
  r = a * numpy.pi ** 3
  r *= fsq
  r += numpy.pi
  r *= f
  return r


def poly5mdsp(f, a=3.1755e-01, b=2.033e-01):
  fsq = f * f
  r = b * numpy.pi ** 5
  r *= fsq
  r += a * numpy.pi ** 3
  r *= fsq
  r += numpy.pi
  r *= f
  return r


def poly5gradient(f, a=3.260e-01, b=1.823e-01):
  f = f * numpy.pi
  fsq = f * f
  r = b
  r *= fsq
  r += a
  r *= fsq
  r += 1.0
  r *= f
  return r


def poly11mdsp(f):
  fsq = f * f
  r = 9.5168091e-03 * numpy.pi ** 11
  r *= fsq
  r += 2.900525e-03 * numpy.pi ** 9
  r *= fsq
  r += 5.33740603e-02 * numpy.pi **7
  r *= fsq
  r += 1.333923995e-01 * numpy.pi **5
  r *= fsq
  r += 3.333314036e-01 * numpy.pi **3
  r *= fsq
  r += numpy.pi
  r *= f
  return r


def compute_filter_settings(cutoff, resonance):
  g = numpy.tan(numpy.pi * cutoff) + resonance * 0
  r = 1.0 / resonance + cutoff * 0
  h = 1 / (1 + r * g + g * g)
  return g, r, h


def evaluate(groundtruth_f, approximate_g):
  approximate_f = numpy.arctan(approximate_g) / numpy.pi
  return numpy.log2(approximate_f / groundtruth_f) * 1200.0

  
f = numpy.exp(numpy.linspace(numpy.log(16), numpy.log(10000), 1000.0))
f /= 48000.0
g, _, _ = compute_filter_settings(f, 0.5)
approximations = [pade, poly3gradient, poly5mdsp, poly5gradient, poly11mdsp]
#
# a_ = numpy.linspace(3.259e-01, 3.261e-01, 100)
# b_ = numpy.linspace(1.822e-01, 1.823e-01, 100)
# error = numpy.zeros((100, 100))
# best = 1e8
# arg_best = None
# for i, a in enumerate(a_):
#   for j, b in enumerate(b_):
#     error[i, j] = (evaluate(f, fast0(f, a, b)) ** 2).sum()
#     if error[i, j] < best:
#       best = error[i, j]
#       arg_best = (a, b)
#
# print arg_best
# pylab.plot(coefficient, error)
# pylab.show()

pylab.figure(figsize=(15,10))
for i, a in enumerate(approximations):
  n = len(approximations)
  # pylab.subplot(n * 100 + 10 + i + 1)
  pylab.plot(f * 48000, evaluate(f, a(f)))
  pylab.xlabel('Hz')
  pylab.ylabel('$\delta$ cents')

pylab.legend(map(lambda x: x.__name__, approximations))
pylab.tight_layout()
#pylab.savefig('plot.pdf')
pylab.show()
